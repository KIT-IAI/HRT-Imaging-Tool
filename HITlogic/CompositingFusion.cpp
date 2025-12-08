/*******************************************************************************
SPDX-License-Identifier: GPL-2.0-or-later
Copyright 2010-2025 Karlsruhe Institute of Technology (KIT)
Contact: stephan.allgeier∂kit.edu,
         Institute of Automation and Applied Informatics

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, 51 Franklin Street,
Fifth Floor, Boston, MA 02110-1301, USA.
*******************************************************************************/



#include "stdafx.h"
#include "CompositingFusion.h"

#include "ImageUndistortion.h"



CCompositingFusion::CCompositingFusion(const CCompositingParameters& Param)
	: CCompositing(Param)
{
}

void CCompositingFusion::PerformCompositing(const CImageRegistrationData& ImageData, const CDenseMatrix& PositioningSolution)
{
	if (m_Parameters.eRegistrationProcessType == CProcessType::eRigidRegistration || m_Parameters.nSubImageHeight == static_cast<size_t>(ImageData.Images[0]->GetSize().y) || m_Parameters.nSubImageHeight == 0)
	{
		FusionRigidRegistration(ImageData, PositioningSolution);
	}
	else
	{
		Fusion(ImageData, PositioningSolution);
	}
}

/**	\brief Berechnet das Mosaikbild durch Bildfusion.
 *
 *	Das Mosaikbild kann nach dem Aufruf der Methode mit GetResultImage()
 *	abgefragt werden.
 *
 *	Die Ausgabeparameter \a pUndistortedImageList, \a pImageOffsetX,
 *	\a pImageOffsetY und \a pXOffsetList sind optional.
 *
 *	\param[out] pUndistortedImageList Enthält nach dem Methodenaufruf die
 *		entzerrten Einzelbilder.
 *	\param[out] pImageOffsetX,pImageOffsetY Enthält nach dem Methodenaufruf
 *		die x- bzw. y-Koordinaten der entzerrten Einzelbilder im
 *		Koordinatensystem des Mosaikbilds. Der Bezugspunkt ist jeweils das
 *		linke obere Pixel des Bilds.
 *	\param[out] pXOffsetList,pYOffsetList Enthält nach dem Methodenaufruf die
 *		x- bzw. y-Koordinaten jeder Bildzeile der Quellbilder im
 *		Koordinatensystem des Mosaikbilds. Der Bezugspunkt ist jeweils das
 *		linke Pixel der Bildzeile.
 */
void CCompositingFusion::Fusion(const CImageRegistrationData& ImageData, const CDenseMatrix& solutionMatrix)
{
	size_t nImageCount = ImageData.Images.size();

	assert(nImageCount > 0);

	m_UndistortedImages.resize(nImageCount);

	ProcessSolutionMatrix(solutionMatrix, nImageCount);

	SetSingleImageSize(ImageData.Images[0]->GetSize());

	assert(m_InterpolatedPosX.Rows() == m_InterpolatedPosY.Rows());
	assert(m_InterpolatedPosX.Cols() == m_InterpolatedPosY.Cols());
	assert(m_InterpolatedPosX.Rows() == static_cast<size_t>(ImageData.Images[0]->GetSize().y));
	assert(m_InterpolatedPosX.Cols() == nImageCount);

	CDenseMatrix matPosX(m_InterpolatedPosX);
	CDenseMatrix matPosY(m_InterpolatedPosY);

	// The vertical image row position values at this point don't represent
	// absolute positions. A value of y=posY(k) for an image row k means
	// something like "position the top source image row at coordinate y,
	// then image row k is at it's correct location". In order to get the
	// actual absolute vertical positions for all image rows, their offsets
	// relative to the top row have to be added.
	for (size_t row = 0; row < matPosY.Rows(); row++)
	{
		matPosY[row] += static_cast<double>(row);
	}

	// The global positioning step calculates image row positions in such a
	// way that the undistorted images are arranged more or less evenly
	// around the coordinate origin. In other words, the origin (0,0) is
	// located approximately in the center of the mosaic image or the global
	// frame. For the motion-correction ("undistortion") process it is easier
	// to translate the origin to the upper left corner, so that the global
	// coordinate system is identical to the global frame's (i.e. the mosaic
	// image's) one.
	matPosX -= ceil(matPosX.Min()) - m_Parameters.nBorder;
	matPosY -= ceil(matPosY.Min()) - m_Parameters.nBorder;

	assert(ceil(matPosX.Min()) == m_Parameters.nBorder);
	assert(ceil(matPosY.Min()) == m_Parameters.nBorder);

	// The motion correction approach implemented here assumes that the eye
	// does not move faster than the scanning laser along the vertical axis.
	// Otherwise, all kinds of funny things can happen, such as vertically
	// flipped images and multiple image rows representing the exact same
	// region of the cornea. Apart from being very difficult to actually
	// register successfully, this would also necessitate a more complicated
	// motion correction (or _T("undistortion")) scheme.
	// In mathematical terms this assumption translates to each single
	// image's row y-coordinates, stored in the column vectors of matPosY,
	// being strictly monotonic increasing.
	size_t nMonotonyViolatedCount = GetMonotonyViolationCount(matPosY);
	if (nMonotonyViolatedCount > 0)
	{
		std::wostringstream oss;
		oss << L"Violation of monotony of image line y-positions (this occurred " << nMonotonyViolatedCount << L" times)";
		CLog::Log(CLog::eWarning, L"CCompositingFusion", oss.str().c_str());
	}

	long long nImageSizeX = m_SingleImageSize.x;

	long long nMosaicSizeX = static_cast<long long>(floor(matPosX.Max())) + m_Parameters.nBorder + nImageSizeX;
	long long nMosaicSizeY = static_cast<long long>(floor(matPosY.Max())) + m_Parameters.nBorder + 1;

	m_result.emplace<StlImage<float>>(StlImage<float>());
	m_result = StlImage<float>();

	assert(std::holds_alternative<StlImage<float>>(m_result));	//nahezu unmöglich, das ein anderer Datentyp bei diesem Aufruf in der Variable ist
	std::get<StlImage<float>>(m_result).Free();
	std::get<StlImage<float>>(m_result).Alloc({ nMosaicSizeX, nMosaicSizeY });
	std::get<StlImage<float>>(m_result).Clear(0.0);

	StlImage<float> imgMosaicWeight;
	imgMosaicWeight.Alloc({ nMosaicSizeX, nMosaicSizeY });
	imgMosaicWeight.Clear(0.0);

	CImageUndistortion Undistorter(CImageUndistortion::EInterpolationMode::eLinear, CImageUndistortion::EInterpolationMode::eLinear);

	for (size_t nBild = 0; nBild < nImageCount; nBild++)
	{
		if (m_bIsCanceled)
			break;

		auto imgWeight = GenerateWeightImage(*ImageData.Images[nBild]);

		CDenseMatrix Distortion(std::vector<CDenseVector>{ matPosX.Col(nBild), matPosY.Col(nBild) }, true);
		auto Undistorted = Undistorter.UndistortImage(*ImageData.Images[nBild], Distortion);
		auto UndistortedWeight = Undistorter.UndistortImage(imgWeight, Distortion);
		m_UndistortedImages[nBild] = Undistorted;

		if (m_Parameters.bExcludeNonSNPImages && ImageData.ImageClasses.IsLoaded() && ImageData.ImageClasses.GetClass(nBild) != EHRTImageClassificationClass::eSNP)
			continue;

		auto WeightedUndistortedImage = *Undistorted.pUndistoredImage * *UndistortedWeight.pUndistoredImage;

		// Fuse the current input image with the output mosaic image
		std::get<StlImage<float>>(m_result).AddAtOffset({ Undistorted.TopLeftCorner.x, Undistorted.TopLeftCorner.y }, WeightedUndistortedImage);

		imgMosaicWeight.AddAtOffset({ Undistorted.TopLeftCorner.x, Undistorted.TopLeftCorner.y }, *UndistortedWeight.pUndistoredImage);
	}

	// The following prevents divisions by zero. Note that all pixels with
	// a weight of zero will also have a value of zero and remain unchanged
	// by the division.
	imgMosaicWeight.Replace(0, 1);
	std::get< StlImage<float>>(m_result) /= imgMosaicWeight;

	// Finally, fill the background with the background color
	std::get< StlImage<float>>(m_result).Replace(0.0f, static_cast<float>(m_Parameters.cBackgroundColor));
}

void CCompositingFusion::FusionRigidRegistration(const CImageRegistrationData& ImageData, const CDenseMatrix& solutionMatrix)
{
	//hier fehlt noch das Verschleifen der Ränder
	assert(!ImageData.Images.empty());

	StlImageSize SingleImageSize;
	StlImageSize szMosaicSize;

	CDenseVector vecSolutionPosX = solutionMatrix.Col(0);
	CDenseVector vecSolutionPosY = solutionMatrix.Col(1);

	for (size_t v = 0; v < vecSolutionPosX.Size(); v++)
	{
		vecSolutionPosX[v] = CMathTools::RoundValue(vecSolutionPosX[v]);
		vecSolutionPosY[v] = CMathTools::RoundValue(vecSolutionPosY[v]);
	}

	CDenseVector vecPosX = vecSolutionPosX - static_cast<int>(ceil(vecSolutionPosX.Min()));
	CDenseVector vecPosY = vecSolutionPosY - static_cast<int>(ceil(vecSolutionPosY.Min()));

	assert(vecPosX.Min() == 0.0);
	assert(vecPosY.Min() == 0.0);

	szMosaicSize.x = static_cast<int>(floor(vecPosX.Max())) + ImageData.Images[0]->GetSize().x;
	szMosaicSize.y = static_cast<int>(floor(vecPosY.Max())) + ImageData.Images[0]->GetSize().y;

	SingleImageSize = ImageData.Images[0]->GetSize();

	StlImage<float> mosaicWeightImage;
	mosaicWeightImage.Alloc(szMosaicSize);
	mosaicWeightImage.Clear(0);

	m_result.emplace<StlImage<float>>(StlImage<float>());
	m_result = StlImage<float>();
	assert(std::holds_alternative<StlImage<float>>(m_result));	//nahezu unmöglich, das ein anderer Datentyp bei diesem Aufruf in der Variable ist
	std::get<StlImage<float>>(m_result).Free();
	std::get<StlImage<float>>(m_result).Alloc(szMosaicSize);
	std::get<StlImage<float>>(m_result).Clear(0);

	StlImage<float> weightedImage;
	weightedImage.Alloc(SingleImageSize);

	for (size_t i = 0; i < ImageData.Images.size(); i++)
	{
		StlImage<float> singleWeightImage = GenerateWeightImage(*ImageData.Images[i]);

		//Bilddaten
		weightedImage = (*ImageData.Images[i]) * singleWeightImage;
		StlImage<float>& res = std::get<StlImage<float>>(m_result);
		res.AddAtOffset({ static_cast<int>(vecPosX[i]), static_cast<int>(vecPosY[i]) }, weightedImage);
		/*childResultImage.Child2d(std::get<CMeasureImage>(m_result), static_cast<int>(vecPosX[i]), static_cast<int>(vecPosY[i]), SingleImageSize.cx, SingleImageSize.cy);
		childResultImage += weightedImages;
		childResultImage.Free();*/

		////Gewichtung
		mosaicWeightImage.AddAtOffset({ static_cast<int>(vecPosX[i]), static_cast<int>(vecPosY[i]) }, singleWeightImage);
		/*childWeightImage.Child2d(mosaicWeightImage, static_cast<int>(vecPosX[i]), static_cast<int>(vecPosY[i]), SingleImageSize.cx, SingleImageSize.cy);
		childWeightImage += singleWeightImage;
		childWeightImage.Free();*/
	}

	mosaicWeightImage.Replace(0, 1);
	//std::get<CMeasureImage>(m_result).Arith(std::get<CMeasureImage>(m_result), mosaicWeightImage, M_DIV);
	std::get<StlImage<float>>(m_result) /= mosaicWeightImage;
}

size_t CCompositingFusion::GetMonotonyViolationCount(const CDenseMatrix& matPosY)
{
	size_t nMonotonyViolatedCount = 0;
	for (size_t col = 0; col < matPosY.Cols(); col++)
	{
		for (size_t row = 1; row < matPosY.Rows(); row++)
		{
			if (matPosY[row][col] <= matPosY[row - 1][col])
			{
				// Probably a totally
				// wrong registration result, that somehow slipped through
				// all of the checks and was finally accepted as correct.
				//assert(false);
				// Small update. Since this occures very often at the moment (Rev. 1553) It is no longer an assertion (hasn't been for a while), and no longer logs it every time, but only once.
				nMonotonyViolatedCount++;
			}
		}
	}
	return nMonotonyViolatedCount;
}

const std::vector<CUndistortedImage>& CCompositingFusion::GetUndistortedImages() const
{
	return m_UndistortedImages;
}

/**	\brief Generiert das Gewichtsbild für die Gewichtung der Einzelbilder bei
 *	der Bildfusion.
 *
 *	Der Puffer für das Gewichtsbild muss vor dem Aufruf der Methode in der
 *	Größe des Quellbilds \a imgSource allokiert werden.
 *
 *	\param[out] imgWeight Enthält nach der Rückkehr aus der Methode das
 *		Gewichtsbild.
 *	\param[in] imgSource Das Quellbild.
 */
StlImage<float> CCompositingFusion::GenerateWeightImage(const StlImage<float>& imgSource) const
{
	assert(imgSource.IsAllocated());

	StlImage<float> weightIm;

	switch (m_Parameters.eWeightMode)
	{
	case CCompositingParameters::EWeightMode::eCos2:
		weightIm = GenerateWeightImgCos2(imgSource.GetSize());
		break;
	case CCompositingParameters::EWeightMode::eDistance:
		weightIm = GenerateWeightImgDistance(imgSource);
		break;
	case CCompositingParameters::EWeightMode::eNone:
		weightIm.Alloc(imgSource.GetSize());
		weightIm.Clear(1.0);
		break;
	default:
		assert(false);
	}
	return weightIm;
}

/**	\brief Generiert das Gewichtsbild für die Gewichtung der Einzelbilder bei
 *	der Bildfusion nach der Methode des doppelten Cosinus.
 *
 *	Der Puffer für das Gewichtsbild muss vor dem Aufruf der Methode in der
 *	gewünschten Größe allokiert werden.
 *
 *	\param[out] imgWeight Enthält nach der Rückkehr aus der Methode das
 *		Gewichtsbild.
 */
StlImage<float> CCompositingFusion::GenerateWeightImgCos2(const StlImageSize& ImageSize) const
{
	StlImage<float> imgWeight;
	imgWeight.Alloc(ImageSize);

	long long nSizeX = imgWeight.GetSize().x;
	long long nSizeY = imgWeight.GetSize().y;

	// The weight image is a two-dimensional function composed of two
	// one-dimensional sine functions. We first compute the one-dimensional
	// functions and multiplicate them later. This way we only have O(N)
	// calls to the probably computationally expensive sin function instead
	// of O(N^2) if it was done in the x-y-loop below. N is equal to the
	// larger dimension of the buffer size.
	std::vector<double> pSinX(nSizeX);
	std::vector<double> pSinY(nSizeY);

	// NOTE: The addition of 0.5 to the x and y coordinates is for symmetry
	//		 reasons. It shifts the scan interval of the sine function from
	//		 [0,size-1]/size to [0.5,size-0.5]/size.
	for (int x = 0; x < nSizeX; x++)
	{
		pSinX[x] = sin(M_PI * (x + 0.5) / nSizeX);
	}
	for (int y = 0; y < nSizeY; y++)
	{
		pSinY[y] = sin(M_PI * (y + 0.5) / nSizeY);
	}

	for (int x = 0; x < nSizeX; x++)
	{
		for (int y = 0; y < nSizeY; y++)
		{
			(imgWeight)[{x, y}] = static_cast<float>(pow(pSinX[x] * pSinY[y], 2));
		}
	}

	return imgWeight;
}

/**	\brief Generiert das Gewichtsbild für die Gewichtung der Einzelbilder bei
 *	der Bildfusion mittels Distanztransformation.
 */
StlImage<float> CCompositingFusion::GenerateWeightImgDistance(const StlImage<float>& imgSource, size_t nEdgeBlendingLength /*= 10*/) const
{
	assert(imgSource.IsAllocated());					//the distance-Funktion is missing in stlImage

	// Sadly the source buffer needs to be framed in order to be able to perform the Distance-transformation on images that contain no backgroudn pixels (like SNP-Images).
	StlImage<float> FramedWeightImage;
	FramedWeightImage.Alloc({ imgSource.GetSize().x + 2, imgSource.GetSize().y + 2 });
	FramedWeightImage.Clear(0.0);
	FramedWeightImage.Copy2d(StlImageRect({ 1,1 }, imgSource.GetSize()), imgSource, { 0,0 });

	float fEdgeBlendingLength = static_cast<float>(nEdgeBlendingLength);

	FramedWeightImage.DistanceChamfer();
	FramedWeightImage.Clamp(0, fEdgeBlendingLength);
	FramedWeightImage /= fEdgeBlendingLength;

	StlImage<float> CroppedWeightImage;
	CroppedWeightImage.Alloc(imgSource.GetSize());
	CroppedWeightImage.Copy2d(StlImageRect({ 0,0 }, imgSource.GetSize()), FramedWeightImage, { 1,1 });

	return CroppedWeightImage;
}

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
#include "Compositing3DCloud.h"
#include "ImageUndistortion.h"
#include "Log.h"

CCompositing3DCloud::CCompositing3DCloud(const CCompositingParameters& Param)
	: CCompositing(Param)
{
}

void CCompositing3DCloud::PerformCompositing(const CImageRegistrationData& ImageData, const CDenseMatrix& solutionMatrix)
{

	size_t nImageCount = ImageData.Images.size();

	ASSERT(nImageCount > 0);


	m_UndistortedImages.resize(nImageCount);


	ProcessSolutionMatrix(solutionMatrix, nImageCount);

	SetSingleImageSize(ImageData.Images[0]->GetSize());

	ASSERT(m_InterpolatedPosX.Rows() == m_InterpolatedPosY.Rows());
	ASSERT(m_InterpolatedPosX.Cols() == m_InterpolatedPosY.Cols());
	ASSERT(m_InterpolatedPosX.Rows() == static_cast<size_t>(ImageData.Images[0]->GetSize().y));
	ASSERT(m_InterpolatedPosX.Cols() == nImageCount);

	//!!!kontrolieren !
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

	ASSERT(ceil(matPosX.Min()) == m_Parameters.nBorder);
	ASSERT(ceil(matPosY.Min()) == m_Parameters.nBorder);

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
		CLog::Log(CLog::eWarning, _T("HRT Image Motion Correction"), _T("Unexpected condition: Violation of monotony of image line Y-positions. Occured %d times."), nMonotonyViolatedCount);
	//!!!kontrolieren !


	m_result.emplace<C3DCloud>(C3DCloud());
	//ASSERT(std::holds_alternative<C3DCloud>(m_result));	//nahezu unmöglich, das ein anderer Datentyp bei diesem Aufruf in der Variable ist

	//zum debuggen
	/*
	INT_PTR nImageSizeX = m_SingleImageSize.x;
	INT_PTR nMosaicSizeX = static_cast<INT_PTR>(floor(matPosX.Max())) + m_Parameters.nBorder + nImageSizeX;
	INT_PTR nMosaicSizeY = static_cast<INT_PTR>(floor(matPosY.Max())) + m_Parameters.nBorder + 1;
	CMeasureImage out;
	out.Alloc2d(nMosaicSizeX, nMosaicSizeY, M_FLOAT | 32);
	out.Clear(0.0);*/

	float intLimit = m_Parameters.fIntensityLimit;
	if (intLimit < 0.0f || intLimit > 256.0f) intLimit = 0.0f;	//wenn Intensitätslimit außerhalb der Grenzen: setzen auf nur schwarz Filtern, schwarz (0.0) wird immer gefiltert (transparente Pixel)

	if (intLimit == 0.0)  std::get<C3DCloud>(m_result).reserve(nImageCount * (ImageData.Images[0]->GetSize().x) * (ImageData.Images[0]->GetSize().y));	//Größe nur im vorraus bekannt wenn intLimit = 0
	//CImageUndistortion Undistorter(CImageUndistortion::EInterpolationMode::eLinear, CImageUndistortion::EInterpolationMode::eLinear);

	float zMulti = m_Parameters.fZMultiplier;

	float* imgLineBuff = new float[384];

	for (INT_PTR nBild = 0; static_cast<size_t>(nBild) < nImageCount; nBild++)
	{
		if (m_bIsCanceled)
			break;

		int imgSizeX = ImageData.Images[nBild]->GetSize().x;

		CDenseMatrix Distortion(vector<CDenseVector>{ matPosX.Col(nBild), matPosY.Col(nBild) }, true);

		for (int imgY = 0; imgY < 384; imgY++) {	//Bildzeile
			auto x = Distortion[imgY][0];
			auto y = Distortion[imgY][1];
			auto z = ImageData.ImageInfo.getImageDepthAt(nBild) + ImageData.ImageInfo.getImageInclination(nBild) * imgY;
			z *= zMulti;

			for (int imgX = 0; imgX < imgSizeX; imgX++) {	//setze Bild Punktweise in Volumen
				if (imgLineBuff[imgX] > intLimit) {
					std::get<C3DCloud>(m_result).add(x + imgX, y, z, (*ImageData.Images[nBild])[{imgX, imgY}]);
				}
			}

		}
	}
	delete[] imgLineBuff;
}

size_t CCompositing3DCloud::GetMonotonyViolationCount(const CDenseMatrix& matPosY)
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
				//ASSERT(false);
				// Small update. Since this occures very often at the moment (Rev. 1553) It is no longer an assertion (hasn't been for a while), and no longer logs it every time, but only once.
				nMonotonyViolatedCount++;
			}
		}
	}
	return nMonotonyViolatedCount;
}

const vector<CUndistortedImage>& CCompositing3DCloud::GetUndistortedImages() const
{
	return m_UndistortedImages;
}
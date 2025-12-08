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
#include "CompositingVoxelBuffer.h"



CCompositingVoxelBuffer::CCompositingVoxelBuffer(const CCompositingParameters& Param)
	: CCompositing(Param)
{
}

void CCompositingVoxelBuffer::PerformCompositing(const CImageRegistrationData& ImageData, const CDenseMatrix& solutionMatrix)
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

	//für genauere Dokumentation siehe CompositingFusion bzw. Compositing3DCloud

	//Positionen der verschiedene Bildzeilen
	CDenseMatrix matPosX(m_InterpolatedPosX);
	CDenseMatrix matPosY(m_InterpolatedPosY);

	//Bestimmen der absoluten Positionen
	for (size_t row = 0; row < matPosY.Rows(); row++)
	{
		matPosY[row] += static_cast<double>(row);
	}

	//verschieben Koordinaten damit alle Positiv
	matPosX -= ceil(matPosX.Min()) - m_Parameters.nBorder;
	matPosY -= ceil(matPosY.Min()) - m_Parameters.nBorder;

	assert(ceil(matPosX.Min()) == m_Parameters.nBorder);
	assert(ceil(matPosY.Min()) == m_Parameters.nBorder);

	CHRTImageDepthAndAngleData depthInfo = ImageData.ImageInfo;

	depthInfo.depthMinus(depthInfo.getMinDepth()); //z-Koordinate

	//prüfen zu schnelle Augenbewegung bzw. Registrierfehler
	size_t nMonotonyViolatedCount = GetMonotonyViolationCount(matPosY);
	if (nMonotonyViolatedCount > 0)
	{
		std::wostringstream oss;
		oss << L"Violation of monotony of image line y-positions (this occurred " << nMonotonyViolatedCount << L" times)";
		CLog::Log(CLog::eWarning, L"CCompositingVoxelBuffer", oss.str().c_str());
	}

	//Größe für Buffer bestimmen
	long long nImageSizeX = m_SingleImageSize.x;
	long long nMosaicSizeX = static_cast<long long>(floor(matPosX.Max())) + m_Parameters.nBorder + nImageSizeX;
	long long nMosaicSizeY = static_cast<long long>(floor(matPosY.Max())) + m_Parameters.nBorder + 1;

	double interpolationArea = 1.75;		//todo: Eintrag als Parameter
	auto zOffset = ceil(interpolationArea + abs((depthInfo.getMinInclination() / 2) * 384));	//sorge für ausreichend Platz in Volumen (meist etwas zu viel)

	//Anlegen Ausgabedatentyp
	m_result.emplace<C3DBuffer<float>>(C3DBuffer<float>());
	std::get<C3DBuffer<float>>(m_result).Alloc({ nMosaicSizeX, nMosaicSizeY, static_cast<long long>((ceil(depthInfo.getMaxDepth()) + zOffset * 2) * m_Parameters.fZMultiplier) });
	std::get<C3DBuffer<float>>(m_result).Clear(0.0);

	//Anlegen temporärer Buffer
	C3DBuffer<float> divider;
	divider.Alloc({ nMosaicSizeX, nMosaicSizeY, static_cast<long long>((ceil(depthInfo.getMaxDepth()) + zOffset * 2) * m_Parameters.fZMultiplier) });
	divider.Clear(0.0);

	auto maxDist = interpolationArea;
	/*sqrt(	//simpler Pythagoras
	pow(ceil(interpolationArea), 2) +
	pow(ceil(interpolationArea), 2) +
	pow(ceil(interpolationArea), 2) + 4
);*/

//auto zOffset = ceil(interpolationArea) + m_depthInfo.getMinInclination() * 384;

//auto imgWeight = GenerateWeightImage(*pImages.at(nBild));

	std::vector<float> imgLineBuff;
	std::vector<float> divBuff;
	for (int imgNr = 0; imgNr < nImageCount; imgNr++)
	{
		int imgSizeX = ImageData.Images[imgNr]->GetSize().x;
		imgLineBuff.resize(imgSizeX);
		divBuff.resize(imgSizeX);
		CDenseMatrix Distortion(std::vector<CDenseVector>{ matPosX.Col(imgNr), matPosY.Col(imgNr) }, true);
		//Distortion.WriteMatrix(L"E://tmp//Matrix_" + std::to_wstring(imgNr) + L".txt");	//debug

		for (int j = 0; j < 384; j++) {	//j = Bildzeile
			auto x = Distortion[j][0];
			auto y = Distortion[j][1];
			auto z = depthInfo.getImageDepthAt(imgNr) + depthInfo.getImageInclination(imgNr) * j;
			z += zOffset;						 //damit Ergebnis nicht unten aus Volumen herausragt
			//z *= m_Parameters.fZMultiplier;

			auto xHigh = floor(x + interpolationArea);
			auto yHigh = floor(y + interpolationArea);
			auto zHigh = floor(z + interpolationArea);
			//	auto distHigh = maxDist-sqrt(pow(x - xHigh, 2.0) + pow(y - yHigh, 2.0));	//1.5 ~ sqrt(2) => maximale Distanz

			auto xLow = ceil(x - interpolationArea);
			auto yLow = ceil(y - interpolationArea);
			auto zLow = ceil(z - interpolationArea);
			//		auto distLow = maxDist-sqrt(pow(x - xLow, 2.0) + pow(y - yLow, 2.0));

			for (int iX = xLow; iX <= xHigh; iX++) {
				for (int iY = yLow; iY <= yHigh; iY++) {
					for (int iZ = zLow; iZ <= zHigh; iZ++) {
						auto dist = maxDist - sqrt(pow(x - iX, 2.0) + pow(y - iY, 2.0) + pow(z - iZ, 2.0));

						if (dist < 0) {	//suche in Kugel um Zielvoxel
							continue;
						}
						dist /= maxDist; //Wert zwischen 0 und 1

						dist = pow(dist, 4);	//quartische statt lineare Gewichtung

						for (long long i = 0; i < imgSizeX; i++)
						{
							imgLineBuff[i] = ImageData.Images[imgNr]->at({ i, j }) * dist; //hole Zeile und multipliziere mit Gewichtung
						}

						std::fill(divBuff.begin(), divBuff.end(), dist);	//merke Gewichtung

						std::get<C3DBuffer<float>>(m_result).AddLineAtOffset({ iX, iY, iZ }, imgLineBuff);	//lege gewichtete Zeile ab
						divider.AddLineAtOffset({ iX, iY, iZ }, divBuff);	//lege Gewicht ab

					}
				}
			}

		}
	}

	divider.Replace([&](float n) -> bool {return n < 0.00001; }, 1.0);		//vllt. < durch > tauschen
	//divider.replaceEqual(0.0, 1.0, 0.000001);	//sehr geringes epsilon, damit nicht zu viele Informationen verloren gehen, verhindert teilen durch 0

	std::get<C3DBuffer<float>>(m_result) /= divider;

	//assert(false); //todo: Code ersetzen
	/*auto count = std::get<C3DBuffer<float>>(m_result).replaceBigger(255.001, 255.0);				//255.1 da aufgrund von float Werte manchmal leicht größer als 255 sind;
	if(count > 0)
		CLog::Log(CLog::eWarning, _T("CCompositingVoxelBuffer"), _T("Found and replaced %d miscalculated Voxels (Voxelvalue > 255)."), count);*/

	divider.Free();
}

size_t CCompositingVoxelBuffer::GetMonotonyViolationCount(const CDenseMatrix& matPosY)
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

const std::vector<CUndistortedImage>& CCompositingVoxelBuffer::GetUndistortedImages() const
{
	return m_UndistortedImages;
}

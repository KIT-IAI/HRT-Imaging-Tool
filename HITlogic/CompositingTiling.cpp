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

#include "CompositingTiling.h"

const int CCompositingTiling::s_FrameHeightDiff = 2;
const double CCompositingTiling::s_FrameSizeFactor = 0.9;


CCompositingTiling::CCompositingTiling(const CCompositingParameters& Param)
	: CCompositing(Param)
{
}


void CCompositingTiling::PerformCompositing(const CImageRegistrationData& ImageData, const CDenseMatrix& PositioningSolution)
{
	Combining(ImageData.Images, PositioningSolution);
}

/** \brief Steuert die Methoden Aufrufe für die Bildkomposition
*	\param [in] nMaximumGridLength maximale Größe eines Rasterbereichs
*	\param [in] nSx	Teilbildhöhe für die die Verzerrungswerte berechnet wurden
*	\param [in] PositioningSolution Eingangsmatrix z. B. von C2DNerveFiberMapping
*	\param [in] nEdgeBlendingLength Uberlappungsbereich zum verschmelzen der Kanten
*	\see CCompositing::SetMatrix
*/
void CCompositingTiling::Combining(const vector<StlImage<float>*>& Images, const CDenseMatrix& PositioningSolution)
{
	ProcessSolutionMatrix(PositioningSolution, Images.size());

	SetSingleImageSize(Images.at(0)->GetSize());

	StlImageSize MosaicSize = CalculateMosaicImageSize();

	Cmps_Size_Selection(MosaicSize);
	GenerateImage(Images, MosaicSize);
}
StlImageSize CCompositingTiling::CalculateMosaicImageSize()
{
	StlImageSize MosaicImageSize;

	m_MinimumMeanImagePosition = { m_MeanImagePosition.Col(0).Min(), m_MeanImagePosition.Col(1).Min() };
	m_MaximumMeanImagePosition = { m_MeanImagePosition.Col(0).Max(), m_MeanImagePosition.Col(1).Max() };

	MosaicImageSize.x = static_cast<LONG>(ceil(m_MaximumMeanImagePosition.x - m_MinimumMeanImagePosition.x + m_SingleImageSize.x + m_Parameters.nBorder));
	MosaicImageSize.y = static_cast<LONG>(ceil(m_MaximumMeanImagePosition.y - m_MinimumMeanImagePosition.y + m_SingleImageSize.y + m_Parameters.nBorder));

	return MosaicImageSize;
}


/** \brief brechnet die Größe des Ergebnisbildes und trifft eine Auswahl der zu verwendenden
*			Bilder bei der Gesamtbilderstellung.
*
*	\author Robert Paasche
*
*	\param[in] mX mitllere Verschiebungswerte für horizentale und vertikale Richtung
*	\param[in] fGridLength minimales Bildraster, für jedes Raster wird nur ein Bild ausgewählt
*	\param[in] m_fSizeT Größe der vorliegenden unkorrigierten Bilder
*
*	\param[out] fSize ermittelte Größe des Gesamtbilds
*
*	\see GetImageSize
*	\see SelectImageToUse
*	\see SelectImageToUse
*/
void CCompositingTiling::Cmps_Size_Selection(const StlImageSize& nSize)
{
	m_nUsedPic.FreeMatrix();

	/*Anzahl der Fenster bestimmen*/
	m_nNumberOfFrames.x = static_cast<long>(floor((nSize.x + m_SingleImageSize.x) / m_Parameters.nMaximumGridLength));
	m_nNumberOfFrames.y = static_cast<long>(floor((nSize.y + m_SingleImageSize.y) / m_Parameters.nMaximumGridLength));

	/*Fenstergröße bestimmen*/
	m_fSubSize.x = (nSize.x / (static_cast<double>(m_nNumberOfFrames.x) + 0.1));
	m_fSubSize.y = (nSize.y / (static_cast<double>(m_nNumberOfFrames.y) + 0.1));

	m_nUsedPic.AllocMatrix(m_nNumberOfFrames.x, m_nNumberOfFrames.y);
	m_nUsedPic.Fill(0.0);
	SelectImageToUse();
}

void CCompositingTiling::SelectImageToUse()
{

	double min_d;
	double d;

	for (int nIndexX = 0; nIndexX < m_nNumberOfFrames.x; nIndexX++)
	{
		for (int nIndexY = 0; nIndexY < m_nNumberOfFrames.y; nIndexY++)
		{
			min_d = sqrt(pow(m_SingleImageSize.x, 2) + pow(m_SingleImageSize.y, 2)) * s_FrameSizeFactor;
			m_nUsedPic[nIndexX][nIndexY] = -1;
			for (size_t nImgIndex = 0; nImgIndex < m_MeanImagePosition.Rows(); nImgIndex++)
			{
				/*Satz des Pythagoras zum ermitteln welches Bild am Nähesten zum Mittelpunkt des Rasters liegt*/
				auto a = m_fSubSize.x * (nIndexX + 0.5) - m_MeanImagePosition[nImgIndex][0] - static_cast<double>(m_SingleImageSize.x / 2) + m_MinimumMeanImagePosition.x;
				auto b = m_fSubSize.y * (nIndexY + 0.5) - m_MeanImagePosition[nImgIndex][1] - static_cast<double>(m_SingleImageSize.y / 2) + m_MinimumMeanImagePosition.y;
				d = sqrt(pow(a, 2) + pow(b, 2));

				if (d < min_d)
				{
					min_d = d;
					m_nUsedPic[nIndexX][nIndexY] = static_cast<double>(nImgIndex);
				}
			}
		}
	}
}

/** \brief erstellt das Gesamtbild
*
*	\author Robert Paasche
*
*	\param[in] Ux, Uy Verzerrungsfunktionen
*	\param[in] BoundX Minimal- und Maximalwerte der Verzerrungsfunktionen
*/
void CCompositingTiling::GenerateImage(const vector<StlImage<float>*>& Images, const StlImageSize& MosaicImageSize)
{
	size_t nMosaicImageArea = static_cast<size_t>(MosaicImageSize.x * MosaicImageSize.y);

	vector<float> pDest(nMosaicImageArea);
	std::fill(pDest.begin(), pDest.end(), static_cast<float>(m_Parameters.cBackgroundColor));

	vector<float> pWeight(nMosaicImageArea);
	std::fill(pWeight.begin(), pWeight.end(), 0.0f);

	vector<bool> bDrawn(nMosaicImageArea);
	std::fill(bDrawn.begin(), bDrawn.end(), false);

	m_result.emplace<StlImage<float>>(StlImage<float>());
	m_result = StlImage<float>();
	assert(std::holds_alternative<StlImage<float>>(m_result));	//nahezu unmöglich, das ein anderer Datentyp bei diesem Aufruf in der Variable ist
	std::get<StlImage<float>>(m_result).Alloc(MosaicImageSize);

	long nMaxSize_y = std::min(m_SingleImageSize.y, static_cast<long long>(m_InterpolatedPosX.Rows()) - 1);

	for (int kx = 0; kx < m_nNumberOfFrames.x; kx++) /*(pNumberOfFrames.x ) = Anzahl der Zeilen von m_nUsedPic*/
	{
		for (int ky = 0; ky < m_nNumberOfFrames.y; ky++)
		{
			if (m_nUsedPic[kx][ky] < 0 || m_nUsedPic[kx][ky] > static_cast<INT_PTR>(Images.size()))
				continue;

			size_t nB = static_cast<size_t>(m_nUsedPic[kx][ky]);

			long nXNull = CMathTools::RoundValue(static_cast<double>(m_fSubSize.x * kx));
			long nYNull = CMathTools::RoundValue(static_cast<double>(m_fSubSize.y * ky));



			long nxSb = kx > 0 ? m_Parameters.nEdgeBlendingLength : 0;
			long nxSe = kx < m_nNumberOfFrames.x - 1 ? m_Parameters.nEdgeBlendingLength : 0;
			long nySb = ky > 0 ? m_Parameters.nEdgeBlendingLength : 0;
			long nySe = ky < m_nNumberOfFrames.y - 1 ? m_Parameters.nEdgeBlendingLength : 0;


			auto& pData = (*Images.at(nB));


			double yStart = static_cast<double>(nYNull - m_InterpolatedPosY[0][nB] + m_MinimumMeanImagePosition.y);

			long nYstart = static_cast<long>(floor(yStart));

			/* Auf das Raster sollten wenigstens 8 Bildzeilen entfallen, sonst nächstes Raster*/
			if (nYstart + ceill(m_fSubSize.y) >= 8 && nYstart + 8 < m_SingleImageSize.y)
			{
				/*yMin erste Zeile mit Daten im Quellbild*/
				long yMin = nYstart < 0 ? std::max(0l, 2 - nYstart) : 0;
				/*yMin letzte Zeile mit Daten im Quellbild*/
				long yMax = std::min((long long)ceill(m_fSubSize.y) - 1, m_SingleImageSize.y - nYstart);
				long nYa = nYstart + yMin - 1 - nySb;

				if (nYa < 0)
				{
					nYa = 0;
				}
				else if (nYa > static_cast<INT_PTR>(m_InterpolatedPosX.Cols() - s_FrameHeightDiff))
				{
					nYa = static_cast<long>(m_InterpolatedPosX.Cols() - s_FrameHeightDiff);
				}

				/*Werte ins Gesamtbild eintragen*/

				for (int nIndexY = yMin - nySb; nIndexY <= yMax + nySe; nIndexY++)
				{

					double fHelp;
					fHelp = nYstart + nIndexY - (m_InterpolatedPosY[nYa + 1][nB] - m_InterpolatedPosY[0][nB]);
					long nY = static_cast<long>(floor(fHelp));
					double qY = fHelp - nY;

					if (0 <= nY && nY < nMaxSize_y)
					{
						nYa = nY;
						double xStart = nXNull - m_InterpolatedPosX[0][nB] + m_MinimumMeanImagePosition.x
							- (m_InterpolatedPosX[nYa][nB] - m_InterpolatedPosX[0][nB]);

						long nXstart = static_cast<long>(floor(xStart));
						double qX = xStart - nXstart;


						double wIy = 1;

						if (nIndexY < m_Parameters.nEdgeBlendingLength)
						{
							wIy = (m_Parameters.nEdgeBlendingLength + nIndexY + 0.5) / (2 * m_Parameters.nEdgeBlendingLength);
						}

						if (nIndexY > ceill(m_fSubSize.y) - m_Parameters.nEdgeBlendingLength - 1)
						{
							wIy = (m_Parameters.nEdgeBlendingLength - (nIndexY - static_cast<double>(ceill(m_fSubSize.y))) - 0.5) / (2 * m_Parameters.nEdgeBlendingLength);
						}

						for (int nIndexX = -nxSb; nIndexX <= ceill(m_fSubSize.x) - 1 + nxSe; nIndexX++)
						{
							int x = nXNull + nIndexX;
							int y = nYNull + nIndexY;
							long nX = nXstart + nIndexX;

							double wIx = 1;

							if (nIndexX < m_Parameters.nEdgeBlendingLength)
							{
								wIx = (m_Parameters.nEdgeBlendingLength + nIndexX + 0.5) / (2 * m_Parameters.nEdgeBlendingLength);
							}

							if (nIndexX > ceill(m_fSubSize.x) - m_Parameters.nEdgeBlendingLength - 1)
							{
								wIx = (m_Parameters.nEdgeBlendingLength - (nIndexX - static_cast<double>(ceill(m_fSubSize.x))) - 0.5) / (2 * m_Parameters.nEdgeBlendingLength);
							}

							double wIr = wIx * wIy;

							if ((0 <= nX) && (nX < m_SingleImageSize.x - 2) && (0 <= nY) && (nY < m_SingleImageSize.y - 2))
							{
								if (wIr == 1)
								{
									pDest[(y + static_cast<int>(m_Parameters.nBorder / 2)) * MosaicImageSize.x + (x + static_cast<int>(m_Parameters.nBorder / 2))] = static_cast<float>((pData[{nX, nY + 1}] * (1 - qX)
										+ pData[{(nX + 1), (nY + 1)}] * qX)* qY
										+ (pData[{nX, nY}] * (1 - qX) + pData[{(nX + 1), nY}] * (qX))* (1 - qY));

									bDrawn[(y + static_cast<int>(m_Parameters.nBorder / 2)) * MosaicImageSize.x + (x + static_cast<int>(m_Parameters.nBorder / 2))] = true;
									pWeight[(y + static_cast<int>(m_Parameters.nBorder / 2)) * MosaicImageSize.x + (x + static_cast<int>(m_Parameters.nBorder / 2))] = 1;
								}
								else
								{
									if (bDrawn[(y + static_cast<int>(m_Parameters.nBorder / 2)) * MosaicImageSize.x + (x + static_cast<int>(m_Parameters.nBorder / 2))] == true)
									{
										pDest[(y + static_cast<int>(m_Parameters.nBorder / 2)) * MosaicImageSize.x + (x + static_cast<int>(m_Parameters.nBorder / 2))] +=
											static_cast<float>(((pData[{nX, (nY + 1)}] * (1 - qX)
												+ pData[{(nX + 1), (nY + 1)}] * qX)* qY
												+ (pData[{nX, nY}] * (1 - qX)
													+ pData[{(nX + 1), nY}] * (qX))* (1 - qY))
												* wIr);
									}
									else
									{
										pDest[(y + static_cast<int>(m_Parameters.nBorder / 2)) * MosaicImageSize.x + (x + static_cast<int>(m_Parameters.nBorder / 2))] =
											static_cast<float>(((pData[{nX, (nY + 1)}] * (1 - qX)
												+ pData[{(nX + 1), (nY + 1)}] * qX)* qY
												+ (pData[{nX, nY}] * (1 - qX)
													+ pData[{(nX + 1), nY}] * (qX))* (1 - qY))
												* wIr);

										bDrawn[(y + static_cast<int>(m_Parameters.nBorder / 2)) * MosaicImageSize.x + (x + static_cast<int>(m_Parameters.nBorder / 2))] = true;
									}
									pWeight[(y + static_cast<int>(m_Parameters.nBorder / 2)) * MosaicImageSize.x + (x + static_cast<int>(m_Parameters.nBorder / 2))] += static_cast<float>(wIr);
								}
							}
						}

					}
				}
			}
		}
	}
	for (size_t nIndex = 0; nIndex < nMosaicImageArea; nIndex++)
	{
		if (pWeight[nIndex] != 0)
		{
			pDest[nIndex] = pDest[nIndex] / pWeight[nIndex];
		}
		pWeight[nIndex] *= 100;
	}

	std::get<StlImage<float>>(m_result).Put(&pDest[0]);
	//std::get<StlImage<float>>(m_result).Put2d(0, 0, MosaicImageSize.x, MosaicImageSize.y, reinterpret_cast<unsigned char *>(pDest.data()));
}
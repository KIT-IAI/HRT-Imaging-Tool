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
#include "Compositing.h"

#include "ImageUndistortion.h"



CCompositing::CCompositing(const CCompositingParameters& Param)
	:m_Parameters(Param)
{
}

void CCompositing::SetParameters(const CCompositingParameters& Param)
{
	m_Parameters = Param;
}

const CSNPCompositingResult* CCompositing::GetResult() const
{
	return &m_result;
}

size_t CCompositing::GetResultImageArea() const
{
	if (std::holds_alternative<StlImage<float>>(m_result))
	{
		const StlImage<float>& result = std::get<StlImage<float>>(m_result);
		if (m_Parameters.nBorder == 0 || m_Parameters.cBackgroundColor == 0)
			return result.NumberOfNonzeroPixels();

		auto childSizeX = static_cast<int>(result.GetSize().x - (m_Parameters.nBorder * 2));
		auto childSizeY = static_cast<int>(result.GetSize().y - (m_Parameters.nBorder * 2));

		StlImage<float> child;
		child.Alloc({ childSizeX, childSizeY });
		child.Copy2d(StlImageRect({ 0,0 , childSizeX, childSizeY }), result, { static_cast<int>(m_Parameters.nBorder), static_cast<int>(m_Parameters.nBorder) });
		return child.NumberOfNonzeroPixels();
	}
	else
	{
		return 0;
	}
}

std::unique_ptr<CSNPCompositingResult> CCompositing::Fuse(const CImageRegistrationData& ImageData, const CDenseMatrix& PositioningSolution)
{
	PerformCompositing(ImageData, PositioningSolution);
	std::unique_ptr<CSNPCompositingResult> Result = std::make_unique<CSNPCompositingResult>();
	/*if (std::holds_alternative<CMeasureImage>(m_result)) {
		*Result = CMeasureImage();
		std::get<CMeasureImage>(*Result).Clone(std::get<CMeasureImage>(*GetResult()));
	}*/
	if (std::holds_alternative<C3DCloud>(m_result)) {
		*Result = C3DCloud();
		std::get<C3DCloud>(*Result).Clone(std::get<C3DCloud>(*GetResult()));
	}
	if (std::holds_alternative<C3DBuffer<float>>(m_result)) {
		*Result = C3DBuffer<float>();
		std::get<C3DBuffer<float>>(*Result) = (std::get<C3DBuffer<float>>(*GetResult()));
	}
	if (std::holds_alternative<StlImage<float>>(m_result)) {
		*Result = StlImage<float>(std::get<StlImage<float>>(*GetResult()));
	}
	return Result;
}

/*	\brief Berechnet die mittleren Verschiebungswerte von \a posX und \a posY
 *	und schreibt sie in die Membervariable \a m_MeanImagePosition.
 *
 *	\param[in] posX Die Verschiebungswerte in x-Richtung aller Bilder und
 *		Teilbilder.
 *	\param[in] posY Die Verschiebungswerte in y-Richtung aller Bilder und
 *		Teilbilder.
 *
 *	\author Robert Paasche
 *
 *	\see m_MeanImagePosition
 */
void CCompositing::SetMeanImagePosition(const CDenseMatrix& posX, const CDenseMatrix& posY)
{
	assert((posX.Cols() == posY.Cols()));

	m_MeanImagePosition.AllocMatrix(posX.Cols(), 2);

	for (size_t nCol = 0; nCol < posX.Cols(); nCol++)
	{
		m_MeanImagePosition[nCol][0] = posX.Col(nCol).Mean();
		m_MeanImagePosition[nCol][1] = posY.Col(nCol).Mean();
	}
}

/**
 * Spaltet die Solution bildweise.
 * Die Ausgabematrix posx beinhaltet für jedes Bild eine Spalte mit Anzahl Zeilen = Teilbildanzahl Werten.
 */
void CCompositing::SetMatrix(const CDenseMatrix& Solution, size_t nImages, CDenseMatrix& posX, CDenseMatrix& posY)
{
	assert(Solution.Rows() % nImages == 0);
	assert(Solution.Rows() > 0);
	assert(Solution.Cols() == 2);
	assert(nImages > 0);

	size_t nSubImagesPerImage = Solution.Rows() / nImages;

	posX = CDenseMatrix(nSubImagesPerImage, nImages);
	posY = CDenseMatrix(nSubImagesPerImage, nImages);

	size_t nSubImage = 0;
	for (size_t nSizeX = 0; nSizeX < nImages; nSizeX++)
	{
		for (size_t nSizeY = 0; nSizeY < nSubImagesPerImage; nSizeY++)
		{
			assert(nSubImage >= 0 && nSubImage < Solution.Rows());

			posX[nSizeY][nSizeX] = Solution[nSubImage][0];
			posY[nSizeY][nSizeX] = Solution[nSubImage][1];

			nSubImage++;
		}
	}
}

void CCompositing::OnCancel()
{
}

void CCompositing::OnUncancel()
{
}

CProgress CCompositing::GetProgress()
{
	return CProgress();
}

void CCompositing::SetSingleImageSize(const StlImageSize& szSize)
{
	m_SingleImageSize = szSize;
}

/**	\brief Setzt die Ergebnisse der globalen Positionsbestimmung (absolute
 *	Verzerrungen pro Teilbild) für die Bildmontage.
 *
 *	\param[in] PositioningSolution Die Matrix mit den Ergebnissen der globalen
 *		Positionsbestimmung.
 *	\param[in] nImageCount Die Anzahl der Bilder.
 *	\param[in] SubImageHeight Die Teilbildhöhe.
 */
void CCompositing::ProcessSolutionMatrix(const CDenseMatrix& solutionMatrix, size_t nImageCount)
{
	CDenseMatrix posX(solutionMatrix.Col(0), false);
	CDenseMatrix posY(solutionMatrix.Col(1), false);

	SetMatrix(solutionMatrix, nImageCount, posX, posY);

	m_InterpolatedPosX = CImageUndistortion::InterpolateRowPositions(posX, m_Parameters.nSubImageHeight);
	m_InterpolatedPosY = CImageUndistortion::InterpolateRowPositions(posY, m_Parameters.nSubImageHeight);

	SetMeanImagePosition(posX, posY);
}

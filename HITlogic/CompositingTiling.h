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



#pragma once
#include "Compositing.h"
class CCompositingTiling :
	public CCompositing
{
public:
	explicit CCompositingTiling(const CCompositingParameters& Param);

	void PerformCompositing(const CImageRegistrationData& ImageData, const CDenseMatrix& PositioningSolution);


private:
	void Combining(const std::vector<StlImage<float>*>& pImages, const CDenseMatrix& solutionMatrix);

	StlImageSize CalculateMosaicImageSize();
	void Cmps_Size_Selection(const StlImageSize& szSize);
	void SelectImageToUse();
	void GenerateImage(const std::vector<StlImage<float>*>& pImages, const StlImageSize& MosaicImageSize);



	/**	\brief Die Minimal- und Maximalwerte der Verzerrungsfunktionen.*/
	CVector2d<double> m_MinimumMeanImagePosition;
	CVector2d<double> m_MaximumMeanImagePosition;

	/**<enthält die Nummer der gewählten Bilder*/
	CDenseMatrix m_nUsedPic;

	/**	\brief Die Anzahl der Rastergitter in horizontaler und vertikaler Richtung.*/
	CVector2d<long> m_nNumberOfFrames;

	/**	\brief Die Größe eines Gitterrasters.*/
	CVector2d<double> m_fSubSize;


	static const int s_FrameHeightDiff;
	static const double s_FrameSizeFactor;


};


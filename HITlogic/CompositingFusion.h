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
#include "StlImage.h"

class CCompositingFusion : public CCompositing
{
public:

	explicit CCompositingFusion(const CCompositingParameters& Param);

	void PerformCompositing(const CImageRegistrationData& ImageData, const CDenseMatrix& PositioningSolution);

	const std::vector <CUndistortedImage>& GetUndistortedImages() const;
private:
	void Fusion(const CImageRegistrationData& ImageData, const CDenseMatrix& solutionMatrix);
	void FusionRigidRegistration(const CImageRegistrationData& ImageData, const CDenseMatrix& solutionMatrix);

	static size_t GetMonotonyViolationCount(const CDenseMatrix& matPosY);
	StlImage<float> GenerateWeightImage(const StlImage<float>& imgSource) const;
	StlImage<float> GenerateWeightImgCos2(const StlImageSize& ImageSize) const;
	StlImage<float> GenerateWeightImgDistance(const StlImage<float>& imgSource, size_t nEdgeBlendingLength = 10) const;
	std::vector <CUndistortedImage> m_UndistortedImages;

};

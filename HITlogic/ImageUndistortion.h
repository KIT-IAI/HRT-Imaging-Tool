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

#include "UndistortedImage.h"
#include "StlImage.h"

class CImageUndistortion
{
public:
	enum class EInterpolationMode
	{
		eLinear,
		eCubicSpline
	};

	CImageUndistortion(EInterpolationMode eInterpolationModeX = EInterpolationMode::eLinear, EInterpolationMode eInterpolationModeY = EInterpolationMode::eLinear);
	~CImageUndistortion() = default;

	CUndistortedImage UndistortImage(const StlImage<float>& Image, const CDenseMatrix& Distortion);

	static void CreateUndistortedImage(const StlImage<float>& imgSrc, const CDenseVector& vecPosX, const CDenseVector& vecPosY, EInterpolationMode eInterpolationModeX, EInterpolationMode eInterpolationModeY, StlImage<float>& imgDest, StlImageRect& posDest);
	static CDenseMatrix InterpolateRowPositions(const CDenseMatrix& SubImagePositions, size_t nSubImageHeight);

	EInterpolationMode m_eInterpolationModeX = EInterpolationMode::eLinear;
	EInterpolationMode m_eInterpolationModeY = EInterpolationMode::eLinear;

private:

	StlImageSize DetermineUndistortedImageSize(const CDenseVector& XPositions, const CDenseVector& YPositions, const StlImageSize& originalImageSize) const;

	static void MotionCorrectionXLinear(const StlImage<float>& imgSrc, const CDenseVector& vecPosX, StlImage<float>& imgDest, StlImageRect& posDest);
	static void MotionCorrectionXSpline(const StlImage<float>& imgSrc, const CDenseVector& vecPosX, StlImage<float>& imgDest, StlImageRect& posDest);
	static void MotionCorrectionYLinear(const StlImage<float>& imgSrc, const CDenseVector& vecPosX, const CDenseVector& vecPosY, size_t nFrameWidth, StlImage<float>& imgDest, StlImageRect& posDest);
	static void MotionCorrectionYSpline(const StlImage<float>& imgSrc, const CDenseVector& vecPosX, const CDenseVector& vecPosY, size_t nFrameWidth, StlImage<float>& imgDest, StlImageRect& posDest);


};


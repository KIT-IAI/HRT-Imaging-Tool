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
#include <list>
#include <mutex>
#include "StlImage.h"

class CScoreNormalizer
{
private:
	static const float s_fAreaNormalizationConstant;
	static const int s_nMaximumOverlapImagesToStore = 5;

public:

	CScoreNormalizer();
	explicit CScoreNormalizer(const CScoreNormalizer&) = delete;

	void CalculateScoreNorm1(const StlImage<float>& Phase_Image, StlImage<float>& Normalized_Image) const;

private:
	void NormToStandardDeviation(const StlImage<float>& Phase_Image, StlImage<float>& Normalized_Image) const;
	void NormToStandardDeviation(StlImage<float>& Normalized_Image) const;

/*	std::shared_ptr<CMeasureImage> InquireOverlappingPixelMask(CSize fft_size, CSize Image_Size1, CSize Image_Size2) const;
	void AllocMask(CMeasureImage& image, CSize fft_size, CSize image_size) const;
	void CreateFFTOfMask(CComplexImage& image, CSize fft_size, CSize image_size) const;
	bool HasBeenCalculated(std::pair<CSize, CSize> size_pair) const;
	void AddToList(std::shared_ptr<CMeasureImage> overlapping_pixels, std::pair<CSize, CSize> size_pair) const;
	void CalculateOverlappingPixels(CSize fft_size, std::pair<CSize, CSize> size_pair) const;
	std::shared_ptr<CMeasureImage> GetImageFor(std::pair<CSize, CSize> size_pair) const;
	void MoveToFrontOfList(std::pair<CSize, CSize> sizes) const;

	void NormToOverlappingArea(const CMeasureImage& Phase_Image, CMeasureImage& Normalized_Image, CSize Image_Size1, CSize Image_Size2) const;
	void NormToOverlappingArea(const CMeasureImage& Phase_Image, CMeasureImage& Normalized_Image, const CMeasureImage& OverlappingPixels) const;
	void GenerateOverlappingPixelMask(CMeasureImage& overlapping_pixels, CSize FFTSize, CSize image_size1, CSize image_size2) const;

	// These 3 Variabnles make sure the overlapping pixels are only calculated if they chagned from the last time this is called
	mutable std::list <
		std::pair <
		std::pair<CSize, CSize>,
		std::shared_ptr < CMeasureImage >
		>
	> m_OverlappingPixelImagesList;
	mutable std::mutex OverlappingPixelGenerationMutex; */
};


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
#include "StlImage.h"

class CHrtImageParameters {
public:
	CHrtImageParameters(StlImageSize ImageSize = StlImageSize(0, 0), size_t nSubImageHeight = 1)
		: ImageSize(ImageSize)
		, nSubImageHeight(nSubImageHeight)
	{
		nSubImagesPerImageWithoutGap = static_cast<size_t>(ceil(static_cast<double>(ImageSize.y) / nSubImageHeight));
		nGapsBeforeImage = nSubImagesPerImageWithoutGap / 6;
		nGapsAfterImage = nSubImagesPerImageWithoutGap / 6;
	}

	size_t nSubImageHeight = 1;
	size_t nSubImagesPerImageWithoutGap = 1;
	size_t nGapsBeforeImage = 0;
	size_t nGapsAfterImage = 0;
	size_t nColumnCount = 1;
	StlImageSize ImageSize;

	size_t GetSubImageCountWithGap() const
	{
		return nSubImagesPerImageWithoutGap + nGapsBeforeImage + nGapsAfterImage;
	}

	friend bool operator==(const CHrtImageParameters& lhs, const CHrtImageParameters& rhs)
	{
		return lhs.nSubImageHeight == rhs.nSubImageHeight
			&& lhs.nSubImagesPerImageWithoutGap == rhs.nSubImagesPerImageWithoutGap
			&& lhs.nGapsBeforeImage == rhs.nGapsBeforeImage
			&& lhs.nGapsAfterImage == rhs.nGapsAfterImage
			&& lhs.nColumnCount == rhs.nColumnCount
			&& lhs.ImageSize == rhs.ImageSize;
	}

	friend bool operator!=(const CHrtImageParameters& lhs, const CHrtImageParameters& rhs)
	{
		return !(lhs == rhs);
	}
};
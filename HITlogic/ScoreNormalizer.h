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

};

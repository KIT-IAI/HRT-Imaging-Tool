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
#include "ScoreNormalizer.h"



const float CScoreNormalizer::s_fAreaNormalizationConstant = 1/*std::numeric_limits<float>::denorm_min()*/;

CScoreNormalizer::CScoreNormalizer()
{
}

void CScoreNormalizer::CalculateScoreNorm1(const StlImage<float>& Phase_Image, StlImage<float>& Normalized_Image) const
{
	return NormToStandardDeviation(Phase_Image, Normalized_Image);
}

void CScoreNormalizer::NormToStandardDeviation(const StlImage<float>& Phase_Image, StlImage<float>& Normalized_Image) const
{
	float std2 = static_cast<float>(Phase_Image.StandardDeviation());
	Normalized_Image = Phase_Image / std2;
}

void CScoreNormalizer::NormToStandardDeviation(StlImage<float>& Normalized_Image) const
{
	return NormToStandardDeviation(Normalized_Image, Normalized_Image);
}

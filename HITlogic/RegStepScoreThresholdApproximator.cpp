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
#include "RegStepScoreThresholdApproximator.h"



CRegStepScoreThresholdApproximator::~CRegStepScoreThresholdApproximator()
{
}

bool CRegStepScoreThresholdApproximator::IsScoreThresholdSufficient(std::vector<CRegistrationResult>& RegistrationResults)
{
	auto allResiduals = CRegistrationPostProcessor::GetAllResiduals(RegistrationResults);
	double fMean = CResidual::CalculateMeanResidual(allResiduals);
	double fStdev = CResidual::CalculateStdevResidual(allResiduals, fMean);

	return fMean < m_fUpperLimitForResidualMean && fStdev < m_fUpperLimitForResidualMean;
}

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
#include "RegStepScoreThresholdOptimizer.h"


CRegStepScoreThresholdOptimizer::~CRegStepScoreThresholdOptimizer()
{
}


bool CRegStepScoreThresholdOptimizer::IsScoreThresholdSufficient(vector<CRegistrationResult>& RegistrationResults)
{
	auto allResiduals = CRegistrationPostProcessor::GetAllResiduals(RegistrationResults);
	double fMean = CResidual::CalculateMeanResidual(allResiduals);
	double fStdev = CResidual::CalculateStdevResidual(allResiduals, fMean);
	double fLastMean = m_fLastMean;
	double fLastStdev = m_fLastStd;

	m_nLastRemovedRegistrations = m_nLastRegistrationCount - RegistrationResults.size();
	m_nLastRegistrationCount = RegistrationResults.size();
	m_fLastMean = fMean;
	m_fLastStd = fStdev;

	if (fMean < 2 && fStdev < 2)
		return true;


	if (fLastMean == -1)
		return false;

	if (m_nLastRemovedRegistrations <= 0)
		return false;


	if (fLastMean - fMean > m_fEpsilon)
		return false;
	if (fLastStdev - fStdev > m_fEpsilon)
		return false;

	return true;
}

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
#include "RegistrationScoreParameters.h"
#include "RegStepScoreThresholdAdapter.h"

class CRegStepScoreThresholdOptimizer :
	public CScoreThresholdAdapter
{
public:
	CRegStepScoreThresholdOptimizer(CRegistrationScoreParameters ScoreParameters, size_t nImageCount, CSLESolver::EAlgorithm eSolverAlgorithm)
		: CScoreThresholdAdapter(ScoreParameters, nImageCount, eSolverAlgorithm)
	{
		m_fAdaptScoreThresholdBy = 0.01;
	}

	virtual ~CRegStepScoreThresholdOptimizer();

private:
	bool IsScoreThresholdSufficient(std::vector<CRegistrationResult>& RegistrationResults) override;

	double m_fLastMean = -1.0;
	double m_fLastStd = -1.0;

	size_t m_nLastRemovedRegistrations = 0;
	size_t m_nLastRegistrationCount = 0;

	const double m_fEpsilon = 0.01;
	const double m_fUpperLimitForResiduals = 3;

};


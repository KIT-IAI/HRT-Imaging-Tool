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

#include "RegistrationPostProcessor.h"
#include "RegistrationScoreParameters.h"
#include "ProcessType.h"

class CRegStepSubImageScoreThresholdAdapter :
	public CRegistrationPostProcessor
{

public:
	CRegStepSubImageScoreThresholdAdapter(CRegistrationScoreParameters ScoreParameters, size_t nImageCount, CSLESolver::EAlgorithm eSolverAlgorithm, CProcessType::EProcessType eProcessType, size_t nSubImageHeight)
		: m_nImageCount(nImageCount),
		m_eSolverAlgorithm(eSolverAlgorithm),
		m_eProcessType(eProcessType),
		m_ScoreParameters(ScoreParameters),
		m_nSubImageHeight(nSubImageHeight)
	{
	};

	void ProcessRegistrationData(std::vector<StlImage<float>*>& images, std::vector<CRegistrationResult>& validRegistrationResults, std::vector<CRegistrationResult>& invalidRegistrationResults, const std::vector<std::list<size_t>>& imagegroups) override;

	static void EnableDetailedLogging(bool bEnable = true);

private:
	void AdaptScoreThreshold();
	void CRegStepSubImageScoreThresholdAdapter::ChangeValidity(std::vector<CRegistrationResult>& validRegistrationResults);

	CDenseMatrix SolveFlexiblePositioning(const vector<StlImage<float>*>& images, const vector<CRegistrationResult>& validRegistrationResults, const vector<std::list<size_t>>& imagegroups) const;

	bool IsScoreThresholdSufficient(const std::vector<CRegistrationResult>& RegistrationResults);
	void CalculateStochasticValues(const std::vector<CResidual>& allResiduals);

protected:
	size_t m_nImageCount;
	CSLESolver::EAlgorithm m_eSolverAlgorithm;
	CRegistrationScoreParameters m_ScoreParameters;
	CProcessType::EProcessType m_eProcessType;
	size_t m_nSubImageHeight;

	double m_fAdaptScoreThresholdBy = 0.1;

	double pMean = 0;
	double pStdev = 0;
	double pMedian = 0;
	double p9Quantile = 0;
	double p99Quantile = 0;
	double p999Quantile = 0;
	double pMax = 0;

	size_t m_nLastRegistrationCount = 0;
	size_t m_nLastRemovedRegistrations = 0;

private:
	static bool s_bDetailedLogging;

};

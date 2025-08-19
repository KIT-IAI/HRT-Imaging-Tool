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

class CScoreThresholdAdapter :
	public CRegistrationPostProcessor
{
public:
	CScoreThresholdAdapter(CRegistrationScoreParameters ScoreParameters, size_t nImageCount, CSLESolver::EAlgorithm eSolverAlgorithm)
		: m_nImageCount(nImageCount),
		m_eSolverAlgorithm(eSolverAlgorithm),
		m_ScoreParameters(ScoreParameters) {
	};

	virtual void ProcessRegistrationData(std::vector<StlImage<float>*>& images, std::vector<CRegistrationResult>& validRegistrationResults, std::vector<CRegistrationResult>& invalidRegistrationResults, vector<std::list<size_t>>& imagegroups) override;

	static void EnableDetailedLogging(bool bEnable = true);

private:
	void AdaptScoreThreshold();
	void RemoveWrongRegistrations(std::vector<CRegistrationResult>& validRegistrationResults, std::vector<CRegistrationResult>& invalidRegistrationResults) const;
	bool IsThresholdSufficient(std::vector<CRegistrationResult>& RegistrationResults);
	bool IsMaximumThresholdReached() const;

	virtual bool IsScoreThresholdSufficient(std::vector<CRegistrationResult>& RegistrationResults) = 0;

protected:
	size_t m_nImageCount;
	CSLESolver::EAlgorithm m_eSolverAlgorithm;
	CRegistrationScoreParameters m_ScoreParameters;

	double m_fAdaptScoreThresholdBy = 0;

private:
	static bool s_bDetailedLogging;
};

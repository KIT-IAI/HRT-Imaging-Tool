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
#include "Log.h"
#include "RegStepScoreThresholdAdapter.h"

///	<summary> Specifies whether detailed logging of the score threshold computation process is enabled (default: <c>false</c>) </summary>
///	<remarks>
///	<para> If detailed logging is enabled, each score threshold adaptation iteration will be logged with the score threshold change, the number of removed registrations and the residual mean and standard deviation values. </para>
///	<para> The default value is <c>false</c>. </para>
///	</remarks>
bool CScoreThresholdAdapter::s_bDetailedLogging = false;

///	<summary> Enables detailed logging of the score threshold computation process </summary>
///	<param name="bEnable"> Specifies whether to enable (<c>true</c>) or disable (<c>false</c>) detailed logging (default: <c>true</c>) </param>
///	<remarks>
///	<para> If detailed logging is enabled, each score threshold adaptation iteration will be logged with the score threshold change, the number of removed registrations and the residual mean and standard deviation values. </para>
///	<para> The default value is <c>false</c>. </para>
///	</remarks>
void CScoreThresholdAdapter::EnableDetailedLogging(bool bEnable /* = true*/)
{
	s_bDetailedLogging = bEnable;
}

void CScoreThresholdAdapter::ProcessRegistrationData(vector<StlImage<float>*>& /*images*/, vector<CRegistrationResult>& validRegistrationResults, vector<CRegistrationResult>& invalidRegistrationResults)
{
	size_t nValidRegistrations = validRegistrationResults.size();
	auto fThreshold = m_ScoreParameters.GetScoreThreshold();

	auto pRigidSolution = std::make_shared<CDenseMatrix>(m_nImageCount, 2);

	CRegistrationPostProcessor::SolveRigidPositioning(validRegistrationResults, pRigidSolution, m_eSolverAlgorithm, m_nImageCount);
	CRegistrationPostProcessor::CalculateResiduals(validRegistrationResults, pRigidSolution);

	auto logStep = [](double fCurrentThreshold, double fPreviousThreshold, size_t nPreviousRegs, const vector<CRegistrationResult>& regResults)->void {
		auto allResiduals = CRegistrationPostProcessor::GetAllResiduals(regResults);
		double fMean = CResidual::CalculateMeanResidual(allResiduals);
		double fStdev = CResidual::CalculateStdevResidual(allResiduals, fMean);
		CLog::Log(CLog::eInformational, _T("ScoreThresholdAdapter"), L"Adapted score threshold from %f to %f and removed %d registrations, Residual stats: %f +- %f", fPreviousThreshold, fCurrentThreshold, nPreviousRegs - regResults.size(), fMean, fStdev);
		};

	if (s_bDetailedLogging)
	{
		auto allResiduals = CRegistrationPostProcessor::GetAllResiduals(validRegistrationResults);
		double fMean = CResidual::CalculateMeanResidual(allResiduals);
		double fStdev = CResidual::CalculateStdevResidual(allResiduals, fMean);
		CLog::Log(CLog::eInformational, _T("ScoreThresholdAdapter"), L"Initial score threshold at %f, Residual stats: %f +- %f", fThreshold, fMean, fStdev);
	}

	size_t nPreviousRegs = nValidRegistrations;
	double fPreviousThreshold = fThreshold;

	while (!IsScoreThresholdSufficient(validRegistrationResults) && !IsMaximumThresholdReached())
	{
		AdaptScoreThreshold();
		RemoveWrongRegistrations(validRegistrationResults, invalidRegistrationResults);
		CRegistrationPostProcessor::SolveRigidPositioning(validRegistrationResults, pRigidSolution, m_eSolverAlgorithm, m_nImageCount);
		CRegistrationPostProcessor::CalculateResiduals(validRegistrationResults, pRigidSolution);

		if (s_bDetailedLogging)
		{
			logStep(m_ScoreParameters.GetScoreThreshold(), fPreviousThreshold, nPreviousRegs, validRegistrationResults);
			nPreviousRegs = validRegistrationResults.size();
			fPreviousThreshold = m_ScoreParameters.GetScoreThreshold();
		}
	}

	CLog::Log(CLog::eInformational, _T("ScoreThresholdAdapter"), L"Adapted score threshold from %f to %f and removed %d registrations.", fThreshold, m_ScoreParameters.GetScoreThreshold(), nValidRegistrations - validRegistrationResults.size());
}

void CScoreThresholdAdapter::AdaptScoreThreshold()
{
	m_ScoreParameters.SetScoreThreshold(m_ScoreParameters.GetScoreThreshold() + m_fAdaptScoreThresholdBy);

	if (m_ScoreParameters.GetScoreThreshold() > m_ScoreParameters.fCertainScore)
		m_ScoreParameters.SetScoreThreshold(m_ScoreParameters.fCertainScore);
}

void CScoreThresholdAdapter::RemoveWrongRegistrations(vector<CRegistrationResult>& validRegistrationResults, vector<CRegistrationResult>& invalidRegistrationResults) const
{
	auto condition = [&](const CRegistrationResult& reg)
		{
			return reg.RigidRegistrationResult.GetScore() < m_ScoreParameters.GetScoreThreshold();
		};
	auto invalidOperation = [](CRegistrationResult& reg)
		{
			reg.RigidRegistrationResult.SetValidity(CHrtValidityCodes::eInvalidRoughCorrelationMinScore);
		};
	move_if(validRegistrationResults, invalidRegistrationResults, condition, invalidOperation);
}

bool CScoreThresholdAdapter::IsThresholdSufficient(vector<CRegistrationResult>& RegistrationResults)
{
	return !IsMaximumThresholdReached() && IsScoreThresholdSufficient(RegistrationResults);
}

bool CScoreThresholdAdapter::IsMaximumThresholdReached() const
{
	return m_ScoreParameters.GetScoreThreshold() >= m_ScoreParameters.fCertainScore;
}

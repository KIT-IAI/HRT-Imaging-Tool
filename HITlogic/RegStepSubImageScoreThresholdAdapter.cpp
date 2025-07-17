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
#include "RegStepSubImageScoreThresholdAdapter.h"
#include "HRTGlobalPositioning.h"
#include "SNPDatasetOutputParameters.h"
#include "SNPGroupResult.h"
#include <iostream>

///	<summary> Specifies whether detailed logging of the score threshold computation process is enabled (default: <c>false</c>) </summary>
///	<remarks>
///	<para> If detailed logging is enabled, each score threshold adaptation iteration will be logged with the score threshold change, the number of removed registrations and the residual mean and standard deviation values. </para>
///	<para> The default value is <c>false</c>. </para>
///	</remarks>
bool CRegStepSubImageScoreThresholdAdapter::s_bDetailedLogging = false;

///	<summary> Enables detailed logging of the score threshold computation process </summary>
///	<param name="bEnable"> Specifies whether to enable (<c>true</c>) or disable (<c>false</c>) detailed logging (default: <c>true</c>) </param>
///	<remarks>
///	<para> If detailed logging is enabled, each score threshold adaptation iteration will be logged with the score threshold change, the number of removed registrations and the residual mean and standard deviation values. </para>
///	<para> The default value is <c>false</c>. </para>
///	</remarks>
void CRegStepSubImageScoreThresholdAdapter::EnableDetailedLogging(bool bEnable /* = true*/)
{
	s_bDetailedLogging = bEnable;
}

void CRegStepSubImageScoreThresholdAdapter::ProcessRegistrationData(vector<StlImage<float>*>& images, vector<CRegistrationResult>& validRegistrationResults, vector<CRegistrationResult>& invalidRegistrationResults, vector<std::list<size_t>>& imagegroup)
{
	
	size_t nValidRegistrations = validRegistrationResults.size();
	auto fThreshold = m_ScoreParameters.GetScoreThreshold();

	std::shared_ptr<CDenseMatrix> solution = SolveFlexiblePositioning(images, validRegistrationResults, imagegroup);
	CRegistrationPostProcessor::CalculateSubImageResiduals(validRegistrationResults, solution);

	auto allResiduals = CRegistrationPostProcessor::GetAllResiduals(validRegistrationResults);

	// CSV-Ausgabe
	std::ofstream csv("C:\\Users\\bt3410\\Desktop\\residuals_stats.csv");
	csv << "Mean,Stdev,Median,Q90,Q99,Q99.9,Max\n";

	CalculateStochasticValues(allResiduals, csv);


	auto logStep = [&](double fCurrentThreshold, double fPreviousThreshold, size_t nPreviousRegs, const vector<CRegistrationResult>& regResults)->void {
		CLog::Log(CLog::eInformational, _T("ScoreThresholdAdapter"), L"Adapted score threshold from %f to %f and removed %d registrations, Residual stats: %f +- %f", fPreviousThreshold, fCurrentThreshold, nPreviousRegs - regResults.size(), pMean, pStdev, pMedian, p9Quantile, p99Quantile, p999Quantile, pMax);
	 };

	if (s_bDetailedLogging)
	{
	CLog::Log(CLog::eInformational, _T("SubImageScoreThresholdAdapter"), L"Initial score threshold at %f, Residual stats: %f +- %f", fThreshold, pMean, pStdev, pMedian, p9Quantile, p99Quantile, p999Quantile, pMax);
	}

	size_t nPreviousRegs = nValidRegistrations;
	double fPreviousThreshold = fThreshold;

	while (!ShouldTheAdapterStop(validRegistrationResults, csv) && !IsMaximumThresholdReached())
	{
		AdaptScoreThreshold();
		RemoveWrongRegistrations(validRegistrationResults, invalidRegistrationResults);
		std::shared_ptr<CDenseMatrix> solution = SolveFlexiblePositioning(images, validRegistrationResults, imagegroup);
		CRegistrationPostProcessor::CalculateSubImageResiduals(validRegistrationResults, solution);

		if (s_bDetailedLogging)
		{
			logStep(m_ScoreParameters.GetScoreThreshold(), fPreviousThreshold, nPreviousRegs, validRegistrationResults);
			nPreviousRegs = validRegistrationResults.size();
			fPreviousThreshold = m_ScoreParameters.GetScoreThreshold();
		}
	}
	
	csv.close();
	CLog::Log(CLog::eInformational, _T("SubImageScoreThresholdAdapter"), L"Adapted score threshold from %f to %f and removed %d registrations.", fThreshold, m_ScoreParameters.GetScoreThreshold(), nValidRegistrations - validRegistrationResults.size());
}


void CRegStepSubImageScoreThresholdAdapter::AdaptScoreThreshold()
{
	m_ScoreParameters.SetScoreThreshold(m_ScoreParameters.GetScoreThreshold() + m_fAdaptScoreThresholdBy);

	if (m_ScoreParameters.GetScoreThreshold() > m_ScoreParameters.fCertainScore)
		m_ScoreParameters.SetScoreThreshold(m_ScoreParameters.fCertainScore);
}

void CRegStepSubImageScoreThresholdAdapter::RemoveWrongRegistrations(vector<CRegistrationResult>& validRegistrationResults, vector<CRegistrationResult>& invalidRegistrationResults) const
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

bool CRegStepSubImageScoreThresholdAdapter::ShouldTheAdapterStop(vector<CRegistrationResult>& RegistrationResults, std::ofstream& csv)
{
	return !IsMaximumThresholdReached() && IsScoreThresholdSufficient(RegistrationResults, csv);
}


bool CRegStepSubImageScoreThresholdAdapter::IsMaximumThresholdReached() const
{
	return m_ScoreParameters.GetScoreThreshold() >= m_ScoreParameters.fCertainScore;
}


std::shared_ptr<CDenseMatrix> CRegStepSubImageScoreThresholdAdapter::SolveFlexiblePositioning(vector<StlImage<float>*>& images, vector<CRegistrationResult>& validRegistrationResults, vector<std::list<size_t>>& imagegroup)
{
	StlImage<float>* pImage = images[0];
	StlImageSize size = pImage->GetSize();
	size_t subImageHeight = 32;

	CImageRegistrationResult ImageRegistrationResult = CImageRegistrationResult(validRegistrationResults, imagegroup);
	CHrtImageParameters standardParameters(size, subImageHeight);

	CGlobalPositioningParameters stdPositioningParameters(CSLESolver::EAlgorithm::eCGALGLIBStartIteration, CProcessType::eHRTStreamRegistration);
	CHRTGlobalPositioning globalPositioning(stdPositioningParameters);

	std::shared_ptr<CDenseMatrix> solution = std::make_shared<CDenseMatrix>(globalPositioning.SolvePositioning(ImageRegistrationResult, standardParameters));

	return solution;
}

bool CRegStepSubImageScoreThresholdAdapter::IsScoreThresholdSufficient(std::vector<CRegistrationResult>& RegistrationResults, std::ofstream& csv)
{
	auto allResiduals = CRegistrationPostProcessor::GetAllResiduals(RegistrationResults);
	CalculateStochasticValues(allResiduals, csv);

	return pMax = 50;
}



void CRegStepSubImageScoreThresholdAdapter::CalculateStochasticValues(std::vector<CResidual> allResiduals, std::ofstream& csv)
{
	pMean = CResidual::CalculateMeanResidual(allResiduals);
	pStdev = CResidual::CalculateStdevResidual(allResiduals, pMean);
	pMedian = CResidual::CalculateMedianResidual(allResiduals);
	p9Quantile = CResidual::CalculateQuantileResidual(allResiduals, 0.90);
	p99Quantile = CResidual::CalculateQuantileResidual(allResiduals, 0.99);
	p999Quantile = CResidual::CalculateQuantileResidual(allResiduals, 0.999);
	pMax = CResidual::CalculateMaximumResidual(allResiduals);


	// CSV-Ausgabe
	csv << pMean << ","
		<< pStdev << ","
		<< pMedian << ","
		<< p9Quantile << ","
		<< p99Quantile << ","
		<< p999Quantile << ","
		<< pMax << "\n";
}


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

#include <iostream>

#include "Log.h"
#include "RegStepSubImageScoreThresholdAdapter.h"
#include "HRTGlobalPositioning.h"
#include "SNPDatasetOutputParameters.h"
#include "SNPGroupResult.h"

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

void CRegStepSubImageScoreThresholdAdapter::ProcessRegistrationData(std::vector<StlImage<float>*>& images, std::vector<CRegistrationResult>& validRegistrationResults, std::vector<CRegistrationResult>& invalidRegistrationResults, const vector<std::list<size_t>>& imagegroups)
{
	m_ScoreParameters.SetScoreThreshold(m_ScoreParameters.fMinScoreFlexible); 
	size_t counterForRemovedRegs = 0;

	std::shared_ptr<CDenseMatrix> solution = std::make_shared<CDenseMatrix>(SolveFlexiblePositioning(images, validRegistrationResults, imagegroups));
	CRegistrationPostProcessor::CalculateSubImageResiduals(validRegistrationResults, solution, m_nSubImageHeight);
	auto allResiduals = CRegistrationPostProcessor::GetSubImageResiduals(validRegistrationResults);
	CalculateStochasticValues(allResiduals);

	//std::ofstream csv("\\residual_stats.csv");
	//csv << "ScoreThreshold" << ";" << "Mean" << ";" << "Stdev" << ";" << "Median" << ";"
	//	<< "9Quantile" << ";" << "99Quantile" << ";" << "999Quantile" << ";" << "MaxResidual" << ";" << "LastRemovedRegistrations" << ";" << "LastRegistrationCount" << "\n";

	while (!IsScoreThresholdSufficient(validRegistrationResults))
	{
		AdaptScoreThreshold();
		ChangeValidity(validRegistrationResults);
		counterForRemovedRegs += m_nLastRemovedRegistrations;
		std::shared_ptr<CDenseMatrix> solution = std::make_shared<CDenseMatrix>(SolveFlexiblePositioning(images, validRegistrationResults, imagegroups));
		CRegistrationPostProcessor::CalculateSubImageResiduals(validRegistrationResults, solution, m_nSubImageHeight);
		auto allResiduals = CRegistrationPostProcessor::GetSubImageResiduals(validRegistrationResults);
		CalculateStochasticValues(allResiduals);

		//// CSV-Ausgabe
		//csv << std::to_string(m_ScoreParameters.GetScoreThreshold()) << ";"
		//	<< std::to_string(pMean) << ";"
		//	<< std::to_string(pStdev) << ";"
		//	<< std::to_string(pMedian) << ";"
		//	<< std::to_string(p9Quantile) << ";"
		//	<< std::to_string(p99Quantile) << ";"
		//	<< std::to_string(p999Quantile) << ";"
		//	<< std::to_string(pMax) << ";"
		//	<< m_nLastRemovedRegistrations << ";"
		//	<< m_nLastRegistrationCount << "\n";
	}

	//csv.close();

	std::wostringstream oss;
	oss.setf(std::ios::fixed);
	oss.precision(3);
	oss << L"Adapted score threshold to " << m_ScoreParameters.GetScoreThreshold()  << " and removed " << counterForRemovedRegs << L" registrations, max. Residual: " << pMax;
	CLog::Log(CLog::eInformational, _T("SubImageScoreThresholdAdapter"), oss.str().c_str());
}

void CRegStepSubImageScoreThresholdAdapter::AdaptScoreThreshold()
{
	double new_threshold = m_ScoreParameters.GetScoreThreshold() + m_fAdaptScoreThresholdBy;

	m_ScoreParameters.SetScoreThreshold(new_threshold);
}

void CRegStepSubImageScoreThresholdAdapter::ChangeValidity(std::vector<CRegistrationResult>& validRegistrationResults)
{
	size_t invalidCount = 0;
	size_t validCount = 0;

	for (auto& registration : validRegistrationResults)
	{
		for (auto& flexRes : registration.FlexibleRegistrationResults)
		{
			if (flexRes.GetScore() < m_ScoreParameters.GetScoreThreshold())
			{
				if (flexRes.GetValidity() > 0)
				{
					++invalidCount;
				}
				flexRes.SetValidity(CHrtValidityCodes::eInvalidRoughCorrelationMinScore);
			}
			else
			{
				++validCount;
			}
		}
	}
	m_nLastRemovedRegistrations = invalidCount;
	m_nLastRegistrationCount = validCount;
}

CDenseMatrix CRegStepSubImageScoreThresholdAdapter::SolveFlexiblePositioning(const vector<StlImage<float>*>& images, const vector<CRegistrationResult>& validRegistrationResults, const vector<std::list<size_t>>& imagegroups) const
{
	StlImage<float>* pImage = images[0];
	StlImageSize size = pImage->GetSize();

	CHrtImageParameters standardParameters(size, m_nSubImageHeight);
	CGlobalPositioningParameters stdPositioningParameters(m_eSolverAlgorithm, m_eProcessType);
	CHRTGlobalPositioning globalPositioning(stdPositioningParameters);
	CImageRegistrationResult ImageRegistrationResult = CImageRegistrationResult(validRegistrationResults, imagegroups);

	return globalPositioning.SolvePositioning(ImageRegistrationResult, standardParameters);
}

bool CRegStepSubImageScoreThresholdAdapter::IsScoreThresholdSufficient(const std::vector<CRegistrationResult>& RegistrationResults)
{
	if (pMax > 200)
	{
		m_fAdaptScoreThresholdBy = 1;
		return false;
	}

	if (pMax > 50 && pMax <= 200)
	{
		m_fAdaptScoreThresholdBy = 0.1;
		return false;
	}

	if (pMax > m_ScoreParameters.fResidualThreshold && pMax <= 50)
	{
		m_fAdaptScoreThresholdBy = 0.01;
		return false;
	}

	return true;
}

void CRegStepSubImageScoreThresholdAdapter::CalculateStochasticValues(const std::vector<CResidual>& allResiduals)
{
	//pMean = CResidual::CalculateMeanResidual(allResiduals);
	//pStdev = CResidual::CalculateStdevResidual(allResiduals, pMean);
	//pMedian = CResidual::CalculateMedianResidual(allResiduals);
	//p9Quantile = CResidual::CalculateQuantileResidual(allResiduals, 0.90);
	//p99Quantile = CResidual::CalculateQuantileResidual(allResiduals, 0.99);
	//p999Quantile = CResidual::CalculateQuantileResidual(allResiduals, 0.999);
	pMax = CResidual::CalculateMaximumResidual(allResiduals);
}

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
#include "RegStepResidualFilter.h"

CRegStepResidualFilter::~CRegStepResidualFilter()
{
}

void CRegStepResidualFilter::ProcessRegistrationData(std::vector<StlImage<float>*>& images, std::vector<CRegistrationResult>& validRegistrationResults, std::vector<CRegistrationResult>& invalidRegistrationResults, vector<std::list<size_t>>& imagegroups)
{
	size_t validRegistrationCount = validRegistrationResults.size();
	auto pRigidSolution = std::make_shared<CDenseMatrix>(m_nImageCount, 2);

	CRegistrationPostProcessor::SolveRigidPositioning(validRegistrationResults, pRigidSolution, m_eSolverAlgorithm, m_nImageCount);
	CRegistrationPostProcessor::CalculateResiduals(validRegistrationResults, pRigidSolution);

	while (!AreResidualsAcceptable(validRegistrationResults))
	{
		RemoveWrongRegistrations(validRegistrationResults, invalidRegistrationResults);
		CRegistrationPostProcessor::SolveRigidPositioning(validRegistrationResults, pRigidSolution, m_eSolverAlgorithm, m_nImageCount);
		CRegistrationPostProcessor::CalculateResiduals(validRegistrationResults, pRigidSolution);
	}
	CLog::Log(CLog::eInformational, _T("ResidualFilter"), L"Removed %d registration due to a high residual.", validRegistrationCount - validRegistrationResults.size());
}

bool CRegStepResidualFilter::AreResidualsAcceptable(vector<CRegistrationResult>& RegistrationResults) const
{
	ASSERT(CRegistrationPostProcessor::AreResidualsInitialized(RegistrationResults));

	auto condition = [&](const CRegistrationResult& RegResult) {return RegResult.GetResidual() > m_fMaximumResidual; };
	return none_of(RegistrationResults.begin(), RegistrationResults.end(), condition);
}

void CRegStepResidualFilter::RemoveWrongRegistrations(vector<CRegistrationResult>& validRegistrationResults, vector<CRegistrationResult>& invalidRegistrationResults) const
{
	vector<CRegistrationResult> RemovedRegistrations;

	auto condition = [&](const CRegistrationResult& reg)
		{
			return (reg.GetResidual() > m_fMaximumResidual);
		};
	auto invalidOperation = [&](CRegistrationResult& reg)
		{
			reg.RigidRegistrationResult.SetValidity(CHrtValidityCodes::eInvalidDueToHighResidual);
			RemovedRegistrations.push_back(reg);
		};

	move_if(validRegistrationResults, invalidRegistrationResults, condition, invalidOperation);
	PossiblyInvalidateImages(RemovedRegistrations, validRegistrationResults, invalidRegistrationResults);
}
void CRegStepResidualFilter::PossiblyInvalidateImages(vector<CRegistrationResult>& RemovedRegistrations, vector<CRegistrationResult>& validRegistrationResults, vector<CRegistrationResult>& invalidRegistrationResults) const
{
	auto RegistrationsForEachImage = GetRegistrationsForEachImage(validRegistrationResults);
	for (auto registration : RemovedRegistrations)
	{
		const auto& rigidRegistration = registration.RigidRegistrationResult;
		size_t nRegistrationCountRef = RegistrationsForEachImage[rigidRegistration.GetReferenceImageIndex()].size();
		size_t nRegistrationCountTemp = RegistrationsForEachImage[rigidRegistration.GetTemplateImageIndex()].size();

		if (nRegistrationCountRef == 1)
		{
			auto questionableRegistration = RegistrationsForEachImage[rigidRegistration.GetReferenceImageIndex()][0];
			if (questionableRegistration.eClassification < EClassification::eCorrect)
				CRegistrationPostProcessor::RemoveRegistration(questionableRegistration, validRegistrationResults, invalidRegistrationResults);
		}
		if (nRegistrationCountTemp == 1)
		{
			auto questionableRegistration = RegistrationsForEachImage[rigidRegistration.GetTemplateImageIndex()][0];
			if (questionableRegistration.eClassification < EClassification::eCorrect)
				CRegistrationPostProcessor::RemoveRegistration(questionableRegistration, validRegistrationResults, invalidRegistrationResults);
		}

	}
}


vector<vector<CRegistrationResult>> CRegStepResidualFilter::GetRegistrationsForEachImage(vector<CRegistrationResult>& validRegistrationResults) const
{
	vector<vector<CRegistrationResult>> result;

	result.assign(m_nImageCount, vector<CRegistrationResult>());
	for (auto registration : validRegistrationResults)
	{
		result[registration.RigidRegistrationResult.GetReferenceImageIndex()].push_back(registration);
		result[registration.RigidRegistrationResult.GetTemplateImageIndex()].push_back(registration);
	}
	return result;
}
vector<double> CRegStepResidualFilter::GetResidualsForEachImage(vector<CRegistrationResult>& validRegistrationResults) const
{
	auto RegistrationsForEachImage = GetRegistrationsForEachImage(validRegistrationResults);
	vector<double> ResidualsForEachImage;

	ResidualsForEachImage.assign(m_nImageCount, 0.0);

	for (size_t i = 0; i < ResidualsForEachImage.size(); i++)
	{
		auto& registrationsForImage = RegistrationsForEachImage[i];
		auto Residuals = CRegistrationPostProcessor::GetAllResiduals(registrationsForImage);
		auto fMeanResidual = CResidual::CalculateMeanResidual(Residuals);

		ResidualsForEachImage[i] = fMeanResidual;
	}
	return ResidualsForEachImage;
}



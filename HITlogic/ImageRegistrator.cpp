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
#include "ImageRegistrator.h"

#include "RegStepFlexibleRegistration.h"
#include "RegStepImageScalation.h"
#include "RegStepResidualFilter.h"
#include "RegStepRigidRegistration.h"
#include "RegStepScoreThresholdApproximator.h"
#include "RegStepScoreThresholdOptimizer.h"
#include "RegStepSubImageScoreThresholdAdapter.h"
#include "RegStepZeroOffsetFilter.h"



CImageRegistrator::CImageRegistrator(CRegistrationProcedureParameters registration_parameters)
	:m_Parameters(registration_parameters)
{
}

std::shared_ptr<IComputable<CImageRegistrationData, CImageRegistrationResult>> CImageRegistrator::CreateProcessingPipeline(size_t nImageCount)
{
	auto RigidImageScalator = std::make_shared<CRegStepImageScalation>(m_Parameters.fScaleReduction);
	auto FlexibleImageScalator = std::make_shared<CRegStepImageScalation>(m_Parameters.fSubImageScaleReduction);

	auto RigidScaledParameters = m_Parameters.GetScaledParameters(CRegistrationProcedureParameters::ScalationType::eRigid);
	auto FlexibleScaledParameters = m_Parameters.GetScaledParameters(CRegistrationProcedureParameters::ScalationType::eSubimage);

	auto RigidRegistration = std::make_shared<CRegStepRigidRegistration>(RigidScaledParameters);
	auto ZeroOffsetFilter = std::make_shared<CRegStepZeroOffsetFilter>();
	auto ScoreApproximator = std::make_shared<CRegStepScoreThresholdApproximator>(RigidScaledParameters.ScoreParameters, nImageCount, RigidScaledParameters.GetSolverAlgorithm());
	auto ResidualFilter = std::make_shared<CRegStepResidualFilter>(nImageCount, RigidScaledParameters.VerificationParameters.eSolverAlgorithm);
	auto ScoreOptimizer = std::make_shared<CRegStepScoreThresholdOptimizer>(RigidScaledParameters.ScoreParameters, nImageCount, RigidScaledParameters.GetSolverAlgorithm());

	auto FlexibleRegistration = std::make_shared<CRegStepFlexibleRegistration>(FlexibleScaledParameters);
	auto FlexibleThresholdAdapter = std::make_shared<CRegStepSubImageScoreThresholdAdapter>(FlexibleScaledParameters.ScoreParameters, nImageCount, FlexibleScaledParameters.GetSolverAlgorithm(), FlexibleScaledParameters.GetProcessType(), m_Parameters.CorrelationParameters.nSubImageHeight);

	if (m_Parameters.fScaleReduction != 1)
	{
		RigidRegistration->AddPreceedingComputationStep(RigidImageScalator);
		RigidRegistration->AddSubsequentComputationStep(RigidImageScalator);
	}
	if (m_Parameters.fSubImageScaleReduction != 1)
	{
		FlexibleRegistration->AddPreceedingComputationStep(FlexibleImageScalator);
		FlexibleRegistration->AddSubsequentComputationStep(FlexibleImageScalator);
	}

	RigidRegistration->AddSubsequentComputationStep(ZeroOffsetFilter);

	if (m_Parameters.bAutomaticThresholdDetection && m_Parameters.GetProcessType() == CProcessType::eRigidRegistration)
		RigidRegistration->AddSubsequentComputationStep(ScoreApproximator);

	if (m_Parameters.bConsistencyCheck && m_Parameters.GetProcessType() == CProcessType::eRigidRegistration)
		RigidRegistration->AddSubsequentComputationStep(ResidualFilter);

	if (m_Parameters.bAutomaticThresholdDetection && m_Parameters.GetProcessType() == CProcessType::eRigidRegistration)
		RigidRegistration->AddSubsequentComputationStep(ScoreOptimizer);

	if (m_Parameters.GetProcessType() != CProcessType::eRigidRegistration)
	{
		RigidRegistration->AddSubsequentComputationStep(FlexibleRegistration);
		if (m_Parameters.bAutomaticThresholdDetection)
			FlexibleRegistration->AddSubsequentComputationStep(FlexibleThresholdAdapter);
	}

	return RigidRegistration;
}

CImageRegistrationResult CImageRegistrator::RegisterImages(CImageRegistrationData registrationData)
{
	if (registrationData.Images.empty())
		throw std::invalid_argument("No images for registration specified.");

	m_pCurrentPipeline = CreateProcessingPipeline(registrationData.Images.size());
	m_pCurrentPipeline->AddProgressListener(this);


	CImageRegistrationResult regResults;

	m_pCurrentPipeline->Compute(registrationData, regResults);
	regResults.RecalulateImageGroups();

	return regResults;
}

void CImageRegistrator::OnCancel()
{
	if (m_pCurrentPipeline != nullptr)
		m_pCurrentPipeline->Cancel();
}

void CImageRegistrator::OnUncancel()
{
	if (m_pCurrentPipeline != nullptr)
		m_pCurrentPipeline->OnUncancel();
}

CProgress CImageRegistrator::GetProgress()
{
	return m_CurrentProgress;
}

void CImageRegistrator::OnProgress(CProgress progress)
{
	m_CurrentProgress = progress;
	ReportProgress();
}

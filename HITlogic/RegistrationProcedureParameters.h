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
#include "RegistrationStrategy.h"
#include "CorrelationParameters.h"
#include "RegistrationScoreParameters.h"
#include "ImagePairVerificator.h"
#include "SLESolver.h"

class CRegistrationProcedureParameters
{
public:
	enum class ScalationType
	{
		eRigid,
		eSubimage
	};

	CRegistrationProcedureParameters()
	{
		StrategyParameters.ScoreParameters = ScoreParameters;
		StrategyParameters.eProcessType = eProcessType;
	}

	ERegistrationStrategy eRegistrationStrategy = ERegistrationStrategy::eBackForth;
	CRegistrationStrategyParameters StrategyParameters;

	EImagePairVerificator ePairVerificator = EImagePairVerificator::eNone;
	CRegistrationVerificationParameters VerificationParameters;

	CCorrelationParameters CorrelationParameters;

	CRegistrationScoreParameters ScoreParameters;

	double fSubImageScaleReduction = 1;
	double fScaleReduction = 1;
	bool bAutomaticThresholdDetection = true;
	bool bConsistencyCheck = true;

	CRegistrationProcedureParameters GetScaledParameters(ScalationType type) const;


	void SetProcessType(CProcessType::EProcessType eProcessType);
	void SetSolverAlgorithm(CSLESolver::EAlgorithm eAlgorithm);
	CProcessType::EProcessType GetProcessType() const;
	CSLESolver::EAlgorithm GetSolverAlgorithm() const;

private:
	CProcessType::EProcessType eProcessType = CProcessType::eHRTStreamRegistration;
	CSLESolver::EAlgorithm eSolverAlgorithm = CSLESolver::EAlgorithm::eCGALGLIB;
};


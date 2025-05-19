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
#include "RegistrationProcedureParameters.h"

/*
This method is necessary in order to adjust the ImageSize and SubImageHeight parameters of subsequent calculations
*/
CRegistrationProcedureParameters CRegistrationProcedureParameters::GetScaledParameters(ScalationType type) const
{
	double fScalationFactor = 1 / (type == ScalationType::eRigid ? fScaleReduction : fSubImageScaleReduction);

	StlImageSize UnscaledImageSize = StrategyParameters.ImageSize;
	size_t nUnscaledSubimageHeight = CorrelationParameters.nSubImageHeight;

	StlImageSize ScaledImageSize = StlImageSize(static_cast<long>(UnscaledImageSize.x * fScalationFactor), static_cast<long>(UnscaledImageSize.y * fScalationFactor));
	size_t nScaledSubimageHeight = static_cast<size_t>(nUnscaledSubimageHeight * fScalationFactor);

	CRegistrationProcedureParameters ScaledParameters(*this);
	ScaledParameters.fScaleReduction = 1;
	ScaledParameters.fSubImageScaleReduction = 1;

	ScaledParameters.StrategyParameters.ImageSize = ScaledImageSize;
	ScaledParameters.VerificationParameters.ImageSize = ScaledImageSize;
	ScaledParameters.CorrelationParameters.nSubImageHeight = nScaledSubimageHeight;

	return ScaledParameters;
}

void CRegistrationProcedureParameters::SetProcessType(CProcessType::EProcessType ProcessType)
{
	this->StrategyParameters.eProcessType = ProcessType;
	this->CorrelationParameters.eProcessType = ProcessType;
	this->eProcessType = ProcessType;
}
void CRegistrationProcedureParameters::SetSolverAlgorithm(CSLESolver::EAlgorithm SolverAlgorithm)
{
	this->VerificationParameters.eSolverAlgorithm = SolverAlgorithm;
	this->eSolverAlgorithm = SolverAlgorithm;
}
CProcessType::EProcessType CRegistrationProcedureParameters::GetProcessType() const
{
	return eProcessType;
}
CSLESolver::EAlgorithm CRegistrationProcedureParameters::GetSolverAlgorithm() const
{
	return eSolverAlgorithm;
}
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
#include "Residual.h"


CResidual CResidual::CreateFromRegistration(const CRigidRegistrationResult& regResult, DPoint referenceImageCoordinates, DPoint templateImageCoordinates)
{
	double registrationDistance = regResult.GetRegistrationDistance();
	double solutionDistance = referenceImageCoordinates.distanceFrom(templateImageCoordinates);

	CResidual residual;
	residual.m_fValue = abs(solutionDistance - registrationDistance);
	residual.m_fX = abs(abs(regResult.GetX()) - abs(templateImageCoordinates.m_x - referenceImageCoordinates.m_x));
	residual.m_fY = abs(abs(regResult.GetY()) - abs(templateImageCoordinates.m_y - referenceImageCoordinates.m_y));
	residual.m_bIsInitialized = true;

	return residual;
}


CResidual::CResidual()
{
}


CResidual::~CResidual()
{
}
double CResidual::CalculateMeanResidual(vector<CResidual>& allResiduals)
{
	if (allResiduals.empty())
		return 0;

	double fMean = 0.0;
	for (auto residual : allResiduals)
		fMean += residual.GetValue();
	fMean /= allResiduals.size();
	return fMean;
}
double CResidual::CalculateStdevResidual(vector<CResidual>& allResiduals, double fMean)
{
	if (allResiduals.empty())
		return 0;

	double fStdev = 0.0;
	for (auto residual : allResiduals)
		fStdev += pow(residual.GetValue() - fMean, 2);
	fStdev = sqrt(fStdev / allResiduals.size());
	return fStdev;
}
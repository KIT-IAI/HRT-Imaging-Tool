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
#include "RigidRegistrationResult.h"
#include "Point.h"

class CResidual
{
public:
	CResidual();
	~CResidual();
	static CResidual CreateFromRegistration(const CRigidRegistrationResult& regResult, DPoint referenceImageCoordinates, DPoint templateImageCoordinates);
	static CResidual CreateFromSubimageRegistration(const CRigidRegistrationResult& reg, DPoint referenceImageCoordinates, DPoint templateImageCoordinates);

	static double CalculateMeanResidual(std::vector<CResidual>& allResiduals);
	static double CalculateStdevResidual(std::vector<CResidual>& allResiduals, double fMean);


	double GetValue() const { return m_fValue; };
	double GetX() const { return m_fX; };
	double GetY() const { return m_fY; };
	bool IsInitialized() const { return m_bIsInitialized; };

	bool operator<(double value) const { return m_fValue < value; };
	bool operator<=(double value) const { return m_fValue <= value; };
	bool operator>(double value) const { return m_fValue > value; };
	bool operator>=(double value) const { return m_fValue >= value; };
	bool operator==(double value) const { return m_fValue == value; };
	bool operator!=(double value) const { return m_fValue != value; };
	bool operator<(const CResidual& value) const { return m_fValue < value.m_fValue; };
	bool operator<=(const CResidual& value) const { return m_fValue <= value.m_fValue; };
	bool operator>(const CResidual& value) const { return m_fValue > value.m_fValue; };
	bool operator>=(const CResidual& value) const { return m_fValue >= value.m_fValue; };
	bool operator==(const CResidual& value) const { return m_fValue == value.m_fValue; };
	bool operator!=(const CResidual& value) const { return m_fValue != value.m_fValue; };

private:

	double m_fValue = 0.0;
	double m_fX = 0.0;
	double m_fY = 0.0;
	bool m_bIsInitialized = false;
};


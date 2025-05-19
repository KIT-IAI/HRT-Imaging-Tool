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
#include "HrtValidityCodes.h"
#include "Point.h"

enum class EClassification
{
	eUninitialized,
	eInvalid,
	//eAssumablyWrong,
	eUnsure,
	//eProbablyCorrect,
	eCorrect
};

class CCorrelationOffset
{
private:
	static const int s_nZeroOffsetThreshold = 2;
public:
	CCorrelationOffset() {};
	virtual ~CCorrelationOffset() {};
	EClassification m_EClassification = EClassification::eUninitialized;

	float m_scoreNorm1 = 0.0;
	float m_scoreBeforeNorm = 0.0;

	float xOffset = 0.0;
	float yOffset = 0.0;

	DPoint m_MaximumPosition;

	DPoint ToPoint();

	void Inverse();

	virtual bool IsValid() const;
	virtual CHrtValidityCodes::EValidityCode GetValidity() const;
	virtual void SetValidity(CHrtValidityCodes::EValidityCode eValidity);
	bool IsZeroOffset() const;

	bool operator<(const CCorrelationOffset& rhs) const;
	bool operator<=(const CCorrelationOffset& rhs) const;
	bool operator>(const CCorrelationOffset& rhs) const;
	bool operator>=(const CCorrelationOffset& rhs) const;
	bool operator==(const CCorrelationOffset& rhs) const;
	bool operator!=(const CCorrelationOffset& rhs) const;
private:
	CHrtValidityCodes::EValidityCode m_eValidity = CHrtValidityCodes::EValidityCode::eValidityInitialization;
};
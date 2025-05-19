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
#include "CorrelationOffset.h"

void CCorrelationOffset::Inverse()
{
	xOffset = -xOffset;
	yOffset = -yOffset;
}

bool CCorrelationOffset::IsValid() const
{
	return m_eValidity > 0;
}

CHrtValidityCodes::EValidityCode CCorrelationOffset::GetValidity() const
{
	return m_eValidity;
}

void CCorrelationOffset::SetValidity(CHrtValidityCodes::EValidityCode eValidity)
{
	m_eValidity = eValidity;
}

bool CCorrelationOffset::IsZeroOffset() const
{
	return (fabs(xOffset) < s_nZeroOffsetThreshold && fabs(yOffset) < s_nZeroOffsetThreshold);
}

DPoint CCorrelationOffset::ToPoint()
{
	return DPoint(xOffset, yOffset);
}

bool CCorrelationOffset::operator<(const CCorrelationOffset& rhs) const
{
	if (m_EClassification < rhs.m_EClassification)
		return true;
	if (rhs.m_EClassification < m_EClassification)
		return false;
	return m_scoreNorm1 < rhs.m_scoreNorm1;
}

bool CCorrelationOffset::operator<=(const CCorrelationOffset& rhs) const
{
	return !(rhs < *this);
}

bool CCorrelationOffset::operator>(const CCorrelationOffset& rhs) const
{
	return rhs < *this;
}

bool CCorrelationOffset::operator>=(const CCorrelationOffset& rhs) const
{
	return !(*this < rhs);
}

bool CCorrelationOffset::operator==(const CCorrelationOffset& rhs) const
{
	return m_scoreNorm1 == rhs.m_scoreNorm1
		&& m_scoreBeforeNorm == rhs.m_scoreBeforeNorm
		&& xOffset == rhs.xOffset
		&& yOffset == rhs.yOffset;
}

bool CCorrelationOffset::operator!=(const CCorrelationOffset& rhs) const
{
	return !(*this == rhs);
}

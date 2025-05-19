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
#include "CorrelationOffsetClassifier.h"

#define SCORE m_scoreNorm1

CCorrelationOffsetClassifier::CCorrelationOffsetClassifier(CRegistrationScoreParameters ScoreParameters)
	:m_ScoreParameters(ScoreParameters)
{
}

CCorrelationOffsetClassifier::~CCorrelationOffsetClassifier()
{
}

CCorrelationOffset CCorrelationOffsetClassifier::CreateInvalidOffset() const
{
	CCorrelationOffset offset;
	offset.SetValidity(CHrtValidityCodes::eDeliberatlyInvalid);
	Classify(offset);
	return offset;
}

void CCorrelationOffsetClassifier::Classify(CCorrelationOffset& offset) const
{
	if (offset.SCORE > m_ScoreParameters.fCertainScore)
		offset.m_EClassification = EClassification::eCorrect;
	else if (offset.SCORE < m_ScoreParameters.fMinScore)
		offset.m_EClassification = EClassification::eInvalid;
	else
		offset.m_EClassification = EClassification::eUnsure;
}
void CCorrelationOffsetClassifier::ClassifyFlexible(CCorrelationOffset& offset) const
{
	double fMinScoreFlexible;
	if (m_ScoreParameters.bAutomaticThresholdDetection)
	{
		fMinScoreFlexible = m_ScoreParameters.GetScoreThreshold();
	}
	else
		fMinScoreFlexible = m_ScoreParameters.fMinScoreFlexible;

	if (offset.SCORE > fMinScoreFlexible)
		offset.m_EClassification = EClassification::eCorrect;
	else
		offset.m_EClassification = EClassification::eInvalid;
}
#undef SCORE
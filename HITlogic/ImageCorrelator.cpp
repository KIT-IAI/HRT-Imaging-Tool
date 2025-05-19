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

#include "ImageCorrelator.h"
#include "SubImageCorrelator.h"

CImageCorrelator::CImageCorrelator(CCorrelationParameters correlationParameters, CScoreContainer& scoreContainer)
	: m_CorrelationParameters(correlationParameters), m_ScoreContainer(scoreContainer)
{
}

CRegistrationResult CImageCorrelator::CorrelateImages(CImagePair imageParameters)
{
	CImageCorrelation imageCorrelation(m_CorrelationParameters, imageParameters, m_ScoreContainer);

	if (!imageCorrelation.CalculateBestOffsets(3))
	{
		return CRegistrationResult::InvalidThroughRoughCorrelationResult(imageParameters);
	}

	imageCorrelation.ValidateOffsets();

	auto bestCorrelationOffset = imageCorrelation.GetBestOffset(true);

	return GenerateResultObject(bestCorrelationOffset, imageParameters);
}

CRegistrationResult CImageCorrelator::GenerateResultObject(const CCorrelationOffset& bestCorrelationOffset, const CImagePair& imageParams) const
{
	CRigidRegistrationResult rigResult(bestCorrelationOffset);
	CRegistrationResult correlationResult(rigResult);

	correlationResult.RigidRegistrationResult.SetReferenceImageIndex(imageParams.GetReferenceImageIndex());
	correlationResult.RigidRegistrationResult.SetTemplateImageIndex(imageParams.GetTemplateImageIndex());
	correlationResult.RigidRegistrationResult.SetSubImageRowIndex(0);
	correlationResult.RigidRegistrationResult.SetSubImageHeight(imageParams.GetReferenceImage()->GetSize().y);
	correlationResult.RigidRegistrationResult.SetSpecialImageRowIndex(0);
	correlationResult.eClassification = bestCorrelationOffset.m_EClassification;

	if (!bestCorrelationOffset.IsValid())
	{
		correlationResult.RowOffset.push_back(CVector2d<float>(bestCorrelationOffset.xOffset, bestCorrelationOffset.yOffset));
	}
	return correlationResult;
}

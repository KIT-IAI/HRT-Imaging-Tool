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
#include "SubImageCorrelator.h"



CSubImageCorrelator::CSubImageCorrelator(CCorrelationParameters params, CScoreContainer& scoreContainer)
	:m_CorrelationParameters(params), m_ScoreContainer(scoreContainer)
{
}

CSubImageCorrelator::~CSubImageCorrelator()
{
}

void CSubImageCorrelator::CorrelateSubImages(CImagePair& imageParams, CRegistrationResult& correlationResult)
{
	assert(imageParams.GetReferenceImage()->GetSize() == imageParams.GetTemplateImage()->GetSize());

	CSubImageCorrelation SubImageCorrelation(m_CorrelationParameters, imageParams, m_ScoreContainer, correlationResult);

	SubImageCorrelation.InitCorrelation();

	auto nBestSubImageIndex = SubImageCorrelation.GetBestCorrelatedSubImage();

	SubImageCorrelation.CorrelateSubImage(nBestSubImageIndex, CCorrelationParameters::EDirection::eFirst);

	CCorrelationParameters::EDirection StartingDirection;
	if (nBestSubImageIndex < SubImageCorrelation.GetSubImageCount() / 2)
		StartingDirection = CCorrelationParameters::EDirection::eDown;
	else
		StartingDirection = CCorrelationParameters::EDirection::eUp;

	if (StartingDirection == CCorrelationParameters::EDirection::eDown)
		CorrelateFollowingSubImages(SubImageCorrelation, correlationResult);
	else
		CorrelatePreviousSubImages(SubImageCorrelation, correlationResult);

	SubImageCorrelation.AdjustTrajectory(StartingDirection);

	if (StartingDirection == CCorrelationParameters::EDirection::eDown)
		CorrelatePreviousSubImages(SubImageCorrelation, correlationResult);
	else
		CorrelateFollowingSubImages(SubImageCorrelation, correlationResult);

	if (!IsPlausible(correlationResult))
	{
		correlationResult.RigidRegistrationResult.SetValidity(CHrtValidityCodes::eInvalidSubImagesCriterion);
		InvalidateValidSubImages(correlationResult.FlexibleRegistrationResults);
	}
}

bool CSubImageCorrelator::IsPlausible(CRegistrationResult& correlation_result) const
{
	auto& flexRes = correlation_result.FlexibleRegistrationResults;

	//std::vector<CRigidRegistrationResult> validRegistrations;

	//auto bIsPlausible = [this](const auto & refRes, const auto & checkRes)
	//{
	//	return abs(refRes.GetX() - checkRes.GetX()) < m_CorrelationParameters.nSubImageHeight /2 && abs(refRes.GetY() - checkRes.GetY()) < m_CorrelationParameters.nSubImageHeight /2;
	//};

	//bool bHasChanged = true;
	//while (bHasChanged)
	//{
	//	bHasChanged = false;
	//	validRegistrations.clear();
	//	std::copy_if(flexRes.begin(), flexRes.end(), std::back_inserter(validRegistrations), [](auto& res) {return res.GetValidity() > 0; });
	//	for (auto & validReg : validRegistrations)
	//	{
	//		size_t nIndex = validReg.GetSubImageRowIndex();
	//		if (nIndex >= 1 && flexRes[nIndex - 1].GetValidity() == CHrtValidityCodes::eInvalidMinScoreAfterAllIterations)
	//		{
	//			if(bIsPlausible(validReg, flexRes[nIndex - 1]))
	//			{
	//				flexRes[nIndex - 1].SetValidity(CHrtValidityCodes::eThoughtPlausible);
	//				bHasChanged = true;
	//			}
	//		}
	//		if (nIndex + 1 < flexRes.size() && flexRes[nIndex + 1].GetValidity() == CHrtValidityCodes::eInvalidMinScoreAfterAllIterations)
	//		{
	//			if (bIsPlausible(validReg, flexRes[nIndex + 1]))
	//			{
	//				flexRes[nIndex + 1].SetValidity(CHrtValidityCodes::eThoughtPlausible);
	//				bHasChanged = true;
	//			}
	//		}
	//	}
	//}

	// Check entire validity
	auto nValidSubImages = static_cast<size_t>(std::count_if(flexRes.begin(), flexRes.end(), [](auto& res) {return res.GetValidity() > 0; }));

	if ((nValidSubImages >= flexRes.size() / 4) && (nValidSubImages > 0))
		return true;

	return  false;
}

void CSubImageCorrelator::CorrelateFollowingSubImages(CSubImageCorrelation& subImageCorrelation, CRegistrationResult& correlationResult) const
{
	for (auto nImageIndex = subImageCorrelation.GetBestCorrelatedSubImage() + 1; nImageIndex < subImageCorrelation.GetSubImageCount(); nImageIndex++)
	{
		bool bSuccess = subImageCorrelation.CorrelateSubImage(nImageIndex, CCorrelationParameters::EDirection::eDown);
		if (!bSuccess)
		{
			InvalidateAllFollowingSubImages(nImageIndex, correlationResult.FlexibleRegistrationResults);
			return;
		}
	}
}
void CSubImageCorrelator::CorrelatePreviousSubImages(CSubImageCorrelation& subImageCorrelation, CRegistrationResult& correlationResult) const
{
	for (int nImageIndex = static_cast<int>(subImageCorrelation.GetBestCorrelatedSubImage() - 1); nImageIndex >= 0; nImageIndex--)
	{
		bool bSuccess = subImageCorrelation.CorrelateSubImage(nImageIndex, CCorrelationParameters::EDirection::eUp);
		if (!bSuccess)
		{
			InvalidateAllPreviousSubImages(nImageIndex, correlationResult.FlexibleRegistrationResults);
			return;
		}
	}
}



void CSubImageCorrelator::InvalidateAllFollowingSubImages(size_t nImageIndex, std::vector<CRigidRegistrationResult>& FlexRegistrationResults) const
{
	for (auto lk = nImageIndex; lk < FlexRegistrationResults.size(); lk++)
	{
		FlexRegistrationResults[lk].SetValidity(CHrtValidityCodes::eInvalidSubImageSize);
	}
	// If the current reference window is outside the template image,
	// all windows below the current will also be
}
void CSubImageCorrelator::InvalidateAllPreviousSubImages(size_t nImageIndex, std::vector<CRigidRegistrationResult>& FlexRegistrationResults) const
{
	for (auto lk = static_cast<int>(nImageIndex); lk >= 0; lk--)
	{
		FlexRegistrationResults[lk].SetValidity(CHrtValidityCodes::eInvalidSubImageSize);
	}
	// If the current reference window is outside the template image,
	// all windows below the current will also be
}

void CSubImageCorrelator::InvalidateValidSubImages(std::vector<CRigidRegistrationResult>& FlexRegistrationResults) const
{
	for (size_t index = 0; index < FlexRegistrationResults.size(); index++)
	{
		if (FlexRegistrationResults[index].GetValidity() > 0)
			FlexRegistrationResults[index].SetValidity(CHrtValidityCodes::eInvalidSubImagesCriterion);
	}
}

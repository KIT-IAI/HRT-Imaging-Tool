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

#include "RegistrationResult.h"

CRegistrationResult::CRegistrationResult()
	:CRegistrationResult(CRigidRegistrationResult())
{
}

CRegistrationResult::CRegistrationResult(CRigidRegistrationResult rigidRegistration)
	:RigidRegistrationResult(rigidRegistration)
{
}

CRegistrationResult::~CRegistrationResult()
{
}

CRegistrationResult CRegistrationResult::InvalidThroughRoughCorrelationResult(CImagePair imageParams)
{
	CRegistrationResult correlationResult;

	correlationResult.RigidRegistrationResult.SetReferenceImageIndex(imageParams.GetReferenceImageIndex());
	correlationResult.RigidRegistrationResult.SetTemplateImageIndex(imageParams.GetTemplateImageIndex());
	correlationResult.RigidRegistrationResult.SetX(0.0);
	correlationResult.RigidRegistrationResult.SetY(0.0);
	correlationResult.RigidRegistrationResult.SetScore(0.0);
	correlationResult.RigidRegistrationResult.SetSubImageRowIndex(0);
	correlationResult.RigidRegistrationResult.SetSubImageHeight(imageParams.GetReferenceImage()->GetSize().y);
	correlationResult.RigidRegistrationResult.SetValidity(CHrtValidityCodes::eInvalidRoughCorrelationNoResult);
	correlationResult.eClassification = EClassification::eInvalid;
	return correlationResult;
}

CRegistrationResult CRegistrationResult::InvalidThroughSubImageCriterion(CRegistrationResult correlationResult)
{

	for (auto& result : correlationResult.FlexibleRegistrationResults)
	{
		result.SetValidity(CHrtValidityCodes::eInvalidSubImagesCriterion);
	}

	correlationResult.RigidRegistrationResult.SetValidity(CHrtValidityCodes::eInvalidSubImagesCriterion);
	return correlationResult;
}

bool CRegistrationResult::operator==(const CRegistrationResult& other) const
{
	return RigidRegistrationResult == other.RigidRegistrationResult
		&& FlexibleRegistrationResults == other.FlexibleRegistrationResults;
}

int CRegistrationResult::GetValidFlexibleRegistrationCount() const
{
	int nValidFlexibleRegistrations = 0;
	for (auto& result : FlexibleRegistrationResults)
	{
		if (result.GetValidity() > 0)
		{
			nValidFlexibleRegistrations++;
		}
	}
	return nValidFlexibleRegistrations;
}

bool CRegistrationResult::IsValid() const
{
	return RigidRegistrationResult.GetValidity() > 0;
}

void CRegistrationResult::CalculateResidual(DPoint referenceImageCoordinates, DPoint templateImageCoordinates)
{
	m_Residual = CResidual::CreateFromRegistration(RigidRegistrationResult, referenceImageCoordinates, templateImageCoordinates);
	RigidRegistrationResult.SetPhi((float)m_Residual.GetValue());
}

CResidual CRegistrationResult::CalculateSubImageResidual(CRigidRegistrationResult& registrationresult, std::shared_ptr<CDenseMatrix> pRigidSolution, size_t subImageHeight, size_t subPerImg)
{
	CResidual subImgResidual = CResidual::CreateFromSubimageRegistration(registrationresult, pRigidSolution, subImageHeight, subPerImg);
	return subImgResidual;
}

void CRegistrationResult::Scale(const CRegistrationResult& source, double fScalationFactor)
{
	RigidRegistrationResult.Scale(source.RigidRegistrationResult, fScalationFactor);

	vector<CRigidRegistrationResult> newFlexResults;
	vector<CVector2d<float>> newOffsetArray;

	newFlexResults.assign(source.FlexibleRegistrationResults.size(), CRigidRegistrationResult());
	for (unsigned int i = 0; i < source.FlexibleRegistrationResults.size(); i++)
	{
		newFlexResults[i].Scale(source.FlexibleRegistrationResults[i], fScalationFactor);
	}
	FlexibleRegistrationResults = newFlexResults;

	newOffsetArray.assign(source.RowOffset.size(), CVector2d<float>());
	for (unsigned int i = 0; i < source.RowOffset.size(); i++)
	{
		newOffsetArray[i] = source.RowOffset[i] * static_cast<float>(fScalationFactor);
	}
	RowOffset = newOffsetArray;
}

CResidual CRegistrationResult::GetResidual() const
{
	return m_Residual;
}

std::vector<CResidual> CRegistrationResult::GetValidSubImageResiduals() const
{
	return m_ValidSubImageResiduals;
}

size_t CRegistrationResult::GetReferenceImageIndex() const
{
	return RigidRegistrationResult.GetReferenceImageIndex();
}

size_t CRegistrationResult::GetTemplateImageIndex() const
{
	return RigidRegistrationResult.GetTemplateImageIndex();
}

void CRegistrationResult::SetReferenceImageIndex(size_t nIndex)
{
	RigidRegistrationResult.SetReferenceImageIndex(nIndex);
	for (auto& FlexRes : FlexibleRegistrationResults)
		FlexRes.SetReferenceImageIndex(nIndex);
}

void CRegistrationResult::SetTemplateImageIndex(size_t nIndex)
{
	RigidRegistrationResult.SetTemplateImageIndex(nIndex);
	for (auto& FlexRes : FlexibleRegistrationResults)
		FlexRes.SetTemplateImageIndex(nIndex);
}

void CRegistrationResult::SetSubImageResiduals(std::vector<CResidual> newSubImageResiduals)
{
	m_ValidSubImageResiduals = newSubImageResiduals;
}

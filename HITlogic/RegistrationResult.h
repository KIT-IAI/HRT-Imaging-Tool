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
#include "ImagePair.h"
#include "Point.h"
#include "Residual.h"

class CRegistrationResult
{
public:
	CRegistrationResult();
	CRegistrationResult(CRigidRegistrationResult rigidRegistration);
	~CRegistrationResult();
	static CRegistrationResult InvalidThroughRoughCorrelationResult(CImagePair imageParameters);
	static CRegistrationResult InvalidThroughSubImageCriterion(CRegistrationResult correlationResult);


	CRigidRegistrationResult RigidRegistrationResult;
	std::vector<CRigidRegistrationResult> FlexibleRegistrationResults;
	std::vector<CVector2d<float>> RowOffset;
	EClassification eClassification = EClassification::eUninitialized;

public:
	void SetReferenceImageIndex(size_t nIndex);
	void SetTemplateImageIndex(size_t nIndex);
	void SetSubImageResiduals(std::vector<CResidual> newSubImageResiduals);

	size_t GetReferenceImageIndex() const;
	size_t GetTemplateImageIndex() const;
	int GetValidFlexibleRegistrationCount() const;
	CResidual GetResidual() const;
	std::vector<CResidual> GetSubImageResiduals() const;
	bool IsValid() const;

	void CalculateResidual(DPoint image_coordinates, DPoint coordinates);
	CResidual CalculateSubImageResidual(CRigidRegistrationResult& registrationresult, std::shared_ptr<CDenseMatrix> pRigidSolution);

	void Scale(const CRegistrationResult& source, double fScalationFactor);


	bool operator==(const CRegistrationResult& other) const;

private:
	CResidual m_Residual;
	std::vector<CResidual> m_ValidSubImageResiduals;
};


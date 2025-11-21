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

#include "RegistrationResult.h"
#include "RegistrationVerificationParameters.h"

enum class EImagePairVerificator
{
	eNone = 0,
	eTreeBased = 1,
	ePositionAware = 2,
	eInvalid = 3, // Make sure this is always the last
};

class CImagePairVerificator
{

public:

	explicit CImagePairVerificator(const CRegistrationVerificationParameters& param);
	virtual ~CImagePairVerificator();

	virtual bool ValidateImagePair(size_t nReferenceIndex, size_t nTemplateIndex) = 0;
	virtual void SetSuccess(size_t nReferenceIndex, size_t nTemplateIndex, const CRegistrationResult& registration_result) = 0;
	virtual void Initialize(size_t nImages) = 0;
	virtual void Finalize() = 0;

	static CRegistrationResult CreateInvalidRegistration(size_t nImg1, size_t nImg2);

};

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
#include "Computable.h"
#include "RegistrationResult.h"
#include "RegistrationProcedureParameters.h"
#include "ImageRegistrationResult.h"
#include "ImageRegistrationData.h"
#include "StlImage.h"

class CRegStepRigidRegistration : public IComputable<CImageRegistrationData, CImageRegistrationResult>
{
public:
	explicit CRegStepRigidRegistration(CRegistrationProcedureParameters params);
	virtual ~CRegStepRigidRegistration() = default;

	void DoWork(CImageRegistrationData& registrationData, CImageRegistrationResult& regResults) override;

protected:
	void GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits) override;

private:
	std::list<CRegistrationResult> RegisterImages(std::vector<StlImage<float>*>& Images);

private:
	std::shared_ptr<CRegistrationStrategy> m_pStrategy;
	CRegistrationProcedureParameters m_RegistrationParameters;
	CProgress m_Progress;
};


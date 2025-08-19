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
#include <SLESolver.h>
#include "Computable.h"
#include "ImageRegistrationResult.h"
#include "ImageRegistrationData.h"

class CRegistrationPostProcessor : public IComputable<CImageRegistrationData, CImageRegistrationResult>
{
public:
	virtual ~CRegistrationPostProcessor() {};
	//* raus, check auf vector &

	virtual void ProcessRegistrationData(std::vector<StlImage<float>*>& images, std::vector<CRegistrationResult>& validRegistrationResults, std::vector<CRegistrationResult>& invalidRegistrationResults, CImageRegistrationResult allRegistrationResults) = 0;

	void DoWork(CImageRegistrationData& registrationData, CImageRegistrationResult& regResults) override;
	virtual void GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits) override;

protected:
	static bool AreResidualsInitialized(const std::vector<CRegistrationResult>& CorrelationResults);
	static std::vector<CRigidRegistrationResult> GetRigidRegistrationResults(const std::vector<CRegistrationResult>& RegistrationResults);

	static void CalculateResiduals(std::vector<CRegistrationResult>& RegistrationResults, std::shared_ptr<CDenseMatrix> pRigidSolution);
	static void CalculateSubImageResiduals(std::vector<CRegistrationResult>& RegistrationResults, const std::shared_ptr<CDenseMatrix> pRigidSolution);

	static void SolveRigidPositioning(const std::vector<CRegistrationResult>& RegistrationResults, std::shared_ptr<CDenseMatrix> pRigidSolution, CSLESolver::EAlgorithm eSolverAlgorithm, size_t nImageCount);
	
	static std::vector<CResidual> GetAllResiduals(const std::vector<CRegistrationResult>& RegistrationResults);
	static std::vector<CResidual> CRegistrationPostProcessor::GetSubImageResiduals(const vector<CRegistrationResult>& RegistrationResults);

	static void RemoveRegistration(const CRegistrationResult& registration, std::vector<CRegistrationResult>& validResults, std::vector<CRegistrationResult>& invalidResults);

	static void move_if(std::vector<CRegistrationResult>& valid, std::vector<CRegistrationResult>& invalid, std::function<bool(const CRegistrationResult&)> condition);
	static void move_if(std::vector<CRegistrationResult>& valid, std::vector<CRegistrationResult>& invalid, std::function<bool(const CRegistrationResult&)> condition, std::function<void(CRegistrationResult&)> invalidOperation);
};

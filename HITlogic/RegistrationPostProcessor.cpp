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

#include "RegistrationPostProcessor.h"
#include "HRTGlobalPositioning.h"
#include "Compositing.h"

/// <summary> Checks whether all residuals are initialized </summary>
/// <remarks> The code checks whether no residual is uninitialized. </remarks>
bool CRegistrationPostProcessor::AreResidualsInitialized(const vector<CRegistrationResult>& CorrelationResults)
{
	auto residualIsUninitialized = [](const CRegistrationResult& result) {return !result.GetResidual().IsInitialized(); };
	return none_of(CorrelationResults.begin(), CorrelationResults.end(), residualIsUninitialized);
}

vector<CRigidRegistrationResult> CRegistrationPostProcessor::GetRigidRegistrationResults(const vector<CRegistrationResult>& RegistrationResults)
{
	vector<CRigidRegistrationResult> out;
	out.reserve(RegistrationResults.size());

	for (const auto result : RegistrationResults)
		out.push_back(result.RigidRegistrationResult);
	return out;
}

void CRegistrationPostProcessor::CalculateResiduals(vector<CRegistrationResult>& RegistrationResults, std::shared_ptr<CDenseMatrix> pRigidSolution)
{
	for (auto& Registration : RegistrationResults)
	{
		auto nReferenceIndex = Registration.RigidRegistrationResult.GetReferenceImageIndex();
		auto nTemplateIndex = Registration.RigidRegistrationResult.GetTemplateImageIndex();
		Registration.CalculateResidual(DPoint::FromMatrixRow(nReferenceIndex, pRigidSolution), DPoint::FromMatrixRow(nTemplateIndex, pRigidSolution));
	}
}

void CRegistrationPostProcessor::CalculateSubImageResiduals(std::vector<CRegistrationResult>& RegistrationResults, const std::shared_ptr<CDenseMatrix> pSolution, double fThreshold)
{
	//Nur zum testen!
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2) << fThreshold;  // z. B. "3.50"
	std::string s = oss.str();
	std::replace(s.begin(), s.end(), '.', '_');  // → "3_50"
	std::string dateiname = "threshold_" + s + ".csv";  // → "threshold_3_50.csv"
	std::ofstream csv_res("C:\\Users\\bt3410\\Desktop\\Daten\\TestOrdner\\" + dateiname);
	csv_res << "ReferenceImageIndex" << ";" << "TemplateImageIndex" << ";" << "SubImageIndex" << ";" << "Score" << ";" << "Validity" << ";" << "X" << ";" << "Y"
			<< ";" << "X-res" << ";" << "Y-res" << ";" << "Residual-abs" << "\n";

	for (auto& Registration : RegistrationResults)
	{
		//needs to be changed
		int sub_img_reg_counter = 0;

		std::vector<CResidual> validSubImageResiduals;

		for (auto& FlexibleRegistration : Registration.FlexibleRegistrationResults)
		{
			CResidual& residual = Registration.CalculateSubImageResidual(FlexibleRegistration, pSolution);

			csv_res << residual.GetReferenceImageIndex() << ";" << residual.GetTemplateImageIndex() << ";" << residual.GetSubImageIndex() << ";" << residual.GetScore()
				<< ";" << residual.GetValidity() << ";" << residual.GetXreg() << ";" << residual.GetYreg() << ";"
				<< residual.GetX() << ";" << residual.GetY() << ";" << residual.GetValue() << "\n";

			if (residual.GetValidity() > 0) {
				validSubImageResiduals.push_back(residual);
				sub_img_reg_counter++;
			}
		}

		if (sub_img_reg_counter < 3) 
		{
			for (auto& FlexibleRegistration : Registration.FlexibleRegistrationResults)
			{
				FlexibleRegistration.SetValidity(CHrtValidityCodes::eInvalidSubImagesCriterion);
			}
			validSubImageResiduals.clear();
		}

		Registration.SetSubImageResiduals(validSubImageResiduals);
	}

	csv_res.close();
}

void CRegistrationPostProcessor::SolveRigidPositioning(const vector<CRegistrationResult>& RegistrationResults, std::shared_ptr<CDenseMatrix> pRigidSolution, CSLESolver::EAlgorithm eSolverAlgorithm, size_t nImageCount)
{
	CGlobalPositioningParameters SolverParameters;
	SolverParameters.eAlgorithm = eSolverAlgorithm;
	SolverParameters.eProcessType = CProcessType::eRigidRegistration;
	CHRTGlobalPositioning SolverObject(SolverParameters);

	pRigidSolution->Fill(0.0);

	auto RigidResults = GetRigidRegistrationResults(RegistrationResults);

	// we assume that we process an entire dataset here, i.e. the image indexes
	// in the registration results must all be in the range [0,nImageCount)
	if (!SolverObject.SolvePositioning({ 0, nImageCount - 1 }, 1, 0, 1, pRigidSolution.get(), RigidResults))
		throw L"Solution of Rigid Registration-System unsuccesful.";
}



vector<CResidual> CRegistrationPostProcessor::GetAllResiduals(const vector<CRegistrationResult>& RegistrationResults)
{
	vector<CResidual> residuals;
	residuals.reserve(RegistrationResults.size());
	for (const auto& registrationResult : RegistrationResults)
	{
		residuals.push_back(registrationResult.GetResidual());
	}
	return residuals;
}

std::vector<CResidual> CRegistrationPostProcessor::GetSubImageResiduals(const vector<CRegistrationResult>& RegistrationResults)
{
	vector<CResidual> subImageResiduals;
	for (const auto& registrationResult : RegistrationResults)
	{
		vector<CResidual> subImgResidualsForOneRegistration = registrationResult.GetSubImageResiduals();
		for (const auto& res : subImgResidualsForOneRegistration) 
		{
			subImageResiduals.push_back(res);
		}
	}
	return subImageResiduals;
}


void CRegistrationPostProcessor::move_if(vector<CRegistrationResult>& valid, vector<CRegistrationResult>& invalid, std::function<bool(const CRegistrationResult&)> condition)
{
	return move_if(valid, invalid, condition, [](CRegistrationResult&) {});
}

/*
condition specifies the condition, which if positive will result in the Registration being moved from valid to invalid
invalidOperation specifies the function, that is being called IF the result is being moved.
*/
void CRegistrationPostProcessor::move_if(vector<CRegistrationResult>& valid, vector<CRegistrationResult>& invalid, std::function<bool(const CRegistrationResult&)> condition, std::function<void(CRegistrationResult&)> invalidOperation)
{
	for (auto& reg : valid)
	{
		if (condition(reg))
		{
			invalidOperation(reg);
			invalid.push_back(reg);
		}
	}
	valid.erase(remove_if(valid.begin(), valid.end(), condition), valid.end());
}

void CRegistrationPostProcessor::RemoveRegistration(const CRegistrationResult& registration, vector<CRegistrationResult>& validResults, vector<CRegistrationResult>& invalidResults)
{
	auto iterator = find(validResults.begin(), validResults.end(), registration);

	if (iterator == validResults.end()) // could not find the Registration. Oh, oh!
	{
		ASSERT(false);
		return;
	}

	validResults.erase(iterator);
	invalidResults.push_back(registration);
}

/*
 This is a bit tricky. Because of the existing PostProcessor-Structure the Registration results have to be split into Valid/Invalid and be merged afterwards.
*/
void CRegistrationPostProcessor::DoWork(CImageRegistrationData& registrationData, CImageRegistrationResult& allRegistrationResults)
{
	vector<CRegistrationResult>& validRegistrationResults = allRegistrationResults.RegistrationResults;
	vector<CRegistrationResult> invalidRegistrationResults;

	auto isInvalid = [&](const CRegistrationResult& reg)
		{
			return !(reg.RigidRegistrationResult.IsValid());
		};
	move_if(validRegistrationResults, invalidRegistrationResults, isInvalid);

	//Important for the ThresholdAdapter for SubImage registration
	std::vector<std::list<size_t>>& imagegroups = allRegistrationResults.ForceSingleImageGroup();

	ProcessRegistrationData(registrationData.Images, validRegistrationResults, invalidRegistrationResults, imagegroups);

	move_if(invalidRegistrationResults, validRegistrationResults, [](const CRegistrationResult&) {return true; });
}

void CRegistrationPostProcessor::GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits)
{
	nCompletedWorkUnits = 0;
	nTotalWorkUnits = 1;
}

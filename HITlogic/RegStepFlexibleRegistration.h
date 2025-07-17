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
#include "RegistrationPostProcessor.h"
#include "RegistrationResult.h"
#include "RegistrationProcedureParameters.h"
#include "TimeMeasurement.h"
#include "StlImage.h"

class CRegStepFlexibleRegistration : public CRegistrationPostProcessor
{
public:
	CRegStepFlexibleRegistration(CRegistrationProcedureParameters param);
	~CRegStepFlexibleRegistration();

	void ProcessRegistrationData(vector<StlImage<float>*>& images, vector<CRegistrationResult>& validRegistrationResults, vector<CRegistrationResult>&);

	void ProcessRegistrationData(std::vector<StlImage<float>*>& images, std::vector<CRegistrationResult>& validRegistrationResults, std::vector<CRegistrationResult>& invalidRegistrationResults, vector<std::list<size_t>>& imagegroups) override;


private:
	CImagePair GenerateImageParameters(CRegistrationResult registrationResult, std::vector<StlImage<float>*>& Images, std::vector<StlImage<float>>& ImagesSmooth) const;
	void RegisterImages(std::vector<StlImage<float>*>& Images, std::vector<CRegistrationResult>& m_ValidRegistrationResults);

	virtual void GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits) override;
private:
	CRegistrationProcedureParameters m_RegistrationParameters;

	std::mutex calculationMutex;
	size_t m_nTotalPairs = 0;
	size_t m_nPairsDone = 0;
};


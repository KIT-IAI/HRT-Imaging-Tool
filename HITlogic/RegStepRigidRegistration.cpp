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
#include "RegStepRigidRegistration.h"

#include "ImageCorrelation.h"
#include "ImageCorrelator.h"
#include "PairVerificatorFactory.h"
#include "RegistrationStrategyFactory.h"

using namespace std;



CRegStepRigidRegistration::CRegStepRigidRegistration(CRegistrationProcedureParameters params)
	:m_RegistrationParameters(params)
{
	m_pStrategy = CRegistrationStrategyFactory::CreateStrategy(m_RegistrationParameters.eRegistrationStrategy, m_RegistrationParameters.StrategyParameters);
	auto RegistrationVerifier = CPairVerificatorFactory::CreatePairVerificator(m_RegistrationParameters.ePairVerificator, m_RegistrationParameters.VerificationParameters);
	m_pStrategy->AddRegistrationVerificator(RegistrationVerifier);
}


void CRegStepRigidRegistration::DoWork(CImageRegistrationData& registrationData, CImageRegistrationResult& finalResult)
{
	m_pStrategy->Initialize(registrationData.Images.size());
	m_pStrategy->InitializeAdditionalParameters(registrationData);

	auto registrationResults = RegisterImages(registrationData.Images);

	finalResult.RegistrationResults.insert(finalResult.RegistrationResults.end(), registrationResults.begin(), registrationResults.end());
}
list<CRegistrationResult> CRegStepRigidRegistration::RegisterImages(vector<StlImage<float>*>& Images)
{
	std::list<CRegistrationResult> listRegistrationResults;
	CScoreContainer scoreContainer(m_RegistrationParameters.ScoreParameters);

	CImageCorrelator image_correlator(m_RegistrationParameters.CorrelationParameters, scoreContainer);

	std::mutex resultListMutex;

	auto thread_count = max(1u, std::thread::hardware_concurrency());
	std::vector<std::thread> threads;
	threads.reserve(thread_count);

	auto start = std::chrono::high_resolution_clock::now();
	for (auto i = 0u; i < thread_count; i++)
	{
		threads.push_back(std::thread([i, thread_count, &Images, &image_correlator, &resultListMutex, &listRegistrationResults, this]() {
			while (!m_bIsCanceled && !m_pStrategy->IsFinished())
			{
				size_t nReferenzImageIndex = 0;
				size_t nTemplateImageIndex = 0;

				if (!m_pStrategy->GetNextPair(nReferenzImageIndex, nTemplateImageIndex))
				{
					std::this_thread::sleep_for(0ms);
					continue;
				}

				CRegistrationResult registrationResult;

				// This code block is not really nice. It should be inside CRegistrationStrategy Somehow
				if (m_pStrategy->VerifyImagePair(nReferenzImageIndex, nTemplateImageIndex))
				{
					CImagePair imageParameters(Images[nReferenzImageIndex], Images[nTemplateImageIndex], nReferenzImageIndex, nTemplateImageIndex);
					registrationResult = image_correlator.CorrelateImages(imageParameters);
				}
				else
				{
					registrationResult = CImagePairVerificator::CreateInvalidRegistration(nReferenzImageIndex, nTemplateImageIndex);
				}


				m_pStrategy->SetSuccess(nReferenzImageIndex, nTemplateImageIndex, registrationResult);

				resultListMutex.lock();
				listRegistrationResults.push_back(registrationResult);
				resultListMutex.unlock();

				size_t nPairsDone = 0;
				size_t nPairsTotal = 0;

				m_pStrategy->EstimateProgress(nPairsDone, nPairsTotal);

				if (nPairsDone % 100 == 0)
				{
					ReportProgress();
				}
			}
			}));
	}
	for (auto i = 0u; i < thread_count; i++)
	{
		threads[i].join();
	}

	m_pStrategy->Finalize();

	auto finish = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
	std::wostringstream oss;
	oss.setf(std::ios::fixed);
	oss.precision(3);
	oss << L"Rigid image registration finished (measured time: " << elapsed.count() / 1000.0 << L" seconds)";
	CLog::Log(CLog::eNotice, L"CRegStepRigidRegistration", oss.str().c_str());

	ReportProgress();

	return listRegistrationResults;
}

void CRegStepRigidRegistration::GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits)
{
	m_pStrategy->EstimateProgress(nCompletedWorkUnits, nTotalWorkUnits);
}

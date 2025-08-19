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

#include "RegStepFlexibleRegistration.h"
#include "ScoreContainer.h"
#include "SubImageCorrelator.h"
#include <ppl.h>
#include <boost/thread/barrier.hpp>

CRegStepFlexibleRegistration::CRegStepFlexibleRegistration(CRegistrationProcedureParameters params)
	:m_RegistrationParameters(params)
{
}


CRegStepFlexibleRegistration::~CRegStepFlexibleRegistration()
{
}
void CRegStepFlexibleRegistration::ProcessRegistrationData(vector<StlImage<float>*>& images, vector<CRegistrationResult>& validRegistrationResults, vector<CRegistrationResult>& /*invalidRegistrationResults*/, CImageRegistrationResult allRegistrationResults)
{
	// StlImage<float>* --> StlImage<float> ???
	RegisterImages(images, validRegistrationResults);

	ReportProgress();

}

void CRegStepFlexibleRegistration::RegisterImages(vector<StlImage<float>*>& Images, std::vector<CRegistrationResult>& ValidRegistrationResults)
{
	auto registrationIterator = ValidRegistrationResults.begin();

	CScoreContainer scoreContainer(m_RegistrationParameters.ScoreParameters);
	CSubImageCorrelator imageCorrelator(m_RegistrationParameters.CorrelationParameters, scoreContainer);

	m_nTotalPairs = ValidRegistrationResults.size();

	auto thread_count = std::max(1u, std::thread::hardware_concurrency());

	std::vector<std::thread> threads;
	threads.reserve(thread_count);

	static const auto RegPerMutexLock = 10;

	std::vector<StlImage<float>> ImagesSmooth;
	ImagesSmooth.resize(Images.size());

	boost::barrier barrier(thread_count);

	for (auto i = 0u; i < thread_count; i++)
	{
		threads.push_back(std::thread([i, thread_count, this, &registrationIterator, &ValidRegistrationResults, &Images, &ImagesSmooth, &imageCorrelator, &barrier]() {

			for (int j = i;j < Images.size();j += thread_count) ImagesSmooth[j].Smooth(*Images[j]);
			barrier.wait();

			while (!m_bIsCanceled)
			{
				calculationMutex.lock();

				// Man this is an ugly check. I know. I am very aware of that.
				// Please do me a favor and find a way that I do not have to unlock the mutex manually here.
				if (registrationIterator == ValidRegistrationResults.end())
				{
					calculationMutex.unlock();
					break;
				}
				auto it = registrationIterator;
				auto max_move = std::min((ptrdiff_t)RegPerMutexLock, std::distance(it, ValidRegistrationResults.end()));
				registrationIterator += max_move;
				m_nPairsDone += max_move;
				calculationMutex.unlock();

				for (int j = 0; j < max_move;j++)
				{
					CRegistrationResult& currentRegistration = *it;
					++it;

					ASSERT(currentRegistration.IsValid());

					auto imageParameters = GenerateImageParameters(currentRegistration, Images, ImagesSmooth);

					imageCorrelator.CorrelateSubImages(imageParameters, currentRegistration);
				}
				if (i == 0)
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

}
CImagePair CRegStepFlexibleRegistration::GenerateImageParameters(CRegistrationResult registrationResult, vector<StlImage<float>*>& Images, vector<StlImage<float>>& ImagesSmooth) const
{
	auto refIndex = registrationResult.RigidRegistrationResult.GetReferenceImageIndex();
	auto tempIndex = registrationResult.RigidRegistrationResult.GetTemplateImageIndex();
	return CImagePair(Images[refIndex], Images[tempIndex], &ImagesSmooth[refIndex], &ImagesSmooth[tempIndex], refIndex, tempIndex);
}
void CRegStepFlexibleRegistration::GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits)
{
	std::lock_guard<std::mutex> lock(calculationMutex);
	nCompletedWorkUnits = m_nPairsDone;
	nTotalWorkUnits = m_nTotalPairs;
}
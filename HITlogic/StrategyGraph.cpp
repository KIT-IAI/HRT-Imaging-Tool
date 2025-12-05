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
#include "StrategyGraph.h"



CStrategyGraph::CStrategyGraph()
	:
	m_LockList(0),
	m_nCurrentTemplateImage(0)
{

}
CStrategyGraph::~CStrategyGraph()
{
}

size_t CStrategyGraph::InitialWorkloadEstimation() const
{
	// This number is veeeeeeeeeeery arbitrary.
	return 30 * m_nImageCount;
}
bool CStrategyGraph::EstimateProgress(size_t& nPairsDone, size_t& nPairsFinal)
{
	nPairsDone = m_nPairsDone;

	double fProgressThroughEarlierIterations = 0;
	double fProgressOfCurrentIteration = 0;

	switch (m_nCurrentIteration)
	{
	case 0: fProgressThroughEarlierIterations = 0; break;
	case 1: fProgressThroughEarlierIterations = 0.75; break;
	case 2: fProgressThroughEarlierIterations = 0.9; break;
	}
	switch (m_nCurrentIteration)
	{
	case 0: fProgressOfCurrentIteration = 0.75; break;	// Iteration 0 makes up 75% of runtime
	case 1: fProgressOfCurrentIteration = 0.15; break;	// Iteration 1 makes up 15% of runtime
	case 2: fProgressOfCurrentIteration = 0.10; break;	// Iteration 2 makes up 10% of runtime
	}

	auto fPercentageOfCurrentIterationDone = m_nCurrentTemplateImage / static_cast<double>(m_nImageCount);

	auto fTotalProgressPercentage = fProgressThroughEarlierIterations + fProgressOfCurrentIteration * fPercentageOfCurrentIterationDone;

	// No idea if this can happen though.
	if (fTotalProgressPercentage == 0)
		fTotalProgressPercentage = 1;

	nPairsFinal = static_cast<size_t>(nPairsDone / fTotalProgressPercentage);
	return true;
}

void CStrategyGraph::Initialize(size_t nImages)
{
	::CRegistrationStrategy::Initialize(nImages);
	m_nImageCount = nImages;

	m_LockList.resize(m_nImageCount);
	for (size_t i = 0; i < m_nImageCount; i++)
		m_LockList[i].SetSize(m_nImageCount);

	m_pSuccessfulRegistrationGraph = std::make_unique<CSynchronizedGraph>(m_nImageCount);
	m_pEffectedRegistrationGraph = std::make_unique<CSynchronizedGraph>(m_nImageCount);
}


void CStrategyGraph::SetSuccess(size_t nImg1, size_t nImg2, const CRegistrationResult& correlationResult)
{
	auto bIsSafeRegistration = correlationResult.eClassification >= EClassification::eCorrect;
	auto bExcludedByVerifier = correlationResult.RigidRegistrationResult.GetValidity() == CHrtValidityCodes::eInvalidImagePairVerification;


	UpdateGraphs(nImg1, nImg2, bIsSafeRegistration, bExcludedByVerifier);
	UpdateLockList(nImg1, nImg2, bIsSafeRegistration, bExcludedByVerifier);

	::CRegistrationStrategy::SetSuccess(nImg1, nImg2, correlationResult);
}

void CStrategyGraph::UpdateGraphs(size_t nImg1, size_t nImg2, bool bIsSafeRegistration, bool bExcludedByVerifier) const
{
	if (!bExcludedByVerifier)
		m_pEffectedRegistrationGraph->AddBiDirectionalEdge(nImg1, nImg2);

	if (bIsSafeRegistration)
		m_pSuccessfulRegistrationGraph->AddBiDirectionalEdge(nImg1, nImg2);
}

void CStrategyGraph::UpdateLockList(size_t nImg1, size_t nImg2, bool bIsSafeRegistration, bool bExcludeForOneIterationOnly /*false*/)
{
	if (bIsSafeRegistration)
	{
		ExcludeImagePair({ nImg1 ,nImg2 }, CLockList::IMAGE_EXCLUDED_BY_SUCCESSFUL_NEIGHBOR);
		m_LockList[nImg2].ExcludeImages({ nImg1 }, CLockList::SUCCESSFUL_REGISTRATION);
	}
	else
	{
		if (bExcludeForOneIterationOnly)
			m_LockList[nImg2].ExcludeImages({ nImg1 }, CLockList::EXCLUDED_FOR_THIS_ITERATION);
		else
			m_LockList[nImg2].ExcludeImages({ nImg1 }, CLockList::UNSUCCESSFUL_REGISTRATION);

		UnlockImagePair({ nImg1 ,nImg2 });
	}
}

void CStrategyGraph::UnlockImagePair(std::pair<size_t, size_t> pair)
{
	auto toUnlock = m_pSuccessfulRegistrationGraph->BreathFirstSearch(pair.first, m_Parameters.nMaxRegistrationDistance);
	m_LockList[pair.second].UnlockImages(toUnlock);
}
void CStrategyGraph::ExcludeImagePair(std::pair<size_t, size_t> pair, CLockList::EImageExclusion eReason)
{
	auto toExclude = m_pSuccessfulRegistrationGraph->BreathFirstSearch(pair.first, m_Parameters.nMaxRegistrationDistance);
	m_LockList[pair.second].ExcludeImages(toExclude, eReason);
}

void CStrategyGraph::SetParameters(const CRegistrationStrategyParameters& parameters)
{
	this->m_Parameters = parameters;
}


bool CStrategyGraph::PrepareNextPair()
{
	auto imagePair = GetNextImagePair();

	if (m_bFinished)
		return false;

	LockImagePair(imagePair);

	m_aNextPair[0] = imagePair.first;
	m_aNextPair[1] = imagePair.second;

	return true;
}
std::pair<size_t, size_t> CStrategyGraph::GetNextImagePair()
{
	std::pair<size_t, size_t> imagePair = { 0,0 };

	imagePair.second = m_nCurrentTemplateImage;
	imagePair.first = FindReferenceImageForTemplate();

	while (!m_bFinished && !IsValidIndexPair(imagePair))
	{
		AdvanceToNextTemplateImage();
		imagePair.second = m_nCurrentTemplateImage;
		imagePair.first = FindReferenceImageForTemplate();
	}

	assert(!RegistrationHasBeenEffected(imagePair));

	return imagePair;
}

size_t CStrategyGraph::FindReferenceImageForTemplate() const
{
	if (m_nCurrentTemplateImage == 0)
		return 0;

	auto& vCurrentLockList = m_LockList[m_nCurrentTemplateImage];
	auto nMinToCheck = GetMaxDistance(m_nCurrentTemplateImage);


	for (auto nReferenceImage = m_nCurrentTemplateImage - 1; nReferenceImage > nMinToCheck; nReferenceImage--)
	{
		if (vCurrentLockList[nReferenceImage] == CLockList::IMAGE_FREE)
			return nReferenceImage;
	}
	if (m_LockList[m_nCurrentTemplateImage][0] == CLockList::IMAGE_FREE)
		return 0;
	return m_nCurrentTemplateImage; // This is an invalid value
}


void CStrategyGraph::AdvanceToNextTemplateImage()
{
	m_nCurrentTemplateImage++;

	if (m_nCurrentTemplateImage == m_nImageCount)
	{
		AdvanceToNextIteration();
	}

	ClearLockList();
}
void CStrategyGraph::AdvanceToNextIteration()
{
	WaitForAllThreadsToEnd();

	m_nCurrentIteration++;
	m_nCurrentTemplateImage = 0;

	if (m_nCurrentIteration == m_nMaxIterations)
		m_bFinished = true;
}

void CStrategyGraph::WaitForAllThreadsToEnd() const
{
	while (m_nPairsDealt != m_nPairsDone)
		std::this_thread::yield();
}

void CStrategyGraph::ClearLockList()
{
	auto nMinToCheck = GetMaxDistance(m_nCurrentTemplateImage);


	m_LockList[m_nCurrentTemplateImage].SetLockStatus(0, nMinToCheck, CLockList::EXCEEDS_MAXIMUM_IMAGE_DISTANCE);

	m_LockList[m_nCurrentTemplateImage].FreeLocks(nMinToCheck, m_nCurrentTemplateImage); // In case for some stupid reason there still are locks
	m_LockList[m_nCurrentTemplateImage].WhereXSetY(nMinToCheck, m_nCurrentTemplateImage, CLockList::EXCLUDED_FOR_THIS_ITERATION, CLockList::IMAGE_FREE);
}
size_t CStrategyGraph::GetMaxDistance(size_t fromIndex) const
{
	if (m_Parameters.nMaxDistance > fromIndex)
		return 0;
	return fromIndex - m_Parameters.nMaxDistance;
}

bool CStrategyGraph::IsValidIndexPair(std::pair<size_t, size_t> imagePair) const
{
	return imagePair.first < imagePair.second && imagePair.second < m_nImageCount;
}

bool CStrategyGraph::IsValidImagePair(std::pair<size_t, size_t> imagePair) const
{
	if (!IsValidIndexPair(imagePair))
		return false;
	if (RegistrationHasBeenEffected(imagePair))
		return false;
	return true;
}

bool CStrategyGraph::RegistrationHasBeenEffected(std::pair<size_t, size_t> imagePair) const
{
	return m_pEffectedRegistrationGraph->IsEdge(imagePair.first, imagePair.second);
}


void CStrategyGraph::LockImagePair(std::pair<size_t, size_t> pair)
{
	auto toLock = m_pSuccessfulRegistrationGraph->BreathFirstSearch(pair.first, m_Parameters.nMaxRegistrationDistance);
	m_LockList[pair.second].LockImages(toLock);
}

void CStrategyGraph::Finalize()
{
	CRegistrationStrategy::Finalize();
}
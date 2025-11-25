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
#include "PositionAwarePairVerificator.h"

#include "ProcessType.h"



CPositionAwarePairVerificator::CPositionAwarePairVerificator(const CRegistrationVerificationParameters& Parameters)
	:CImagePairVerificator(Parameters),
	m_pIterativeSolver(nullptr),
	m_Parameters(Parameters),
	m_RegistrationGraph(0)
{
}

CPositionAwarePairVerificator::~CPositionAwarePairVerificator()
{
}

bool CPositionAwarePairVerificator::ValidateImagePair(size_t nReferenceIndex, size_t nTemplateIndex)
{
	if (!AreInSameImageGroup(nReferenceIndex, nTemplateIndex))
		return true;

	return DoSufficientlyOverlap({ nReferenceIndex, nTemplateIndex });
}

bool CPositionAwarePairVerificator::AreInSameImageGroup(size_t nReferenceIndex, size_t nTemplateIndex) const
{
	return m_RegistrationGraph.IsConnected(nReferenceIndex, nTemplateIndex);
}

void CPositionAwarePairVerificator::SetSuccess(size_t nReferenceIndex, size_t nTemplateIndex, const CRegistrationResult& regResult)
{
	if (regResult.eClassification < EClassification::eCorrect)
		return;

	m_RegistrationGraph.AddBiDirectionalEdge(nReferenceIndex, nTemplateIndex);
	m_pIterativeSolver->AddRegistrations({ regResult.RigidRegistrationResult });
}

void CPositionAwarePairVerificator::Initialize(size_t nImageCount)
{
	m_RegistrationGraph = CGraph(nImageCount);

	CGlobalPositioningParameters PositioningParameters(m_Parameters.eSolverAlgorithm, CProcessType::eRigidRegistration);
	m_pIterativeSolver = std::make_unique<CHrtIterativeSLEWorker>(PositioningParameters, nImageCount, 1, 0, true);

	m_pIterativeSolver->Start();
}

void CPositionAwarePairVerificator::Finalize()
{
	m_pIterativeSolver->Stop();
	m_pIterativeSolver = nullptr;
}

bool CPositionAwarePairVerificator::DoSufficientlyOverlap(std::pair<size_t, size_t> imagePair) const
{
	const double fTolerance = 1.1;

	StlImageSize imageSize = m_Parameters.ImageSize;
	StlImageSize increasedSize(static_cast<long long>(imageSize.x * fTolerance), static_cast<long long>(imageSize.y * fTolerance));

	auto RefPos = m_pIterativeSolver->GetPositionFor(imagePair.first);
	auto TempPos = m_pIterativeSolver->GetPositionFor(imagePair.second);

	StlImagePoint referencePos(static_cast<long long>(RefPos.x), static_cast<long long>(RefPos.y));
	StlImagePoint templatePos(static_cast<long long>(TempPos.x), static_cast<long long>(TempPos.y));

	StlImageRect referenceRect(referencePos, increasedSize);
	StlImageRect templateRect(templatePos, increasedSize);

	// this call does not return the intersecting area, but a flag specifying
	// whether such an intersecting area exists or not
	return referenceRect.Intersection(referenceRect, templateRect);
}

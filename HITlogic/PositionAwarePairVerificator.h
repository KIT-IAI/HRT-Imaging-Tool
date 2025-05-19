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
#include "ImagePairVerificator.h"
#include "HrtIterativeSLEWorker.h"
#include "Graph.h"

class CPositionAwarePairVerificator :
	public CImagePairVerificator
{
public:
	explicit CPositionAwarePairVerificator(const CRegistrationVerificationParameters& Parameters);
	virtual ~CPositionAwarePairVerificator();

	virtual bool ValidateImagePair(size_t nReferenceIndex, size_t nTemplateIndex) override;
	virtual void SetSuccess(size_t nReferenceIndex, size_t nTemplateIndex, const CRegistrationResult& registration_result) override;
	virtual void Initialize(size_t nImages) override;
	virtual void Finalize() override;

private:
	bool AreInSameImageGroup(size_t nReferenceIndex, size_t nTemplateIndex) const;
	bool DoSufficientlyOverlap(std::pair<size_t, size_t> imagePair) const;

	CRegistrationVerificationParameters m_Parameters;

	std::vector<CRigidRegistrationResult> m_RegistrationQueue;
	std::unique_ptr<CHrtIterativeSLEWorker> m_pIterativeSolver;
	std::shared_ptr<CDenseMatrix> m_pSolutionSnapshot;
	CGraph m_RegistrationGraph;
};


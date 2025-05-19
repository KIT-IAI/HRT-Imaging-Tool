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


#include "RegistrationStrategy.h"
#include "LockList.h"
#include <SynchronizedGraph.h>


class CStrategyGraph :
	public CRegistrationStrategy
{

public:
	CStrategyGraph();
	~CStrategyGraph();

	virtual void SetParameters(const CRegistrationStrategyParameters&) override;


protected:
	virtual size_t InitialWorkloadEstimation() const override;
	virtual bool EstimateProgress(size_t& nPairsDone, size_t& nPairsFinal) override;

	virtual void Initialize(size_t nImages) override;
	virtual bool PrepareNextPair() override;
	virtual void SetSuccess(size_t nImg1, size_t nImg2, const CRegistrationResult& correlationResult) override;
	virtual void Finalize() override;


private:

	void UpdateGraphs(size_t nImg1, size_t nImg2, bool bIsSafeRegistration, bool bExcludedByVerifier) const;
	void UpdateLockList(size_t nImg1, size_t nImg2, bool bIsSafeRegistration, bool bExcludeForOneIterationOnly);
	void UnlockImagePair(std::pair<size_t, size_t> pair);
	void ExcludeImagePair(std::pair<size_t, size_t> pair, CLockList::EImageExclusion);

	std::pair<size_t, size_t> GetNextImagePair();
	size_t FindReferenceImageForTemplate() const;
	void AdvanceToNextTemplateImage();
	void AdvanceToNextIteration();
	void WaitForAllThreadsToEnd() const;
	void ClearLockList();
	size_t GetMaxDistance(size_t fromIndex) const;
	bool IsValidIndexPair(std::pair<size_t, size_t>) const;
	bool IsValidImagePair(std::pair<size_t, size_t>) const;
	bool RegistrationHasBeenEffected(std::pair<size_t, size_t> imagePair) const;
	void LockImagePair(std::pair<size_t, size_t> pair);

	std::vector<CLockList> m_LockList;

	size_t m_nCurrentTemplateImage;
	std::unique_ptr<CSynchronizedGraph> m_pEffectedRegistrationGraph;
	std::unique_ptr<CSynchronizedGraph> m_pSuccessfulRegistrationGraph;
	CRegistrationStrategyParameters  m_Parameters;
	size_t m_nMaxIterations = 3;
	size_t m_nCurrentIteration = 0;
};


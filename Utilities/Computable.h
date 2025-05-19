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
#include "Progressing.h"
#include "Cancelable.h"
#include <vector>
#include "TimeMeasurement.h"

template<typename TParam, typename TResult = TParam>
class IComputable : public IProgressing, public ICancelable, public IProgressListener
{
	typedef std::shared_ptr<IComputable<TParam, TResult>> ComputationStep;


public:

	virtual void AddPreceedingComputationStep(ComputationStep step) final
	{
		step->AddProgressListener(this);
		m_PreceedingComputationSteps.push_back(step);

		m_CurrentProgress.SubStepProgress.push_back(CProgress());
		m_CurrentProgress.nStepCount++;
	}
	virtual void AddSubsequentComputationStep(ComputationStep step) final
	{
		step->AddProgressListener(this);
		m_SubsequentComputationSteps.push_back(step);

		m_CurrentProgress.SubStepProgress.push_back(CProgress());
		m_CurrentProgress.nStepCount++;
	}

	virtual void Compute(TParam& param, TResult& result) final
	{
		m_Timer.Start();
		m_CurrentProgress.nCurrentStep++;
		ReportProgress();


		for (ComputationStep step : m_PreceedingComputationSteps)
		{
			step->Compute(param, result);
			m_nCurrentSubStep++;
		}

		if (m_bIsCanceled)
			return;

		DoWork(param, result);

		for (ComputationStep step : m_SubsequentComputationSteps)
		{
			step->Compute(param, result);
			m_nCurrentSubStep++;
		}

		ReportProgress();
		m_Timer.Stop();
	}
	void OnCancel() override
	{
		for (ComputationStep step : m_PreceedingComputationSteps)
		{
			step->Cancel();
		}
		for (ComputationStep step : m_SubsequentComputationSteps)
		{
			step->Cancel();
		}
	}
	void OnUncancel() override
	{
		for (ComputationStep step : m_PreceedingComputationSteps)
		{
			step->Uncancel();
		}
		for (ComputationStep step : m_SubsequentComputationSteps)
		{
			step->Uncancel();
		}
	}

	virtual CProgress GetProgress() override final
	{
		m_Timer.TakeTime();

		m_CurrentProgress.nCurrentStep = m_nCurrentSubStep;
		m_CurrentProgress.nSecondsPassed = m_Timer.GetSeconds();
		GetWorkUnits(m_CurrentProgress.nFinishedWorkUnits, m_CurrentProgress.nApproximatedWorkUnits);

		return m_CurrentProgress;
	}
	virtual void OnProgress(CProgress progress) override
	{
		m_CurrentProgress.SubStepProgress[m_nCurrentSubStep] = progress;
		ReportProgress();
	}

protected:
	virtual void DoWork(TParam& param, TResult& result) = 0;
	virtual void GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits) = 0;

private:
	std::vector< std::shared_ptr< IComputable<TParam, TResult> > > m_PreceedingComputationSteps;
	std::vector< std::shared_ptr< IComputable<TParam, TResult> > > m_SubsequentComputationSteps;
	CProgress m_CurrentProgress;
	CTimeMeasurement m_Timer;

	size_t m_nCurrentSubStep = 0;
};


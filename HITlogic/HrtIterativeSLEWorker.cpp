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
#include "HrtIterativeSLEWorker.h"

CHrtIterativeSLEWorker::CHrtIterativeSLEWorker(CGlobalPositioningParameters positioningParameters, size_t nMaximumImageCount, size_t nSubImageCount, size_t nSubImageIndexOffset, bool bGenerateRegularisation)
	: m_PositioningParameters(positioningParameters),
	m_nSubImageCount(nSubImageCount),
	m_nSubImageIndexOffset(nSubImageIndexOffset),
	m_nMaximumImageCount(nMaximumImageCount),
	m_eCurrentState(ECalculationState::eREADY),
	m_eDesiredState(ECalculationState::eREADY)
{
	InitializeMatrices(bGenerateRegularisation);

	m_cgParameters = CSLESolver::CreateIterationParameters(m_PositioningParameters.eAlgorithm);
	//m_cgParameters.m_nMaxIterations = m_nIterationsBetweenChecks;
	m_cgParameters.m_bQuiet = true;
}

CHrtIterativeSLEWorker::~CHrtIterativeSLEWorker()
{
	Stop();
}

void CHrtIterativeSLEWorker::InitializeMatrices(bool bGenerateRegularisation)
{
	m_WtW.AllocMatrix(m_nMaximumImageCount * m_nSubImageCount, m_nMaximumImageCount * m_nSubImageCount);
	m_Wtd.AllocMatrix(m_WtW.Rows(), 2);
	m_Solution.AllocMatrix(m_WtW.Rows(), 2);

	//m_FullWtW.Fill(0.0); // A sparse Matrix does not need to be initialized like this
	m_Wtd.Fill(0.0);
	m_Solution.Fill(0.0);

	m_pCurrentSolutionSnapshot = std::make_shared<CDenseMatrix>();
	SnapShotSolution();

	if (bGenerateRegularisation)
	{
		CHRTGlobalPositioning::GenerateRegularisation(m_PositioningParameters.eProcessType, m_nMaximumImageCount, m_nSubImageCount, 1, m_WtW);
		CHRTGlobalPositioning::AssignAnchorImage(m_PositioningParameters.eAlgorithm, m_WtW);
		m_WtW.Flush();
	}
}

void CHrtIterativeSLEWorker::Start()
{
	if (m_eCurrentState == ECalculationState::ePAUSED)
	{
		m_eDesiredState = ECalculationState::eRUNNING;
		return;
	}
	m_eDesiredState = m_eCurrentState = ECalculationState::eRUNNING;

	if (m_Thread == nullptr)
		m_Thread = std::make_unique<std::thread>(&CHrtIterativeSLEWorker::Run, this);
}

void CHrtIterativeSLEWorker::Stop()
{
	m_eDesiredState = ECalculationState::eSTOPPED;
	Join();
	SnapShotSolution();
}

void CHrtIterativeSLEWorker::Join() const
{
	assert(m_Thread != nullptr);

	if (m_Thread->joinable())
		m_Thread->join();

	while (m_eCurrentState != ECalculationState::eSTOPPED)
		std::this_thread::yield();
}

void CHrtIterativeSLEWorker::Pause()
{
	assert(m_eCurrentState == ECalculationState::eRUNNING);
	m_eDesiredState = ECalculationState::ePAUSED;
	while (m_eCurrentState != m_eDesiredState)
		std::this_thread::yield();
	SnapShotSolution();
}

void CHrtIterativeSLEWorker::AddRegistrations(const vector<CRigidRegistrationResult>& rigidRegistrationResults)
{
	std::lock_guard<std::mutex> lock(m_QueueMutex);
	m_RegistrationQueue.insert(m_RegistrationQueue.end(), rigidRegistrationResults.begin(), rigidRegistrationResults.end());
}





void CHrtIterativeSLEWorker::Run()
{
	while (true)
	{
		switch (m_eDesiredState)
		{
		case ECalculationState::eSTOPPED:
		{
			m_eCurrentState = ECalculationState::eSTOPPED;
			return;
		}
		case ECalculationState::ePAUSED:
		{
			m_eCurrentState = ECalculationState::ePAUSED;
			while (m_eDesiredState == ECalculationState::ePAUSED)
				std::this_thread::yield();
			continue;
		}
		case ECalculationState::eRUNNING:
		{
			m_eCurrentState = ECalculationState::eRUNNING;
			SnapShotSolution();
			ClearSolution();
			ProcessWorkQueue();

			if (m_nCalculationRange == 0)
				continue;
			SolveSLE();
			continue;
		}
		default:
			assert(false);
			return;
		}
	}
}
void CHrtIterativeSLEWorker::SolveSLE()
{
	switch (m_PositioningParameters.eAlgorithm)
	{
	case CSLESolver::EAlgorithm::eJacobi:
	case CSLESolver::EAlgorithm::eRigidStartIteration:
		CSLESolver::SolveEquationJacobi(m_WtW, m_Wtd, m_Solution, m_cgParameters);
		break;

	case CSLESolver::EAlgorithm::eCGALGLIB:
	case CSLESolver::EAlgorithm::eCGALGLIBStartIteration:
		CSLESolver::SolveEquationCGALGLIB(m_WtW, m_Wtd, m_Solution, m_cgParameters);
		break;

	case CSLESolver::EAlgorithm::eCG:
	case CSLESolver::EAlgorithm::eCGStartIteration:
		CSLESolver::SolveEquationCG(m_WtW, m_Wtd, m_Solution, m_cgParameters, true);
		break;
	default:
		assert(false);
		break;
	}
}


void CHrtIterativeSLEWorker::ProcessWorkQueue()
{
	std::lock_guard<std::mutex> lock(m_QueueMutex);

	// we assume that we process an entire dataset here, i.e. the image indexes
	// in the registration results must all be in the range [0,nImageCount)
	CHRTGlobalPositioning::AddSystemData({ 0, m_nMaximumImageCount - 1 }, m_RegistrationQueue, m_nSubImageCount, m_nSubImageIndexOffset, 1, m_WtW, m_Wtd, m_PositioningParameters.eProcessType);
	m_WtW.Flush();
	m_RegistrationQueue.clear();
}
CVector2d<double> CHrtIterativeSLEWorker::GetPositionFor(size_t nImage)
{
	m_SnapshotMutex.LockRead();
	auto res = CVector2d<double>((*m_pCurrentSolutionSnapshot)[nImage][0], (*m_pCurrentSolutionSnapshot)[nImage][1]);
	m_SnapshotMutex.UnlockRead();
	return res;
}


void CHrtIterativeSLEWorker::SnapShotSolution()
{
	m_SnapshotMutex.LockWrite();
	m_pCurrentSolutionSnapshot->Copy(m_Solution);
	m_SnapshotMutex.UnlockWrite();
}

bool CHrtIterativeSLEWorker::IsRunning() const
{
	return(m_eCurrentState == ECalculationState::eRUNNING);
}

bool CHrtIterativeSLEWorker::IsPaused() const
{
	return(m_eCurrentState == ECalculationState::ePAUSED);
}

size_t CHrtIterativeSLEWorker::GetCalculationRange() const
{
	return m_nCalculationRange;
}

std::shared_ptr<CDenseMatrix> CHrtIterativeSLEWorker::GetSolutionSnapshot() const
{
	return m_pCurrentSolutionSnapshot;
}

void CHrtIterativeSLEWorker::ClearSolution()
{
	m_Solution.Fill(0.0);
}
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

#include "SLESolver.h"
#include "RigidRegistrationResult.h"
#include "HRTGlobalPositioning.h"
#include "GlobalPositioningParameters.h"

#include <thread>
#include "Vectors.h"

#include "Graph.h"
#include "SimpleReadWriteLock.h"


class CHrtIterativeSLEWorker
{
public:
	enum class ECalculationState
	{
		eREADY = 0,
		eRUNNING = 1,
		ePAUSED = 2,
		eSTOPPED = 3,
		eWAITING_FOR_REGISTRATIONS = 4
	};

public:
	CHrtIterativeSLEWorker(CGlobalPositioningParameters positioningParameters, size_t nMaximumImageCount, size_t nSubImageCount = 1, size_t nSubImageIndexOffset = 0, bool bGenerateRegularisation = true);
	~CHrtIterativeSLEWorker();

	void Start();
	void Stop();
	void Join() const;
	void Pause();

	void AddRegistrations(const std::vector<CRigidRegistrationResult>& registrationResults);

	CVector2d<double> GetPositionFor(size_t nImage);

	bool IsRunning() const;
	bool IsPaused() const;
	size_t GetCalculationRange() const;
	std::shared_ptr<CDenseMatrix> GetSolutionSnapshot() const;
	std::shared_ptr<CDenseMatrix> GetNextSolutionSnapshot() const;

	//static std::vector<CMeasureImage*> images;



private:
	void InitializeMatrices(bool bGenerateRegularisation);
	void Run();


	void ClearSolution();
	void SnapShotSolution();
	void ProcessWorkQueue();
	void SolveSLE();


	size_t m_nCalculationRange = 0;

	CSimpleReadWriteLock m_SnapshotMutex;
	std::mutex m_QueueMutex;
	std::vector<CRigidRegistrationResult> m_RegistrationQueue;

	CSLESolver::CIterationParameters m_cgParameters;
	CGlobalPositioningParameters m_PositioningParameters;

	size_t m_nSubImageCount;
	size_t m_nSubImageIndexOffset;
	size_t m_nMaximumImageCount;

	std::shared_ptr<CDenseMatrix>		m_pCurrentSolutionSnapshot = nullptr;

	CSparseMatrix		m_WtW;
	CDenseMatrix		m_Wtd;
	CDenseMatrix		m_Solution;

	// The current state describes what the thread is doing at the moment
	ECalculationState m_eCurrentState;
	// The desired state describes what the user would like the thread to do. 
	// If this differs with the current state, it will be executed after the next calculation-iteration
	ECalculationState m_eDesiredState;

	std::unique_ptr<std::thread> m_Thread = nullptr;
};


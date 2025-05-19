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
#include "SNPFusionEventHandler.h"
#include "SNPFusionDataset.h"

class CSNPDatasetBatchProcessor : protected ISNPFusionEventHandler, public ICancelable
{
public:
	CSNPDatasetBatchProcessor() = default;
	virtual ~CSNPDatasetBatchProcessor();

private:
	bool m_bProcessing = false;

	std::thread m_threadProcess;
	std::mutex m_mutexOutput;
	std::mutex m_mutexLists;

	std::vector<std::shared_ptr<CSNPFusionDataset>> m_AllDatasets;

	std::list<std::shared_ptr<CSNPFusionDataset>> m_WaitingDatasets;
	std::list<std::shared_ptr<CSNPFusionDataset>> m_RunningDatasets;
	std::list<std::shared_ptr<CSNPFusionDataset>> m_FinishedDatasets;
	std::list<std::shared_ptr<CSNPFusionDataset>> m_FailedDatasets;

public:
	void StartProcessing();
	//void StartParallelProcessing();
	const std::vector<std::shared_ptr<CSNPFusionDataset>>& GetAllDatasets() const;
	const std::list<std::shared_ptr<CSNPFusionDataset>>& GetWaitingDatasets() const;
	const std::list<std::shared_ptr<CSNPFusionDataset>>& GetRunningDatasets() const;
	const std::list<std::shared_ptr<CSNPFusionDataset>>& GetFinishedDatasets() const;
	const std::list<std::shared_ptr<CSNPFusionDataset>>& GetFailedDatasets() const;

	void AddDatasetsFromFile(std::wstring filePath);
	void AddDataset(CSNPFusionDataset Dataset);
	void AddDatasets(std::vector<CSNPFusionDataset> Dataset);
	void RemoveDataset(std::wstring sDatasetName);
	void ClearDatasets();

	bool AllDatasetsAreFinished() const;

protected:
	void OnCancel() override;
	void OnUncancel() override;
	void OnSNPFusionEvent(CSNPFusionEvent snpEvent) override;

private:
	void QueueDatasets();
	void ProcessNextDataset();
	void StartThread();
	void RunAllDatasets();
	void OnDatsetFinished(CSNPFusionDataset* pDataset);
	void OnDatsetFailed(CSNPFusionDataset* pDataset);
};


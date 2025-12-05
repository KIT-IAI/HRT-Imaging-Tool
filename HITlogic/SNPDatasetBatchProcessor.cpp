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
#include "SNPDatasetBatchProcessor.h"

#include "SNPDatasetLoader.h"
#include "SNPFusionException.h"



void CSNPDatasetBatchProcessor::OnCancel()
{
	for (auto& pDataset : m_AllDatasets)
		pDataset->Cancel();
}
void CSNPDatasetBatchProcessor::OnUncancel()
{
	for (auto& pDataset : m_AllDatasets)
		pDataset->Uncancel();
}
CSNPDatasetBatchProcessor::~CSNPDatasetBatchProcessor()
{
	if (m_threadProcess.joinable())
		m_threadProcess.join();
}
void CSNPDatasetBatchProcessor::StartProcessing()
{
	QueueDatasets();
	StartThread();
}
void CSNPDatasetBatchProcessor::StartThread()
{
	if (m_threadProcess.joinable())
		m_threadProcess.join();

	m_threadProcess = std::thread(&CSNPDatasetBatchProcessor::RunAllDatasets, this);
}

void CSNPDatasetBatchProcessor::RunAllDatasets()
{
	while (!AllDatasetsAreFinished())
		ProcessNextDataset();
}


void CSNPDatasetBatchProcessor::QueueDatasets()
{
	m_RunningDatasets.clear();
	m_WaitingDatasets.clear();
	m_FailedDatasets.clear();
	m_FinishedDatasets.clear();

	for (auto& pDataset : m_AllDatasets)
		m_WaitingDatasets.push_back(pDataset);
}
void CSNPDatasetBatchProcessor::ProcessNextDataset()
{
	if (m_WaitingDatasets.empty())
		return;

	auto NextDataset = m_WaitingDatasets.front();
	m_WaitingDatasets.pop_front();

	m_RunningDatasets.push_back(NextDataset);
	NextDataset->ProcessDataset();
}
void CSNPDatasetBatchProcessor::OnSNPFusionEvent(CSNPFusionEvent Event)
{
	switch (Event.Severity)
	{
	case CSNPFusionEvent::eDatasetDone:
		OnDatsetFinished(Event.pSource);
		break;
	case CSNPFusionEvent::eError:
		OnDatsetFailed(Event.pSource);
		break;
	default:
		return;
	}
}
void CSNPDatasetBatchProcessor::OnDatsetFinished(CSNPFusionDataset* pDataset)
{
	if (m_RunningDatasets.size() != 1 || m_RunningDatasets.front().get() != pDataset)
		throw CSNPFusionException(L"Race condition during the execution of datasets.");


	auto pFinishedDataset = m_RunningDatasets.front();
	m_RunningDatasets.pop_front();

	m_FinishedDatasets.push_back(pFinishedDataset);
}
void CSNPDatasetBatchProcessor::OnDatsetFailed(CSNPFusionDataset* pDataset)
{
	if (m_RunningDatasets.size() != 1 || m_RunningDatasets.front().get() != pDataset)
		throw CSNPFusionException(L"Race condition during the execution of datasets.");

	auto pFailedDataset = m_RunningDatasets.front();

	m_FailedDatasets.push_back(pFailedDataset);
}

void CSNPDatasetBatchProcessor::AddDatasetsFromFile(wstring filePath)
{
	auto newDatasets = CSNPDatasetLoader::FromAnyFile(filePath);

	ClearDatasets();
	AddDatasets(newDatasets);
}
void CSNPDatasetBatchProcessor::AddDatasets(vector<CSNPFusionDataset> Datasets)
{
	for (auto& Dataset : Datasets)
		AddDataset(Dataset);
}
void CSNPDatasetBatchProcessor::AddDataset(CSNPFusionDataset Dataset)
{
	auto pDataset = std::make_shared<CSNPFusionDataset>(Dataset);

	m_AllDatasets.push_back(pDataset);

	pDataset->AddEventListener(this);

	if (m_bProcessing)
		m_WaitingDatasets.push_back(pDataset);
}
void CSNPDatasetBatchProcessor::ClearDatasets()
{
	if (m_bProcessing)
		Cancel();

	m_AllDatasets.clear();
	m_WaitingDatasets.clear();
	m_RunningDatasets.clear();
	m_FinishedDatasets.clear();
	m_FailedDatasets.clear();
}
void CSNPDatasetBatchProcessor::RemoveDataset(wstring sDatasetName)
{
	assert(!m_bProcessing);

	m_AllDatasets.erase(std::remove_if(m_AllDatasets.begin(), m_AllDatasets.end(), [&](std::shared_ptr<CSNPFusionDataset> pDataset)
		{
			return CStringUtilities::CompareNoCase(pDataset->Name, sDatasetName);
		}), m_AllDatasets.end());

}

const vector<std::shared_ptr<CSNPFusionDataset>>& CSNPDatasetBatchProcessor::GetAllDatasets() const
{
	return m_AllDatasets;
}
const std::list<std::shared_ptr<CSNPFusionDataset>>& CSNPDatasetBatchProcessor::GetWaitingDatasets() const
{
	return m_WaitingDatasets;
}
const std::list<std::shared_ptr<CSNPFusionDataset>>& CSNPDatasetBatchProcessor::GetRunningDatasets() const
{
	return m_RunningDatasets;
}
const std::list<std::shared_ptr<CSNPFusionDataset>>& CSNPDatasetBatchProcessor::GetFinishedDatasets() const
{
	return m_FinishedDatasets;
}
const std::list<std::shared_ptr<CSNPFusionDataset>>& CSNPDatasetBatchProcessor::GetFailedDatasets() const
{
	return m_FailedDatasets;
}
bool CSNPDatasetBatchProcessor::AllDatasetsAreFinished() const
{
	return m_FinishedDatasets.size() == m_AllDatasets.size();
}
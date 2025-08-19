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

#include <future>

#include <wx/arrstr.h>

#include "Cancelable.h"
#include "HRTImagingToolResource.h"
#include "SNPDatasetBatchProcessor.h"
#include "SNPFusionEventHandler.h"

class HRTImagingToolDialog : public wxHITDialog_Base, protected ICancelable, protected ISNPFusionEventHandler
{
public:
	HRTImagingToolDialog();
	~HRTImagingToolDialog();

	bool m_bAutomationMode = false;
	wxArrayString m_sAutomationFileNames;

private:
	static const wxString s_sModuleName;
	static const wxColour c_crNoError;
	static const wxColour c_crError;

	static CSNPDatasetParameters GenerateParameters();

	void OnInitDialog(wxInitDialogEvent& event);
	void OnBnClickedStartStop(wxCommandEvent& event);
	void OnBnClickedClose(wxCommandEvent& event);
	void OnBnClickedAddFiles(wxCommandEvent& event);
	void AddFiles(wxArrayString& files);
	void OnBnClickedRemoveFiles(wxCommandEvent& event);
	void OnPreprocessingFinished(wxEvent& event);
	void OnDatasetFinished(wxEvent& event);
	void OnClose(wxCloseEvent& event);

	virtual void OnCancel() override;
	virtual void OnUncancel() override;

	virtual void OnSNPFusionEvent(CSNPFusionEvent Event) override;

	void LockGUI();
	void LockStopButton();
	void UnlockGUI();

	void ClearLogState();
	void RaiseLogError();
	void AppendOutputLine(const wxString& sLine, CLog::ELogLevel logLevel);

	void Start();

	void InitProcessing();
	void PreprocessDatasets();
	void UnpackDataset(const wxString& datasetFilePath, CSNPFusionDataset& dataset);
	void ClassifyDataset(const wxString& datasetFilePath, CSNPFusionDataset& dataset);
	void StartProcessing();
	void FinishProcessing();

	CSNPDatasetBatchProcessor m_BatchProcessor;
	bool m_bProcessing = false;
	wxArrayString m_Files;
	std::future<void> m_PreprocessingResult;

	wxDECLARE_EVENT_TABLE();

};

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



#include "pch.h"
#include "HRTImagingToolIncludes.h"

#include <filesystem>

#include "Utilities.h"

const wxString HRTImagingToolDialog::s_sModuleName = L"CHRTImagingToolDlg";
const wxColour HRTImagingToolDialog::c_crNoError = wxColour(150, 200, 255);
const wxColour HRTImagingToolDialog::c_crError = wxColour(250, 0, 0);

CSNPDatasetParameters HRTImagingToolDialog::GenerateParameters()
{
	std::wstring regpath = L"HIT/Offline";
	auto parameters = CSNPDatasetParameterLoader::FromRegistry(regpath);

	if (!parameters.VignettingProfileBeforeRegistrationIsValid())
		throw std::exception("Vignettierungsprofil1 konnte nicht geladen werden.");
	if (!parameters.VignettingProfileBeforeCompositingIsValid())
		throw std::exception("Vignettierungsprofil2 konnte nicht geladen werden.");

	if (!parameters.IsValidParameterset())
		throw std::exception("Die Parameterüberschreibungen konnten nicht geladen werden oder sind ungültig!");

	return parameters;
}

wxDEFINE_EVENT(EVT_PREPROCESSING_FINISHED, wxThreadEvent);
wxDEFINE_EVENT(EVT_PROCESSING_FINISHED, wxThreadEvent);

wxBEGIN_EVENT_TABLE(HRTImagingToolDialog, wxHITDialog_Base)
EVT_INIT_DIALOG(HRTImagingToolDialog::OnInitDialog)
EVT_BUTTON(XRCID("m_buttonAddFiles"), HRTImagingToolDialog::OnBnClickedAddFiles)
EVT_BUTTON(XRCID("m_buttonRemoveFiles"), HRTImagingToolDialog::OnBnClickedRemoveFiles)
EVT_BUTTON(XRCID("m_buttonStartStop"), HRTImagingToolDialog::OnBnClickedStartStop)
EVT_BUTTON(XRCID("m_buttonClose"), HRTImagingToolDialog::OnBnClickedClose)
EVT_CUSTOM(EVT_PREPROCESSING_FINISHED, wxID_ANY, HRTImagingToolDialog::OnPreprocessingFinished)
EVT_CUSTOM(EVT_PROCESSING_FINISHED, wxID_ANY, HRTImagingToolDialog::OnDatasetFinished)
EVT_CLOSE(HRTImagingToolDialog::OnClose)
wxEND_EVENT_TABLE()

HRTImagingToolDialog::HRTImagingToolDialog()
{
}

HRTImagingToolDialog::~HRTImagingToolDialog()
{
}

void HRTImagingToolDialog::OnInitDialog(wxInitDialogEvent& event)
{
	m_staticTextState->SetBackgroundColour(c_crNoError);
	m_staticTextState->SetLabelText(L"Status: OK");

	if (m_bAutomationMode)
	{
		AddFiles(m_sAutomationFileNames);
		Start();
	}
}

void HRTImagingToolDialog::OnBnClickedAddFiles(wxCommandEvent& event)
{
	ASSERT(!m_bProcessing);

	wxFileDialog dlgFile(this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, L"Multi-Page-TIFF-Dateien (*.tif)|*.tif|Alle Dateien (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE | wxFD_CHANGE_DIR);
	if (dlgFile.ShowModal() != wxID_OK)
	{
		return;
	}

	wxArrayString paths;
	dlgFile.GetPaths(paths);
	AddFiles(paths);
}

void HRTImagingToolDialog::AddFiles(wxArrayString& files)
{
	for (wxFileName sFilePath : files)
	{
		wxString sFileName = sFilePath.GetName();

		ASSERT(sFilePath.FileExists());

		// Dataset already is in list
		if (m_listBoxFiles->FindString(sFileName, false) != wxNOT_FOUND)
		{
			wxString sLogLine = wxString::Format(L"Datei %s ist bereits geladen. Die neu hinzugefügte Datei wird daher ignoriert.", sFileName);
			AppendOutputLine(sLogLine);
			CLog::Log(CLog::eError, s_sModuleName.wc_str(), sLogLine.wc_str());
			continue;
		}

		m_Files.push_back(sFilePath.GetFullPath());
		m_listBoxFiles->Append(sFileName);
	}
}

void HRTImagingToolDialog::OnBnClickedRemoveFiles(wxCommandEvent& event)
{
	wxArrayInt SelectedIndices;
	if (m_listBoxFiles->GetSelections(SelectedIndices) <= 0)
	{
		return;
	}

	// Sort in INVERSE order
	SelectedIndices.Sort([](int* i1, int* i2) -> int {
		if (*i1 < *i2) return 1;
		else if (*i1 > *i2) return -1;
		else return 0;
		});

	for (int nSelectedIndex : SelectedIndices)
	{
		ASSERT(nSelectedIndex >= 0);
		ASSERT(nSelectedIndex < static_cast<int>(m_listBoxFiles->GetCount()));
		m_listBoxFiles->Delete(nSelectedIndex);
		m_Files.RemoveAt(nSelectedIndex);
	}
}

void HRTImagingToolDialog::OnBnClickedStartStop(wxCommandEvent& event)
{
	if (m_bProcessing)
		Cancel();
	else
		Start();
}

void HRTImagingToolDialog::OnBnClickedClose(wxCommandEvent& event)
{
	Close();
}

void HRTImagingToolDialog::OnPreprocessingFinished(wxEvent& event)
{
	StartProcessing();
}

void HRTImagingToolDialog::OnDatasetFinished(wxEvent& event)
{
	if (m_BatchProcessor.GetFinishedDatasets().size() == m_BatchProcessor.GetAllDatasets().size())	// Done
		FinishProcessing();
}

void HRTImagingToolDialog::OnClose(wxCloseEvent& event)
{
	Destroy();
}

void HRTImagingToolDialog::OnSNPFusionEvent(CSNPFusionEvent Event)
{
	CLog::ELogLevel LogSeverity;
	switch (Event.Severity)
	{
	case CSNPFusionEvent::eDatasetDone:
		AppendOutputLine(Event.sMessage);
		QueueEvent(new wxThreadEvent(EVT_PROCESSING_FINISHED));
		LogSeverity = CLog::eInformational;
		break;
	case CSNPFusionEvent::eError:
	case CSNPFusionEvent::eWarning:
		RaiseLogError();
		LogSeverity = CLog::eError;
		break;
	case CSNPFusionEvent::eInformation:
	case CSNPFusionEvent::eProgress:
		AppendOutputLine(wxString::Format(L"%s: %s", Event.pSource->Name, Event.sMessage));
		LogSeverity = CLog::eInformational;
		break;
	default:
		ASSERT(false);
		return;
	}

	CLog::Log(LogSeverity, CSNPFusionDataset::s_sModuleName.c_str(), Event.sMessage.c_str());
}

void HRTImagingToolDialog::OnCancel()
{
	if (wxMessageBox(L"Die Verarbeitung wird nach dem momentan verarbeiteten Datensatz abgebrochen."
		L"Dies kann einige Minuten in Anspruch nehmen."
		L"Sie werden erneut benachrichtigt, wenn die Verarbeitung abgeschlossen ist."
		L" Wollen Sie die Verarbeitung wirklich abbrechen?", L"Abbruch der Verarbeitung", wxYES_NO | wxCANCEL) == wxYES)
	{
		m_BatchProcessor.Cancel();
		LockStopButton();
	}
}

void HRTImagingToolDialog::OnUncancel()
{
	m_BatchProcessor.Uncancel();
}

void HRTImagingToolDialog::LockGUI()
{
	m_buttonStartStop->SetLabel(L"Stop");
	m_buttonAddFiles->Disable();
	m_buttonRemoveFiles->Disable();
	m_buttonClose->Disable();
}

void HRTImagingToolDialog::UnlockGUI()
{
	m_buttonStartStop->SetLabel(L"Start");
	m_buttonAddFiles->Enable();
	m_buttonRemoveFiles->Enable();
	m_buttonClose->Enable();
	m_buttonStartStop->Enable();
}

void HRTImagingToolDialog::LockStopButton()
{
	m_buttonStartStop->SetLabel(L"Stopping ...");
	m_buttonStartStop->Disable();
}

void HRTImagingToolDialog::ClearLogState()
{
	m_textCtrlLog->Clear();
	m_staticTextState->SetBackgroundColour(c_crNoError);
	m_staticTextState->SetLabel(L"Status: OK");
}

void HRTImagingToolDialog::RaiseLogError()
{
	m_staticTextState->SetBackgroundColour(c_crError);
	m_staticTextState->SetLabel(L"Status: Log-Datei prüfen!");
}

void HRTImagingToolDialog::AppendOutputLine(const wxString& sLine)
{
	CLog::Log(CLog::eNotice, L"Dlg-AppendOutputLine", sLine.wc_str());
	m_textCtrlLog->AppendText(wxString::Format(L"%s\r\n", sLine));
}

void HRTImagingToolDialog::Start()
{
	InitProcessing();
	m_PreprocessingResult = std::async(std::launch::async, [this] {PreprocessDatasets(); });
	// StartProcessing() is called after the the dataset files have been
	// preprocessed
}

void HRTImagingToolDialog::InitProcessing()
{
	ASSERT(!m_bProcessing);
	ClearLogState();
	LockGUI();
	m_bProcessing = true;
	m_BatchProcessor.Uncancel();
}

void HRTImagingToolDialog::PreprocessDatasets()
{
	wxString sLogLine(L"Bitte warten. Datensätze werden vorverarbeitet.");
	AppendOutputLine(sLogLine);
	CLog::Log(CLog::eInformational, s_sModuleName.wc_str(), sLogLine.wc_str());

	for (const auto& sFile : m_Files)
	{
		if (m_bIsCanceled)
			break;

		CSNPFusionDataset Dataset;
		try
		{
			UnpackDataset(sFile, Dataset);
			sLogLine = wxString::Format(L"Datensatz %s (%Id Bilder) geladen.", Dataset.Name, Dataset.GetImageCount());
			AppendOutputLine(sLogLine);
			CLog::Log(CLog::eInformational, s_sModuleName.wc_str(), sLogLine.wc_str());
		}
		catch (std::exception ex)
		{
			sLogLine = wxString::Format(L"Datensatz %s konnte nicht geladen werden und wird ignoriert.", Dataset.Name);
			AppendOutputLine(sLogLine);
			CLog::Log(CLog::eError, s_sModuleName.wc_str(), sLogLine.wc_str());
			continue;
		}
		ClassifyDataset(sFile, Dataset);

		m_BatchProcessor.AddDataset(Dataset);
	}
	QueueEvent(new wxThreadEvent(EVT_PREPROCESSING_FINISHED));
}

void HRTImagingToolDialog::UnpackDataset(const wxString& datasetFilePath, CSNPFusionDataset& dataset)
{
	std::wstring filePath = datasetFilePath.ToStdWstring();
	dataset = CSNPDatasetLoader::FromMultiPageTifFile(filePath, true, CSNPDatasetLoader::DefaultImageExportFolderPath(filePath));
}

void HRTImagingToolDialog::ClassifyDataset(const wxString& datasetFilePath, CSNPFusionDataset& dataset)
{
	std::filesystem::path datasetFile(datasetFilePath.ToStdWstring());
	std::filesystem::path imageFolder(CSNPDatasetLoader::DefaultImageExportFolderPath(datasetFile));
	std::filesystem::path createdClassFile(imageFolder.parent_path() / (imageFolder.filename().wstring() + L"_results.csv"));
	std::filesystem::path targetClassFile(CSNPDatasetLoader::DefaultTissueClassFilePath(datasetFile));

	std::wstring xpiwit = L"";
	wxString sLogLine;
	if (CFileUtilities::FileExists(targetClassFile))
	{
		sLogLine = wxString::Format(L"Datensatz %s ist bereits klassifiziert.", dataset.Name);
		AppendOutputLine(sLogLine);
		CLog::Log(CLog::eInformational, s_sModuleName.wc_str(), sLogLine.wc_str());
	}
	else if (GetGSS()->ReadValue(L"HIT/Offline", L"XPIWIT", xpiwit) && CFileUtilities::FileExists(xpiwit))
	{
		auto result = CUtilities::Exec(L"\"" + xpiwit + L"\" -f \"" + imageFolder.wstring() + L"\"");
		CLog::Log(CLog::eInformational, s_sModuleName.wc_str(), result.c_str());
		std::filesystem::rename(createdClassFile, targetClassFile);

		sLogLine = wxString::Format(L"Datensatz %s klassifiziert.", dataset.Name);
		AppendOutputLine(sLogLine);
		CLog::Log(CLog::eInformational, s_sModuleName.wc_str(), sLogLine.wc_str());
	}
	else
	{
		sLogLine = wxString::Format(L"Konfigurationsfehler. Klassifizierung wird übersprungen");
		AppendOutputLine(sLogLine);
		CLog::Log(CLog::eInformational, s_sModuleName.wc_str(), sLogLine.wc_str());
		return;
	}

	CHRTImageClassificationData ClassData;
	int nErrorLines = ClassData.ReadFromFile(targetClassFile);
	if (nErrorLines == -1)
	{
		sLogLine = wxString::Format(L"Die Klassifikationsdaten konnten nicht geladen werden.");
		AppendOutputLine(sLogLine);
		CLog::Log(CLog::eWarning, s_sModuleName.wc_str(), sLogLine.wc_str());
	}
	else
	{
		if (nErrorLines != 0)
		{
			sLogLine = wxString::Format(L"Die Klassifikationsdaten enthalten ungültige Zeilen.");
			AppendOutputLine(sLogLine);
			CLog::Log(CLog::eWarning, s_sModuleName.wc_str(), sLogLine.wc_str());
		}
		dataset.SetImageClassificationData(ClassData);
	}
}

void HRTImagingToolDialog::StartProcessing()
{
	// make sure that the preprocessing thread has ended
	m_PreprocessingResult.get();

	if (m_bIsCanceled)
	{
		AppendOutputLine(L"Berechnung durch Nutzer abgebrochen.");
		FinishProcessing();
		return;
	}

	if (m_BatchProcessor.GetAllDatasets().size() == 0)
	{
		AppendOutputLine(L"Fehler: Keine Datensätze geladen!");
		FinishProcessing();
		return;
	}

	CSNPDatasetParameters Parameters;
	try
	{
		Parameters = GenerateParameters();
	}
	catch (std::exception ex)
	{
		AppendOutputLine(wxString::Format(L"Fehler: %s", wxString(ex.what())));
		RaiseLogError();
		FinishProcessing();
		return;
	}

	for (auto& Dataset : m_BatchProcessor.GetAllDatasets())
	{
		Dataset->SetParameters(Parameters);
		Dataset->RemoveEventListener(this);
		Dataset->AddEventListener(this);
	}

	m_BatchProcessor.StartProcessing();
}

void HRTImagingToolDialog::FinishProcessing()
{
	ASSERT(m_bProcessing);

	wxString sMsg = wxString::Format(L"Verarbeitung beendet. Es wurden %zu der %zu Datensätze verarbeitet.",
		m_BatchProcessor.GetFinishedDatasets().size(),
		m_BatchProcessor.GetAllDatasets().size());

	m_BatchProcessor.ClearDatasets();
	m_bProcessing = false;
	Uncancel();
	UnlockGUI();
	if (!m_bAutomationMode)
	{
		wxMessageBox(sMsg);
	}
	else
	{
		Close();
	}
}

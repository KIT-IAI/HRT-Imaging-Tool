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
#include "HRTImagingToolApp.h"

#include "HRTImagingToolDialog.h"
#include "HRTImagingToolParameterDialog.h"
#include "HRTImagingToolResource.h"



wxIMPLEMENT_APP(HRTImagingToolApp);

HRTImagingToolApp::HRTImagingToolApp()
{
}

HRTImagingToolApp::~HRTImagingToolApp()
{
}

bool HRTImagingToolApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	wxXmlResource::Get()->InitAllHandlers();
	InitXmlResource();

	SetVendorName(L"iai.kit.edu");
	SetAppName(L"HIT");
	SetAppDisplayName(L"HIT");

#ifdef __AVX__
	if (!CInstructionSet::AVX())
	{
		wxMessageBox(L"AVX instruction set not available!");
		return false;
	}
#endif
#ifdef __AVX2__
	if (!CInstructionSet::AVX2())
	{
		wxMessageBox(L"AVX2 instruction set not available!");
		return false;
	}
#endif
#ifdef __AVX512F__
	if (!CInstructionSet::AVX512F())
	{
		wxMessageBox(L"AVX512 instruction set not available!");
		return false;
	}
#endif

	// enable and start logging
	if (!InitLogging())
	{
		wxMessageBox(L"The HIT log file directory does not exist.\nPlease run HIT once with administrator privileges.");
		return false;
	}
	CLog::Log(CLog::eNotice, L"HIT", L"Starting application");
	LogEnvironment();

	SettingsStore::GetInstance(L"HIT");


	setlocale(LC_TIME, "German");


	if (m_bShowParameterDialog)
	{
		CLog::Log(CLog::eNotice, L"HIT", L"Starting parameter dialog");
		// The object is deleted automatically after the dialog window destruction
		HRTImagingToolParameterDialog* pDialogWindow = new HRTImagingToolParameterDialog();
		pDialogWindow->Show();
	}
	else
	{
		CLog::Log(CLog::eNotice, L"HIT", L"Starting offline mode");
		// The object is deleted automatically after the dialog window destruction
		HRTImagingToolDialog* pDialogWindow = new HRTImagingToolDialog();
		pDialogWindow->m_bAutomationMode = m_bAutomation;
		pDialogWindow->m_sAutomationFileNames = m_sCommandLineFileNames;
		pDialogWindow->Show();
	}
	return true;
}

int HRTImagingToolApp::OnExit()
{
	CLog::Log(CLog::eNotice, L"HIT", L"Shutting down application");
	SettingsStore::DestroyInstance();

	return 0;
}

bool HRTImagingToolApp::InitLogging() const
{
	time_t t;
	t = time(nullptr);
	struct tm ts;
	localtime_s(&ts, &t);
	wchar_t date[100];
	wcsftime(date, 100, L"%Y%m%d%H%M%S", &ts);

	wxFileName f(wxStandardPaths::Get().GetExecutablePath());
	wxFileName sLogDir(f);
	sLogDir.AppendDir(L"Logs");
	if (!sLogDir.DirExists() && !sLogDir.Mkdir())
	{
		return false;
	}

	// Initialize logging
	CLog::InitLogging(wxString::Format(L"%s\\HIT_%s", sLogDir.GetPath(), date).wc_str());

	// Remove old log files
	//CFileUtilities::DeleteOldFiles(wxString::Format(L"%s\\HIT_*.log", sLogDir.GetPath()).wc_str(), 90);

	return true;
}

void HRTImagingToolApp::LogEnvironment() const {
	CLog::Log(CLog::eNotice, L"CONFIG", L"_M_IX86_FP: " STRINGIZE(_M_IX86_FP));
#ifdef __AVX__
	CLog::Log(CLog::eNotice, L"CONFIG", L"__AVX__ is enabled");
#endif
#ifdef __AVX2__
	CLog::Log(CLog::eNotice, L"CONFIG", L"__AVX2__ is enabled");
#endif
#ifdef __AVX512BW__
	CLog::Log(CLog::eNotice, L"CONFIG", L"__AVX512BW__ is enabled");
#endif
#ifdef __AVX512CD__
	CLog::Log(CLog::eNotice, L"CONFIG", L"__AVX512CD__ is enabled");
#endif
#ifdef __AVX512DQ__
	CLog::Log(CLog::eNotice, L"CONFIG", L"__AVX512DQ__ is enabled");
#endif
#ifdef __AVX512F__
	CLog::Log(CLog::eNotice, L"CONFIG", L"__AVX512F__ is enabled");
#endif
#ifdef __AVX512VL__
	CLog::Log(CLog::eNotice, L"CONFIG", L"__AVX512VL__ is enabled");
#endif

	CLog::Log(CLog::eNotice, L"WIN_INFO", L"WINVER: " STRINGIZE(WINVER));
	CLog::Log(CLog::eNotice, L"WIN_INFO", L"_WIN32_IE: " STRINGIZE(_WIN32_IE));
	CLog::Log(CLog::eNotice, L"WIN_INFO", L"_WIN32_WINNT: " STRINGIZE(_WIN32_WINNT));
	CLog::Log(CLog::eNotice, L"WIN_INFO", L"NTDDI_VERSION: " STRINGIZE(NTDDI_VERSION));
}

void HRTImagingToolApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	parser.SetDesc(g_cmdLineDesc);
	// must refuse '/' as parameter starter or cannot use "/path" style paths
	parser.SetSwitchChars(wxT("-"));
}

bool HRTImagingToolApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
	m_bShowParameterDialog = parser.Found(wxT("p"));
	m_bAutomation = parser.Found(wxT("a"));
	// to get at your unnamed parameters use

	for (int i = 0; i < parser.GetParamCount(); i++)
	{
		m_sCommandLineFileNames.Add(parser.GetParam(i));
	}

	// and other command line parameters

	// then do what you need with them.

	return true;
}

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
#include <wx/cmdline.h>

class HRTImagingToolApp : public wxApp
{
public:
	HRTImagingToolApp();
	~HRTImagingToolApp();

	virtual bool OnInit();
	virtual int OnExit();

private:
	bool InitLogging() const;
	void LogEnvironment() const;

	virtual void OnInitCmdLine(wxCmdLineParser& parser);
	virtual bool OnCmdLineParsed(wxCmdLineParser& parser);

	bool m_bShowParameterDialog = false;
	bool m_bAutomation = false;
	wxArrayString m_sCommandLineFileNames;
};

static const wxCmdLineEntryDesc g_cmdLineDesc[] =
{
	 { wxCMD_LINE_SWITCH, "h", "help", "displays help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
	 { wxCMD_LINE_SWITCH, "a", "automation", "Automation-Mode", wxCMD_LINE_VAL_NONE, wxCMD_LINE_SWITCH  },
	 { wxCMD_LINE_SWITCH, "p", "parameter", "Parameter-Dialog anzeigen", wxCMD_LINE_VAL_NONE, wxCMD_LINE_SWITCH  },
	 { wxCMD_LINE_PARAM, "f", "File-names", "Dateinamen", wxCMD_LINE_VAL_STRING, wxCMD_LINE_SWITCH | wxCMD_LINE_PARAM_MULTIPLE | wxCMD_LINE_PARAM_OPTIONAL },
	 { wxCMD_LINE_NONE }
};

wxDECLARE_APP(HRTImagingToolApp);


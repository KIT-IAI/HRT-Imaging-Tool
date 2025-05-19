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



#include "StdAfx.h"
#include "Utilities.h"

#include <cstdio>

#include <boost/process.hpp>
#include <boost/process/windows.hpp>
#include <boost/asio.hpp>

#include "StringUtilities.h"



/**	\brief Prüft, ob eine ROI (region of interest) in die Ausmaße eines
 *	Images passt.
 *
 *	Die Kontrolle ist schärfer als die der anderen Implementierungen der
 *	Methode ValidateROI, da jene eine ROI zulassen, die rechts und unten um
 *	ein Pixel zu groß ist.
 *
 *	\return TRUE, wenn die ROI zulässig ist.
 *	\return FALSE sonst.
 *
 *	\param[in] nSizeX Die Größe des Bildes in x-Richtung.
 *	\param[in] nSizeY Die Größe des Bildes in y-Richtung.
 *	\param[in] rcRect Ein Zeiger auf das Rechteck der ROI.
 *
 *	\see CMilBuffer::ValidateROI()
 *	\see CFocusOptions::ValidateROI()
 *	\see CSequenzParam::ValidateROI()
 */
BOOL CUtilities::ValidateROI(int nSizeX, int nSizeY, CRect* rcRect)
{
	// Hier unterscheidet sich diese ValidateROI von den anderen Implementierungen.
	CRect digRect(0, 0, nSizeX, nSizeY);

	CPoint topleft(rcRect->left, rcRect->top);
	CPoint bottright(rcRect->right, rcRect->bottom);

	if (!(digRect.PtInRect(topleft) && digRect.PtInRect(bottright)))
	{
		return FALSE;
	}

	if ((rcRect->top < 0) || (rcRect->left < 0) || (rcRect->bottom <= 0) || (rcRect->right <= 0))
	{
		return FALSE;
	}

	if (rcRect->left >= rcRect->right) return FALSE;
	if (rcRect->top >= rcRect->bottom) return FALSE;

	return TRUE;
}

DWORD CUtilities::IsCurrentProcessElevated()
{
	HANDLE hToken;
	OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken);

	//This only works with UAC on. With UAC off this returns TokenElevationTypeLimited, even we have full rights.
	DWORD infoLen;
	//TOKEN_ELEVATION_TYPE elevationType;
	//GetTokenInformation(hToken, TokenElevationType,&elevationType, sizeof(elevationType), &infoLen);

	//This works with UAC off, as it returns a value >0 if we run as admin or uac is off
	TOKEN_ELEVATION elevation;
	GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &infoLen);
	CloseHandle(hToken);

	return elevation.TokenIsElevated;
}
void CUtilities::RestartWithAdminRights(HINSTANCE instance)
{
	TCHAR buffer[_MAX_PATH];
	::GetModuleFileName(instance, buffer, _MAX_PATH);

	ShellExecute(NULL, _T("runas"), buffer, GetCommandLine(), NULL, SW_SHOWNORMAL);
}
std::wstring CUtilities::Exec(std::wstring command)
{
	boost::asio::io_service ios;

	std::future<std::string> data;

	boost::process::child c(command, boost::process::std_in.close(), (boost::process::std_out & boost::process::std_err) > data, ios, ::boost::process::windows::hide);

	ios.run();

	return CStringUtilities::ConvertToStdWstring(data.get());

	//std::shared_ptr<FILE> pipe(_wpopen(cmd.c_str(), L"r"), _pclose);
	//if (!pipe) throw new std::exception("Cant open pipe");
	//wchar_t buffer[128];
	//std::wstring result = L"";
	//while (!feof(pipe.get())) {
	//	if (fgetws(buffer, 128, pipe.get()) != NULL)
	//		result += buffer;
	//}
	//return result;
}

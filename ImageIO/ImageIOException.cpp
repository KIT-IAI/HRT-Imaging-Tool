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



#include "ImageIOException.h"



CImageIOException::CImageIOException(const std::wstring & sImagePath, EReason eReason, const std::wstring & sAdditionalInfo)
	: std::exception(std::string(sAdditionalInfo.begin(), sAdditionalInfo.end()).c_str())
	, sImagePath(sImagePath)
	, eReason(eReason)
	, sAdditionalInfo(sAdditionalInfo)
{
	sCompleteErrorMessage = GetExceptionMessage(*this);
}

const char* CImageIOException::what() const
{
	return sCompleteErrorMessage.c_str();
}

std::string CImageIOException::GetExceptionMessage(const CImageIOException & ex)
{
	std::string Msg;
	switch (ex.eReason)
	{
	case eUnsupportedFormatException:
		return "Unsupported image format.";
	case eTiffError:
		Msg = std::string(ex.sAdditionalInfo.begin(), ex.sAdditionalInfo.end());
		return Msg;
	case eFileNotFound:
		Msg = std::string(ex.sImagePath.begin(), ex.sImagePath.end());
		return "File " + Msg + " not found";
	case eAlloc:
		return "Could not allocate memory of the image.";
	case ePngOpeningError:
	case ePngFileError:
	case ePngError:
		Msg = std::string(ex.sAdditionalInfo.begin(), ex.sAdditionalInfo.end());
		return "Error when trying to open the .png-image: " + Msg;
	case eMilError:
		return "Mil-error. Check Log-gile.";
	case eNoReason:
	default:
		return "No Message";
	}
}
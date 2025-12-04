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

#include <vector>

#define STRINGIZE(text) STRINGIZE_A((text))
#define STRINGIZE_A(arg) STRINGIZE_I arg
#define STRINGIZE_I(text) L###text

#define STRINGIZE_ANSI(text) STRINGIZE_ANSI_I(text)
#define STRINGIZE_ANSI_I(text) #text

class CStringUtilities
{

public:

	CStringUtilities() = delete;
	virtual ~CStringUtilities() = delete;

public:

	static bool ParseString(const std::wstring& sIn, int& nOut);
	static bool ParseString(const std::wstring& sIn, long& nOut);
	static bool ParseString(const std::wstring& sIn, long long& nOut);
	static bool ParseString(const std::wstring& sIn, unsigned int& nOut);
	static bool ParseString(const std::wstring& sIn, unsigned long& nOut);
	static bool ParseString(const std::wstring& sIn, unsigned short& nOut);
	static bool ParseString(const std::wstring& sIn, unsigned long long& nOut);
	static bool ParseString(const std::wstring& sIn, float& fOut);
	static bool ParseString(const std::wstring& sIn, double& fOut);
	static bool ParseString(const std::wstring& sIn, bool& bOut);
	static bool ParseString(const std::wstring& sIn, std::wstring& sOut);
	static bool ParseString(const std::wstring& sIn, std::string& sOut);

	static std::wstring ToString(int nIn);
	static std::wstring ToString(long nIn);
	static std::wstring ToString(long long nIn);
	static std::wstring ToString(unsigned int nIn);
	static std::wstring ToString(unsigned long nIn);
	static std::wstring ToString(unsigned long long nIn);
	static std::wstring ToString(float fIn);
	static std::wstring ToString(double fIn);
	static std::wstring ToString(bool bIn);
	static std::wstring ToString(const std::wstring& sIn);
	static std::wstring ToString(const wchar_t* sIn);

	static size_t CommonPrefixLength(const std::wstring& sString1, const std::wstring& sString2);
	static size_t CommonSuffixLength(const std::wstring& sString1, const std::wstring& sString2);
	static std::wstring CommonPrefix(const std::wstring& sString1, const std::wstring& sString2);
	static std::wstring CommonSuffix(const std::wstring& sString1, const std::wstring& sString2);

	static void SplitString(const std::wstring& sIn, const std::wstring& sDelimiter, std::vector<std::wstring>& listTokens);

	static std::wstring Trim(const std::wstring& sIn);

	static bool DeleteAfterLastChar(std::wstring& sString, wchar_t chrSearched);

	// static void STRCPY(wchar_t* strDestination, const wchar_t* strSource);
	// static std::shared_ptr<std::wstring> getenv(const std::wstring& name);
	// static std::shared_ptr<std::string> getenv(const std::string& name);

	static std::string ConvertToStdString(const std::wstring& str);
	static std::wstring ConvertToStdWstring(const std::string& str);
	static std::string ConvertToStdString(const std::wstring_view str);
	static std::wstring ConvertToStdWstring(const std::string_view str);

	static std::string ConvertToUnicode(const std::wstring& str);

	static bool StartsWith(const std::wstring& in, const std::wstring& token);

	static bool CompareNoCase(const std::wstring& str1, const std::wstring& str2);
	static std::wstring ReplaceAll(const std::wstring& Input, const std::wstring& SearchFor, const std::wstring& ReplaceWith);
	static std::wstring ReplaceFirst(const std::wstring& Input, const std::wstring& SearchFor, const std::wstring& ReplaceWith);

};

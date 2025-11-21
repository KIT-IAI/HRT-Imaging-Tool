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
#include "StringUtilities.h"

#include <codecvt>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>



bool CStringUtilities::ParseString(const std::wstring& sIn, int& nOut)
{
	try
	{
		nOut = boost::lexical_cast<int>(sIn);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool CStringUtilities::ParseString(const std::wstring& sIn, long& nOut)
{
	try
	{
		nOut = boost::lexical_cast<long>(sIn);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool CStringUtilities::ParseString(const std::wstring& sIn, __int64& nOut)
{
	try
	{
		nOut = boost::lexical_cast<__int64>(sIn);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool CStringUtilities::ParseString(const std::wstring& sIn, unsigned int& nOut)
{
	try
	{
		nOut = boost::lexical_cast<unsigned int>(sIn);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool CStringUtilities::ParseString(const std::wstring& sIn, unsigned long& nOut)
{
	try
	{
		nOut = boost::lexical_cast<unsigned long>(sIn);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool CStringUtilities::ParseString(const std::wstring& sIn, unsigned short& nOut)
{
	try
	{
		nOut = boost::lexical_cast<unsigned short>(sIn);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool CStringUtilities::ParseString(const std::wstring& sIn, unsigned __int64& nOut)
{
	try
	{
		nOut = boost::lexical_cast<unsigned __int64>(sIn);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool CStringUtilities::ParseString(const std::wstring& sIn, float& fOut)
{
	try
	{
		fOut = boost::lexical_cast<float>(sIn);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool CStringUtilities::ParseString(const std::wstring& sIn, double& fOut)
{
	try
	{
		fOut = boost::lexical_cast<double>(sIn);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool CStringUtilities::ParseString(const std::wstring& sIn, bool& bOut)
{
	if (CompareNoCase(sIn, L"true"))
	{
		bOut = true;
	}
	else if (CompareNoCase(sIn, L"false"))
	{
		bOut = false;
	}
	else if (sIn.compare(L"1") == 0)
	{
		bOut = true;
	}
	else if (sIn.compare(L"0") == 0)
	{
		bOut = false;
	}
	else
	{
		return false;
	}

	return true;
}

bool CStringUtilities::ParseString(const std::wstring& sIn, std::wstring& sOut)
{
	sOut = sIn;
	return true;
}

bool CStringUtilities::ParseString(const std::wstring& sIn, std::string& sOut)
{
	sOut = ConvertToStdString(sIn);
	return true;
}

std::wstring CStringUtilities::ToString(int nIn)
{
	return boost::lexical_cast<std::wstring>(nIn);
}

std::wstring CStringUtilities::ToString(long nIn)
{
	return boost::lexical_cast<std::wstring>(nIn);
}

std::wstring CStringUtilities::ToString(__int64 nIn)
{
	return boost::lexical_cast<std::wstring>(nIn);
}

std::wstring CStringUtilities::ToString(unsigned int nIn)
{
	return boost::lexical_cast<std::wstring>(nIn);
}

std::wstring CStringUtilities::ToString(unsigned long nIn)
{
	return boost::lexical_cast<std::wstring>(nIn);
}

std::wstring CStringUtilities::ToString(unsigned __int64 nIn)
{
	return boost::lexical_cast<std::wstring>(nIn);
}

std::wstring CStringUtilities::ToString(float fIn)
{
	return boost::lexical_cast<std::wstring>(fIn);
}

std::wstring CStringUtilities::ToString(double fIn)
{
	return boost::lexical_cast<std::wstring>(fIn);
}

std::wstring CStringUtilities::ToString(bool bIn)
{
	return boost::lexical_cast<std::wstring>(bIn);
}

std::wstring CStringUtilities::ToString(const std::wstring& sIn)
{
	return sIn;
}

std::wstring CStringUtilities::ToString(const wchar_t* sIn)
{
	return std::wstring(sIn);
}

/**	\brief Bestimmt die Länge des gemeinsamen Präfixes der Zeichenketten
 *	\a sString1 und \a sString2.
 *
 *	\return Die Länge des gemeinsamen Präfixes der Zeichenketten \a sString1
 *		und \a sString2.
 *	\return 0, wenn die Zeichenketten kein gemeinsames Präfix besitzen.
 *
 *	\param[in] sString1 Eine Zeichenkette.
 *	\param[in] sString2 Eine Zeichenkette.
 */
size_t CStringUtilities::CommonPrefixLength(const std::wstring& sString1, const std::wstring& sString2)
{
	assert(sString1.length() >= 0);
	assert(sString2.length() >= 0);

	size_t nLength1 = sString1.length();
	size_t nLength2 = sString2.length();
	size_t nStopLength = nLength1 < nLength2 ? nLength1 : nLength2;
	size_t nPrefixLength = 0;

	auto pString1 = sString1.c_str();
	auto pString2 = sString2.c_str();

	while ((nPrefixLength < nStopLength) && (*pString1 == *pString2))
	{
		pString1++;
		pString2++;
		nPrefixLength++;
	}

	assert((nPrefixLength == 0) || (sString1.substr(0, nPrefixLength).compare(sString2.substr(0, nPrefixLength)) == 0));
	assert((nPrefixLength == nStopLength) || (sString1.substr(0, nPrefixLength + 1).compare(sString2.substr(0, nPrefixLength + 1)) != 0));
	assert((nPrefixLength != nLength1) || (sString1.compare(sString2.substr(0, nPrefixLength)) == 0));
	assert((nPrefixLength != nLength2) || (sString2.compare(sString1.substr(0, nPrefixLength)) == 0));

	return nPrefixLength;
}

/**	\brief Bestimmt die Länge des gemeinsamen Suffixes der Zeichenketten
 *	\a sString1 und \a sString2.
 *
 *	\return Die Länge des gemeinsamen Suffixes der Zeichenketten \a sString1
 *		und \a sString2.
 *	\return 0, wenn die Zeichenketten kein gemeinsames Suffix besitzen.
 *
 *	\param[in] sString1 Eine Zeichenkette.
 *	\param[in] sString2 Eine Zeichenkette.
 */
size_t CStringUtilities::CommonSuffixLength(const std::wstring& sString1, const std::wstring& sString2)
{
	assert(sString1.length() >= 0);
	assert(sString2.length() >= 0);

	size_t nLength1 = sString1.length();
	size_t nLength2 = sString2.length();
	size_t nStopLength = nLength1 < nLength2 ? nLength1 : nLength2;
	size_t nSuffixLength = 0;

	auto pString1 = sString1.c_str() + nLength1 - 1;
	auto pString2 = sString2.c_str() + nLength2 - 1;

	while ((nSuffixLength < nStopLength) && (*pString1 == *pString2))
	{
		pString1--;
		pString2--;
		nSuffixLength++;
	}

	assert((nSuffixLength == 0) || (sString1.substr(nLength1 - nSuffixLength).compare(sString2.substr(nLength2 - nSuffixLength)) == 0));
	assert((nSuffixLength == nStopLength) || (sString1.substr(nLength1 - nSuffixLength - 1).compare(sString2.substr(nLength2 - nSuffixLength - 1)) != 0));
	assert((nSuffixLength != nLength1) || (sString1.compare(sString2.substr(nLength2 - nSuffixLength)) == 0));
	assert((nSuffixLength != nLength2) || (sString2.compare(sString1.substr(nLength1 - nSuffixLength)) == 0));

	return nSuffixLength;
}

/**	\brief Bestimmt das gemeinsame Präfix der Zeichenketten \a sString1 und
 *	\a sString2.
 *
 *	\return Das gemeinsame Präfix der Zeichenketten \a sString1 und
 *		\a sString2.
 *	\return Eine leere Zeichenkette, wenn die Zeichenketten kein gemeinsames
 *		Präfix besitzen.
 *
 *	\param[in] sString1 Eine Zeichenkette.
 *	\param[in] sString2 Eine Zeichenkette.
 */
std::wstring CStringUtilities::CommonPrefix(const std::wstring& sString1, const std::wstring& sString2)
{
	return sString1.substr(0, CommonPrefixLength(sString1, sString2));
}

/**	\brief Bestimmt das gemeinsame Suffix der Zeichenketten \a sString1 und
 *	\a sString2.
 *
 *	\return Das gemeinsame Suffix der Zeichenketten \a sString1 und
 *		\a sString2.
 *	\return Eine leere Zeichenkette, wenn die Zeichenketten kein gemeinsames
 *		Suffix besitzen.
 *
 *	\param[in] sString1 Eine Zeichenkette.
 *	\param[in] sString2 Eine Zeichenkette.
 */
std::wstring CStringUtilities::CommonSuffix(const std::wstring& sString1, const std::wstring& sString2)
{
	return sString1.substr(sString1.length() - CommonSuffixLength(sString1, sString2));
}

/**	\brief Zerlegt eine Zeichenkette anhand einer Trenn-Zeichenkette in
 *	Teilketten.
 *
 *	\param[in] sIn Die Zeichenkette, die zerlegt werden soll.
 *	\param[in] sDelimiter Die Trenn-Zeichenkette, die in \a sIn die
 *		Teilketten voneinander abgrenzt.
 *	\param[out] listTokens Enthält nach der Rückkehr aus der Methode die
 *		Teilketten.
 */
void CStringUtilities::SplitString(const std::wstring& sIn, const std::wstring& sDelimiter, std::vector<std::wstring>& listTokens)
{
	listTokens.clear();

	size_t step = sDelimiter.length();
	size_t nLastIndex = 0;
	size_t nNextIndex = sIn.find(sDelimiter, nLastIndex);
	while (nNextIndex != std::wstring::npos)
	{
		listTokens.push_back(sIn.substr(nLastIndex, nNextIndex - nLastIndex));
		nLastIndex = nNextIndex + step;
		nNextIndex = sIn.find(sDelimiter, nLastIndex);
	}

	assert(nNextIndex == std::wstring::npos);

	listTokens.push_back(sIn.substr(nLastIndex));
}

std::wstring CStringUtilities::Trim(const std::wstring& sIn)
{
	auto out = sIn;
	boost::trim(out);
	return out;
}

/**	\brief Schneidet das Ende der Zeichenkette ab dem letzten Vorkommen des
 *	gegebenen Zeichens ab.
 *
 *	Das gefundene letzte Vorkommen des Zeichens wird dabei ebenfalls
 *	entfernt.
 *
 *	Diese Methode kann beispielsweise benutzt werden, um die Dateiendung
 *	eines Dateinamens zu entfernen, indem für chrSearched der Punkt _T('.')
 *	übergeben wird.
 *
 *	\return \c true, wenn das Zeichen in der Zeichenkette gefunden und sie
 *		entsprechend gekürzt wurde.
 *	\return \c false, wenn die Zeichenkette leer ist oder das gesuchte
 *		Zeichen nicht enthält.
 *
 *	\param[in,out] sString Die Zeichenkette, die gekürzt werden soll.
 *	\param[in] chrSearched Das Zeichen, das in der Zeichenkette gesucht
 *		werden soll.
 */
bool CStringUtilities::DeleteAfterLastChar(std::wstring& sString, wchar_t chrSearched)
{
	size_t lastIndex = sString.rfind(chrSearched);
	if (lastIndex != std::wstring::npos)
	{
		sString.erase(lastIndex);
		return true;
	}
	else
	{
		return false;
	}
}

void CStringUtilities::STRCPY(wchar_t* strDestination, const wchar_t* strSource)
{
	wcscpy_s(strDestination, wcslen(strSource) + 1, strSource);
}

std::shared_ptr<std::wstring> CStringUtilities::getenv(const std::wstring& name)
{
	wchar_t* str;
	size_t len;
	auto err = _wdupenv_s(&str, &len, name.c_str());
	if (err || str == nullptr)
	{
		return std::shared_ptr<std::wstring>();
	}
	else
	{
		std::shared_ptr<std::wstring > ret = std::make_shared<std::wstring>(str);
		free(str);
		return ret;
	}
}

std::shared_ptr<std::string> CStringUtilities::getenv(const std::string& name)
{
	char* str;
	size_t len;
	auto err = _dupenv_s(&str, &len, name.c_str());
	if (err || str == nullptr)
	{
		return std::shared_ptr<std::string>();
	}
	else
	{
		std::shared_ptr<std::string > ret = std::make_shared<std::string>(str);
		free(str);
		return ret;
	}
}

std::string CStringUtilities::ConvertToStdString(const std::wstring& str)
{
	char* a = new char[str.size() + 2];
	size_t retval;
	wcstombs_s(&retval, a, str.size() + 2, str.c_str(), str.size());
	std::string tmp(a);
	delete[] a;
	return tmp;
}

std::string CStringUtilities::ConvertToStdString(const std::wstring_view str)
{
	char* a = new char[str.size() + 2];
	size_t retval;
	wcstombs_s(&retval, a, str.size() + 2, str.data(), str.size());
	std::string tmp(a);
	delete[] a;
	return tmp;
}

std::wstring CStringUtilities::ConvertToStdWstring(const std::string& str)
{
	wchar_t* w = new wchar_t[str.size() + 2];
	size_t retval;
	mbstowcs_s(&retval, w, str.size() + 2, str.c_str(), str.size());
	std::wstring tmp(w);
	delete[] w;
	return tmp;
}

std::wstring CStringUtilities::ConvertToStdWstring(const std::string_view str)
{
	wchar_t* w = new wchar_t[str.size() + 2];
	size_t retval;
	mbstowcs_s(&retval, w, str.size() + 2, str.data(), str.size());
	std::wstring tmp(w);
	delete[] w;
	return tmp;
}

std::string CStringUtilities::ConvertToUnicode(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}

bool CStringUtilities::StartsWith(const std::wstring& in, const std::wstring& token)
{
	return boost::starts_with(in, token);
}

bool CStringUtilities::CompareNoCase(const std::wstring& str1, const std::wstring& str2)
{
	return boost::iequals(str1, str2);
}

std::wstring CStringUtilities::ReplaceAll(const std::wstring& Input, const std::wstring& SearchFor, const std::wstring& ReplaceWith)
{
	return boost::replace_all_copy(Input, SearchFor, ReplaceWith);
}

std::wstring CStringUtilities::ReplaceFirst(const std::wstring& Input, const std::wstring& SearchFor, const std::wstring& ReplaceWith)
{
	return boost::replace_first_copy(Input, SearchFor, ReplaceWith);
}

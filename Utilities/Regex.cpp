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
#include "Regex.h"

using namespace std;
using namespace boost;



class CRegex::Impl
{
public:
	basic_regex<wchar_t> rx;
	match_results<wstring::const_iterator> mr;
	//needed for the matches above. The matches are just pointers to this string.
	wstring m_str;
	Impl()
	{

	}
};

CRegex::CRegex(const std::wstring& regex, bool CaseSensitive) : pimpl(new CRegex::Impl())
{
	wstring sr = regex;
	regex_constants::syntax_option_type flags = static_cast<regex_constants::syntax_option_type>(CaseSensitive ? (regex_constants::icase) : (regex_constants::match_default));
	pimpl->rx = basic_regex<wchar_t>(sr.begin(), sr.end(), regex_constants::sed | flags);
}

CRegex::~CRegex(void)
{
}

std::wstring CRegex::Replace(const std::wstring& str, const std::wstring& replacement)
{
	return regex_replace(str, pimpl->rx, replacement);
}

bool CRegex::Search(const std::wstring& str)
{
	pimpl->m_str = str;
	return regex_search(pimpl->m_str, pimpl->mr, pimpl->rx);
}

bool CRegex::Match(const std::wstring& str)
{
	pimpl->m_str = str;
	return regex_match(pimpl->m_str, pimpl->mr, pimpl->rx);
}

wstring CRegex::GetMatch(int index)
{
	return pimpl->mr[index].str();
}

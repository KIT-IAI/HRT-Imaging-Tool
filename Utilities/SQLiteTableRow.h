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
#include "SQLiteException.h"
#include "StringUtilities.h"
#include "AbstractVector.h"

class CSQLiteTable;
/*
This class represents a row in a database. In order to be stored in the database, the type you're trying to save
has to be convertable to a string. Please make sure CStringUtilities contains a proper override for that.
*/
class CSQLiteTableRow
{
public:
	explicit CSQLiteTableRow(size_t size = 0);

	// If you want to implement a new type do the following:
	// If the type you're trying to implement is in Utilities, add it to the .cpp of this class
	// If it is in DIPLOMKernel, add it in CDIPLOMTableRow
	static CSQLiteTableRow From(const std::vector<std::pair<std::wstring, std::wstring>>& KeyValuePairs);
	static CSQLiteTableRow From(const std::vector<std::wstring>& Values);
	static CSQLiteTableRow From(const CAbstractVector& Vector);

	// Has to be implemented in the cpp for the desired types
	template <typename T> T Convert() const;
	template <typename T> std::vector<T> ConvertToVector() const;


	template <typename T> void AddUnnamed(const T& value);
	template <typename T> void AddKeyValuePair(const std::wstring& key, const T& value);
	template <typename T> T Get(size_t column) const;
	template <typename T> T Get(const std::wstring& key) const;
	size_t size() const;

	void InsertInto(CSQLiteTable& Table) const;

	std::wstring GenerateWildcardStatement(const std::wstring& sTableName) const;

	std::vector<std::wstring>::const_iterator begin() const;
	std::vector<std::wstring>::const_iterator end() const;

private:
	bool bUsesKeyValuePairs;
	std::vector<std::wstring> Keys;
	std::vector<std::wstring> Values;
};



/////////////////////////// TEMPLATE IMPLEMENTAITONS /////////////////

template <typename T>
std::vector<T> CSQLiteTableRow::ConvertToVector() const
{
	std::vector<T> out;
	for (size_t i = 0; i < size(); i++)
		out.push_back(Get<T>(i));
	return out;
}

template<typename T>
inline void CSQLiteTableRow::AddUnnamed(const T& value)
{
	if (size() > 0 && bUsesKeyValuePairs)
		throw CSQLException(L"Cannot mix named and unnamed Values in one Table Row! Either use AddUnnamed() in the right order or use AddKeyValuePair()!");

	bUsesKeyValuePairs = false;
	Values.push_back(CStringUtilities::ToString(value));
}
template<typename T>
inline void CSQLiteTableRow::AddKeyValuePair(const std::wstring& key, const T& value)
{
	if (size() > 0 && !bUsesKeyValuePairs)
		throw CSQLException(L"Cannot mix named and unnamed Values in one Table Row! Either use AddUnnamed() in the right order or use AddKeyValuePair()!");

	bUsesKeyValuePairs = true;
	Keys.push_back(key);
	Values.push_back(CStringUtilities::ToString(value));
}

template<typename T>
inline T CSQLiteTableRow::Get(size_t column) const
{
	T Out;
	CStringUtilities::ParseString(Values.at(column), Out);
	return Out;
}

template<typename T>
inline T CSQLiteTableRow::Get(const std::wstring& key) const
{
	auto it = std::find(Keys.begin(), Keys.end(), key);
	if (it == Keys.end())
		throw CSQLException(L"Key " + key + L"not found.");
	size_t col = std::distance(Keys.begin(), it);
	return Get<T>(col);
}

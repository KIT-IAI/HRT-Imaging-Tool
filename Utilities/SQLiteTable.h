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
#include "SQLiteTableRow.h"
#include "DenseMatrix.h"

class CSQLiteDatabase;

class CSQLiteTable
{
public:
	class iterator;

	explicit CSQLiteTable(std::wstring Name);
	~CSQLiteTable() = default;

	static CSQLiteTable From(const std::wstring& Name, const CDenseMatrix& Matrix);
	static CSQLiteTable From(const std::wstring& Name, const std::vector<std::wstring>& Values);
	static CSQLiteTable From(const std::wstring& Name, const std::vector<CSQLiteTableRow>& Rows);
	template <typename T> T Convert() const;

	void AddColumn(const std::wstring& Name, const std::wstring& Type);
	void AddRow(const CSQLiteTableRow& Row);

	const CSQLiteTableRow& GetRow(size_t nIndex) const;
	const CSQLiteTableRow& operator[](size_t nIndex) const;
	CSQLiteTableRow GetColumn(size_t nIndex) const;

	std::wstring GetName() const;
	size_t size() const;

	void InsertIntoDatabaseWithoutContent(CSQLiteDatabase& Database, bool bDropIfExists = true) const;
	void InsertIntoDatabase(CSQLiteDatabase& Database, bool bDropIfExists = true) const;
	void InsertContentIntoDatabase(CSQLiteDatabase& Database) const;

	static std::wstring GetDefaultColumnName(size_t nColumn);
private:
	std::vector<std::wstring> GenerateColumns() const;
	static const std::wstring DefaultColumnName;


public:
	iterator begin() const;
	iterator end() const;

	class iterator : public std::iterator<std::input_iterator_tag, CSQLiteTableRow>
	{
	public:
		iterator(const CSQLiteTable& Table, size_t nIndex = 0);
		const CSQLiteTableRow& operator*() const;
		iterator& operator++();
		iterator& operator++(int);
		ptrdiff_t operator-(iterator) const;
		bool operator!=(const iterator& rhs) const;
		bool operator==(const iterator& rhs) const;
	private:
		size_t nIndex = 0;
		const CSQLiteTable& Table;
	};


private:
	std::wstring m_TableName;
	std::vector<CSQLiteTableRow> m_Rows;
	std::vector<std::wstring> m_ColumnNames;
	std::vector<std::wstring> m_ColumnTypes;
};


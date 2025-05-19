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

class CDenseMatrix;
class CDenseVector;

class  CTextFileOutput
{
public:
	CTextFileOutput();
	virtual ~CTextFileOutput();

	void Write(int nInt);
	void Write(double fDouble);
	void Write(float fFloat);
	void Write(float* fArray, int nSize);
	void Write(float** fArray, int nSizeY, int nSizeX);
	void Write(std::wstring sString);

	void Writeln(int nInt);
	void Writeln(double fDouble);
	void Writeln(float fFloat);
	void Writeln(float* fArray, int nSize);
	void Writeln(std::wstring sString = _T(""));
	void Writeln(const CDenseMatrix& mat);
	void Writeln(const CDenseVector& mat);
	template <typename T> void Writeln(const std::vector<T>& vec);

	template <typename T> CTextFileOutput& operator<<(const T& in);

	bool Save(std::wstring sPathname) const;

	void Reset();

private:
	std::wstring m_sDump;

};

template <typename T> inline CTextFileOutput& CTextFileOutput::operator<<(const T& in)
{
	Writeln(in);
	return *this;
}
template <typename T> inline void CTextFileOutput::Writeln(const std::vector<T>& vec)
{
	for (const auto& content : vec)
		Writeln(content);
}
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

#include <array>

// IMPORTANT NOTE: The classes that have distances associated must receive
//		consecutive values starting with 0; the value is used to index into
//		the distances array CHRTImageClassificationSet::Distances
enum class EHRTImageClassificationClass {
	eReject = -1,
	eEpithelium = 0,
	eSNP = 1,
	eStroma = 2
};

struct CHRTImageClassificationSet
{
	std::wstring Filepath = L"";
	EHRTImageClassificationClass Class = EHRTImageClassificationClass::eReject;
	std::array<double, 3> Distances = { 0.0, 0.0, 0.0 };
};

class CHRTImageClassificationData
{
public:
	CHRTImageClassificationData();
	~CHRTImageClassificationData();

	int ReadFromFile(const std::wstring& filepath);

	bool IsLoaded() const;
	bool HasDistances() const;
	size_t GetSize() const;

	const CHRTImageClassificationSet& GetDataset(size_t ind) const;
	const std::wstring& GetFilepath(size_t ind) const;
	EHRTImageClassificationClass GetClass(size_t ind) const;
	const std::array<double, 3>& GetDistances(size_t ind) const;
	double GetDistance(size_t ind, EHRTImageClassificationClass cls) const;

	CHRTImageClassificationData Filter(std::list<size_t> inds) const;

private:
	bool m_bLoaded = false;
	bool m_bDistances = false;
	std::vector<CHRTImageClassificationSet> m_Data;
};

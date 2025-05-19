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

#include "HRTImageClassificationData.h"

CHRTImageClassificationData::CHRTImageClassificationData()
{
}

CHRTImageClassificationData::~CHRTImageClassificationData()
{
}

int CHRTImageClassificationData::ReadFromFile(const wstring& filepath)
{
	m_Data.clear();
	m_bLoaded = false;
	m_bDistances = false;

	wifstream ifs(filepath);

	if (!ifs.is_open())
	{
		return -1;
	}

	const wchar_t delim = L';';
	auto tokenizeLine = [delim](const wstring& line, vector<wstring>& tokens) -> void {
		tokens.clear();
		size_t ind1 = 0;
		size_t ind2 = line.find(delim, ind1);
		while (ind2 != wstring::npos)
		{
			tokens.push_back(line.substr(ind1, ind2 - ind1));
			ind1 = ind2 + 1;
			ind2 = line.find(delim, ind1);
		}
		tokens.push_back(line.substr(ind1));
		};

	auto parseDistance = [](const wstring& str, double& dist) -> bool {
		wistringstream iss(str);
		double temp;
		iss >> temp;
		if (iss)
		{
			dist = temp;
			return true;
		}
		else
		{
			return false;
		}
		};

	auto parseClass = [](const wstring& str, EHRTImageClassificationClass& cls) -> bool {
		wistringstream iss(str);
		int temp;
		iss >> temp;
		if (iss)
		{
			cls = static_cast<EHRTImageClassificationClass>(temp);
			return true;
		}
		else
		{
			return false;
		}
		};

	int errorlines = 0;
	wstring line;
	bool bNamePresent = false;
	ptrdiff_t lineindex = -1;
	size_t ntokens = 0;
	vector<wstring> tokens;
	while (std::getline(ifs, line))
	{
		lineindex++;

		// only for first (correct) line
		if (ntokens == 0)
		{
			ntokens = std::count(line.begin(), line.end(), delim) + 1;
			if (ntokens == 1)
			{
				bNamePresent = false;
				m_bDistances = false;
			}
			else if (ntokens == 2)
			{
				bNamePresent = true;
				m_bDistances = false;
			}
			else if (ntokens == 4)
			{
				bNamePresent = false;
				m_bDistances = true;
			}
			else if (ntokens == 5)
			{
				bNamePresent = true;
				m_bDistances = true;
			}
			else
			{
				ntokens = 0;
				errorlines++;
				continue;
			}
		}

		ASSERT(ntokens > 0);

		tokenizeLine(line, tokens);
		if (tokens.size() != ntokens)
		{
			errorlines++;
			continue;
		}

		ASSERT(tokens.size() == ntokens);

		CHRTImageClassificationSet set;
		if (bNamePresent)
			set.Filepath = tokens[0];
		else
		{
			wostringstream oss;
			oss << std::setw(5) << std::setfill(L'0') << lineindex;
			set.Filepath = oss.str();
		}
		if (m_bDistances)
		{
			if (!parseDistance(tokens[1], set.Distances[0])) { errorlines++; continue; }
			if (!parseDistance(tokens[2], set.Distances[1])) { errorlines++; continue; }
			if (!parseDistance(tokens[3], set.Distances[2])) { errorlines++; continue; }
		}
		if (!parseClass(tokens[ntokens - 1], set.Class)) { errorlines++; continue; }
		m_Data.push_back(set);
	}

	if (m_Data.empty())
	{
		return -1;
	}
	else
	{
		m_bLoaded = true;
		return errorlines;
	}
}

bool CHRTImageClassificationData::IsLoaded() const
{
	return m_bLoaded;
}

bool CHRTImageClassificationData::HasDistances() const
{
	return m_bDistances;
}

size_t CHRTImageClassificationData::GetSize() const
{
	return m_Data.size();
}

const CHRTImageClassificationSet& CHRTImageClassificationData::GetDataset(size_t ind) const
{
	ASSERT(ind < GetSize());
	return m_Data[ind];
}

const wstring& CHRTImageClassificationData::GetFilepath(size_t ind) const
{
	ASSERT(ind < GetSize());
	return m_Data[ind].Filepath;
}

EHRTImageClassificationClass CHRTImageClassificationData::GetClass(size_t ind) const
{
	ASSERT(ind < GetSize());
	return m_Data[ind].Class;
}

const array<double, 3>& CHRTImageClassificationData::GetDistances(size_t ind) const
{
	ASSERT(ind < GetSize());
	return m_Data[ind].Distances;
}

double CHRTImageClassificationData::GetDistance(size_t ind, EHRTImageClassificationClass cls) const
{
	ASSERT(ind < GetSize());
	ASSERT(static_cast<int>(cls) >= 0);
	return m_Data[ind].Distances[static_cast<int>(cls)];
}

CHRTImageClassificationData CHRTImageClassificationData::Filter(std::list<size_t> inds) const
{
	CHRTImageClassificationData ret;
	if (IsLoaded())
	{
		ret.m_bLoaded = IsLoaded();
		ret.m_bDistances = HasDistances();
		for (auto ind : inds)
		{
			ret.m_Data.push_back(GetDataset(ind));
		}
	}
	return ret;
}

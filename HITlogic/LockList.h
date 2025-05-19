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
#include <list>
#include <set>
#include <mutex>

class CLockList
{
public:
	enum EImageExclusion
	{
		IMAGE_FREE = 0,
		SUCCESSFUL_REGISTRATION = -1,
		UNSUCCESSFUL_REGISTRATION = -2,
		IMAGE_EXCLUDED_BY_SUCCESSFUL_NEIGHBOR = -3,
		EXCEEDS_MAXIMUM_IMAGE_DISTANCE = -4,
		EXCLUDED_FOR_THIS_ITERATION = -5
	};

	CLockList();
	CLockList(const CLockList&);
	explicit CLockList(size_t size);
	~CLockList();

	void SetSize(size_t);
	void UnlockImages(const std::set<size_t>& toUnlock);
	void ExcludeImages(const std::set<size_t>& toExclude, EImageExclusion eReason);
	void LockImages(const std::set<size_t>& toExclude);
	void SetLockStatus(size_t nFromIndex, size_t nToIndex, EImageExclusion eImageExclusion);
	void FreeLocks(size_t nFromIndex, size_t nToIndex);
	void WhereXSetY(size_t nFromIndex, size_t nToIndex, EImageExclusion eWhereCondition, EImageExclusion eSetValue);

	INT_PTR& operator[](const size_t);
	const INT_PTR& operator[](const size_t) const;
private:
	std::vector<INT_PTR> m_LockList;
	mutable std::mutex m_Mutex;

};


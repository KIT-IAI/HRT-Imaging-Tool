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
#include "LockList.h"

#define LOCK std::lock_guard<std::mutex> lock(m_Mutex);

CLockList::CLockList()
	:CLockList(0)
{
}

CLockList::CLockList(const CLockList& toCopyFrom)
{
	m_LockList = toCopyFrom.m_LockList;
}

CLockList::CLockList(size_t size)
{
	LOCK
		m_LockList.assign(size, IMAGE_FREE);
}


CLockList::~CLockList()
{
}

void CLockList::SetSize(size_t size)
{
	LOCK
		m_LockList.resize(size, IMAGE_FREE);
}

void CLockList::UnlockImages(const std::set<size_t>& toUnlock)
{
	LOCK
		for (auto elementToUnlock : toUnlock)
		{
			if (m_LockList[elementToUnlock] > IMAGE_FREE)
				m_LockList[elementToUnlock]--;
		}
}

void CLockList::ExcludeImages(const std::set<size_t>& toExclude, EImageExclusion eReason)
{
	LOCK
		for (auto a : toExclude)
			m_LockList[a] = eReason;
}

void CLockList::LockImages(const std::set<size_t>& toLock)
{
	LOCK
		for (auto elementToLock : toLock)
		{
			if (m_LockList[elementToLock] >= IMAGE_FREE)
				m_LockList[elementToLock]++;
		}
}

void CLockList::SetLockStatus(size_t nFromIndex, size_t nToIndex, EImageExclusion eImageExclusion)
{
	LOCK
		for (auto i = nFromIndex; i < nToIndex; i++)
			m_LockList[i] = eImageExclusion;
}

INT_PTR& CLockList::operator[](const size_t i)
{
	LOCK
		return m_LockList[i];
}

const INT_PTR& CLockList::operator[](const size_t i) const
{
	LOCK
		return m_LockList[i];
}
void CLockList::WhereXSetY(size_t nFromIndex, size_t nToIndex, EImageExclusion eWhereCondition, EImageExclusion eSetValue)
{
	LOCK
		for (auto i = nFromIndex; i < nToIndex; i++)
		{
			if (m_LockList[i] == eWhereCondition)
				m_LockList[i] = eSetValue;
		}
}
void CLockList::FreeLocks(size_t nFromIndex, size_t nToIndex)
{
	LOCK
		for (auto i = nFromIndex; i < nToIndex; i++)
		{
			if (m_LockList[i] > 0)
				m_LockList[i] = IMAGE_FREE;
		}
}

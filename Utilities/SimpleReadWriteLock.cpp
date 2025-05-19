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
#include "SimpleReadWriteLock.h"



CSimpleReadWriteLock::CSimpleReadWriteLock()
	:m_nReadCounter(0),
	m_bNewReaderAllowed(true)
{
}

CSimpleReadWriteLock::~CSimpleReadWriteLock()
{
}

void CSimpleReadWriteLock::LockRead() const
{
	m_WriteMutex.lock();
	while (!m_bNewReaderAllowed)
		std::this_thread::yield();
	++m_nReadCounter;
	m_WriteMutex.unlock();
}

void CSimpleReadWriteLock::LockWrite() const
{
	m_WriteMutex.lock();
	m_bNewReaderAllowed = false;
	while (m_nReadCounter > 0)
		std::this_thread::yield();
}

void CSimpleReadWriteLock::UnlockWrite() const
{
	m_bNewReaderAllowed = true;
	m_WriteMutex.unlock();
}

void CSimpleReadWriteLock::UnlockRead() const
{
	assert(m_nReadCounter > 0);
	--m_nReadCounter;
}

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

#include "ProgressListener.h"
#include <list>

class IProgressing
{
protected:
	std::list<IProgressListener*> m_ProgressListeners;
	virtual CProgress GetProgress() = 0;
public:
	virtual ~IProgressing()
	{
	}

	virtual void AddProgressListener(IProgressListener* progressListener) final
	{
		m_ProgressListeners.push_back(progressListener);
	};
	virtual void RemoveProgressListener(IProgressListener* progressListener) final
	{
		m_ProgressListeners.remove(progressListener);
	};
	virtual void AddProgressListeners(std::list<IProgressListener*> progressListeners) final
	{
		for (auto listener : progressListeners)
			AddProgressListener(listener);
	};
	virtual void RemoveProgressListeners(std::list<IProgressListener*> progressListeners) final
	{
		for (auto listener : progressListeners)
			RemoveProgressListener(listener);
	};
	virtual void ReportProgress()
	{
		auto progress = GetProgress();
		ReportProgress(progress);
	}
	virtual void ReportProgress(CProgress progress) final
	{
		for (auto listener : m_ProgressListeners)
			listener->OnProgress(progress);
	}
	virtual void ReportProgress(IProgressListener* listener) final
	{
		listener->OnProgress(GetProgress());
	}
};

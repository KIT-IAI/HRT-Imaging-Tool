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

class CProgress
{
public:
	size_t nCurrentStep = 0;
	size_t nStepCount = 1;

	size_t nApproximatedWorkUnits = 1;
	size_t nFinishedWorkUnits = 0;

	size_t nSecondsPassed = 0;
	std::vector<CProgress> SubStepProgress;

	CProgress() {};
	CProgress(size_t nCurrentStep, size_t nStepCount, size_t nTotalSecondsPassed, size_t nApproximatedWorkUnits, size_t nFinishedWorkUnits)
		:nCurrentStep(nCurrentStep),
		nStepCount(nStepCount),
		nSecondsPassed(nTotalSecondsPassed),
		nApproximatedWorkUnits(nApproximatedWorkUnits),
		nFinishedWorkUnits(nFinishedWorkUnits) {
	};

	CProgress GetCurrentProgress()
	{
		if (nCurrentStep < SubStepProgress.size())
			return SubStepProgress[nCurrentStep];
		return *this;
	}
};

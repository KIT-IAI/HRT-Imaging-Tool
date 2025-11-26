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

#include "RegistrationStrategyFactory.h"
#include "StrategyBackForth.h"
#include "StrategyComplete.h"
#include "StrategyWindow.h"
#include "StrategyStep.h"
#include "StrategySector.h"
#include "StrategyGraph.h"
#include "StrategyPlanes.h"

std::shared_ptr<CRegistrationStrategy> CRegistrationStrategyFactory::CreateStrategy(ERegistrationStrategy eRegistrationStrategy, const CRegistrationStrategyParameters& param)
{
	std::shared_ptr<CRegistrationStrategy> pStrategy;
	auto Parameters = param;

	switch (eRegistrationStrategy)
	{
	case ERegistrationStrategy::eSimpleForth:
		pStrategy = std::make_shared<CStrategyStep>();
		Parameters.nMaxDistance = 1;
		Parameters.nStepSize = 1;
		Parameters.nWindowSize = 1;
		break;
	case ERegistrationStrategy::eBackForth:
		pStrategy = std::make_shared<CStrategyBackForth>();
		break;
	case ERegistrationStrategy::eComplete:
		pStrategy = std::make_shared<CStrategyComplete>();
		break;
	case ERegistrationStrategy::eWindow:
		pStrategy = std::make_shared<CStrategyWindow>();
		break;
	case ERegistrationStrategy::eStep:
		pStrategy = std::make_shared<CStrategyStep>();
		break;
	case ERegistrationStrategy::eSector:
		pStrategy = std::make_shared<CStrategySector>();
		break;
	case ERegistrationStrategy::ePositionBased:
		pStrategy = std::make_shared<CStrategyGraph>();
		break;
	case ERegistrationStrategy::ePlaneSelect:
		pStrategy = std::make_shared<CStrategyPlanes>();
		break;
	case ERegistrationStrategy::eInvalid:
	default:
		assert(false);
		return nullptr;
	}


	pStrategy->SetParameters(Parameters);

	return pStrategy;
}

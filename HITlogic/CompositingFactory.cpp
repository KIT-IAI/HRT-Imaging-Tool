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

#include "CompositingFactory.h"
#include "CompositingFusion.h"
#include "CompositingTiling.h"
#include "Compositing3DCloud.h"
#include "CompositingVoxelBuffer.h"
#include "CompositingMax.h"

std::shared_ptr<CCompositing> CCompositingFactory::CreateCompositing(const CCompositingParameters& param)
{
	std::shared_ptr<CCompositing> pCompositing;
	auto Parameters = param;

	switch (param.eCompositingMode)
	{
	case CCompositingParameters::ECompositingType::eFusion:
		pCompositing = std::make_shared<CCompositingFusion>(param);				//Vorsicht!!!!!	//muss wieder zu Fusion geändert werden!
		break;
	case CCompositingParameters::ECompositingType::e3DCloud:
		pCompositing = std::make_shared<CCompositing3DCloud>(param);
		break;
	case CCompositingParameters::ECompositingType::eVoxelBuffer:
		pCompositing = std::make_shared<CCompositingVoxelBuffer>(param);
		break;
	case CCompositingParameters::ECompositingType::eTiling:
		pCompositing = std::make_shared<CCompositingTiling>(param);
		break;
	case CCompositingParameters::ECompositingType::eMax:
		pCompositing = std::make_shared<CCompositingMax>(param);
		break;
	case CCompositingParameters::ECompositingType::eInvalid:
	default:
		ASSERT(false);
		return nullptr;
	}


	pCompositing->SetParameters(Parameters);

	return pCompositing;
}
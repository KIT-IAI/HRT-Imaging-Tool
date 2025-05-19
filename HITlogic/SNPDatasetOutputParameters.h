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
#include "HrtImageParameters.h"
class CSNPDatasetOutputParameters
{
	/***********************************************
	************ VERY IMPORTANT NOTICE *************
	************************************************

	************************************************
	*If you add a parameter to this file make sure:*
	**********1. It is an OUTPUT parameter**********
	********2. It is not an INPUT parameter*********
	************3. You added a line in**************
	*CSNPDatasetParameterLoader::ConvertToTableRows*
	************************************************/
public:
	static const std::wstring TableName;

	CSNPDatasetOutputParameters();
	static CSNPDatasetOutputParameters ConstructWithCurrentUserdata();


	size_t nImageCount = 0;
	CHrtImageParameters ImageParameters;
	size_t nDetectedImageGroups = 0;
	double fDetectedThreshold = 0;


	bool operator==(const CSNPDatasetOutputParameters& rhs) const;
	bool operator!=(const CSNPDatasetOutputParameters& rhs) const;

	std::wstring sDate;
	std::wstring sComputer;
	std::wstring sUser;
};


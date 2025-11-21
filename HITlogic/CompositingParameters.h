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

#include "ProcessType.h"



class CCompositingParameters
{

public:

	enum class ECompositingType
	{
		eFusion,
		e3DCloud,
		eVoxelBuffer,
		eTiling,
		eMax,
		eInvalid
	};

	enum class EWeightMode
	{
		eNone,
		eCos2,
		eDistance,
		eInvalid
	};

public:

	CProcessType::EProcessType	eRegistrationProcessType = CProcessType::eHRTStreamRegistration;
	ECompositingType	eCompositingMode = ECompositingType::eFusion;
	EWeightMode			eWeightMode = EWeightMode::eDistance;	/**	\brief Die Gewichtung der Quellbilder beim Erzeugen des Mosaikbilds.*/
	size_t				nBorder = 10;							/**	\brief Die Größe der Umrandung des Mosaikbilds.*/
	double				cBackgroundColor = 0;					/**	\brief Die Hintergrundfarbe des Mosaikbilds.*/
	size_t				nSubImageHeight = 0;
	long				nMaximumGridLength = 0;					/** Only Used for Combining-mode*/
	long				nEdgeBlendingLength = 0;				/** Only Used for Combining-mode*/
	float				fIntensityLimit = 0;					/**	Für 3D-Compositer: Wenn Punkte herausgefiltert werden sollen.*/
	float				fZMultiplier = 2;						/**	Für 3D-Compositer: Legt fest ob Punktwolke in x_achse gestreckt werden soll.*/
	bool				bExcludeNonSNPImages = false;

};

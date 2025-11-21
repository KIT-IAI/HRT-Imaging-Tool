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
#include "StrategyPlanes.h"



CStrategyPlanes::CStrategyPlanes()
{
}


CStrategyPlanes::~CStrategyPlanes()
{
}

size_t CStrategyPlanes::InitialWorkloadEstimation() const
{
	// This number is veeeeeeeeeeery arbitrary.
	return m_Parameters.nPlaneCount * m_nImageCount * int((m_Parameters.nPlaneCount * m_Parameters.nPlaneIncrement) / m_Parameters.fSelectionHeight);
}

void CStrategyPlanes::InitializeAdditionalParameters(CImageRegistrationData addData)
{
	m_Data = addData;
	if (m_Data.ImageInfo.getCount() < 1) {
		CLog::Log(CLog::eWarning, L"CStrategyPlanes: InitializeAdditionalParameters", L"Keine Tiefendaten gefunden, liefere gleiche Ergebnisse wie Komplettstrategie");
	}
}

bool CStrategyPlanes::PrepareNextPair()
{
	size_t mainImage = m_aNextPair[0];
	size_t imageRunner = m_aNextPair[1];

	bool checkOnce = true;

	while
		( //weiterlaufen solange keine Gültige Kombination gefunden, mindestens einmaliger Aufruf
			(checkOnce) ||
			(
				(imageRunner > (mainImage + m_Parameters.nWindowSize)) &&
				(((m_Data.ImageInfo.getImageDepthAt(static_cast<int>(mainImage)) - m_Parameters.fSelectionHeight) > m_Data.ImageInfo.getImageDepthAt(static_cast<int>(imageRunner))) ||
					((m_Data.ImageInfo.getImageDepthAt(static_cast<int>(mainImage)) + m_Parameters.fSelectionHeight) < m_Data.ImageInfo.getImageDepthAt(static_cast<int>(imageRunner))))
				)
			)
	{
		imageRunner++;
		checkOnce = false;

		if (imageRunner >= m_nImageCount) {
			mainImage++;
			imageRunner = mainImage + 1;
		}

		if ((mainImage + 1) >= m_nImageCount) {
			m_bFinished = true;
			return false;
		}
	}

	m_aNextPair[0] = mainImage;
	m_aNextPair[1] = imageRunner;

	return true;
}

void CStrategyPlanes::SetParameters(const CRegistrationStrategyParameters& parameters)
{
	this->m_Parameters = parameters;
}

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
#include "HRTImageDepthAndAngleData.h"



class CHRTImageDepthAndAngleCalculator
{
public:
	CHRTImageDepthAndAngleCalculator();
	~CHRTImageDepthAndAngleCalculator();

	bool giveBasicInfo(int ImageCount, int PlaneCount, int PlaneIncrement = 1);

	//filePath without extension (justSeriesName) Example: E:\EGAFT1_001-S\EGAFT1_001-S_S01_01 (Path like Vol-File, but without Vol)
	//Offset: if useOffset = false: lookup / calculate Offset from file
	bool givePiezoData(int ImageCount, wstring filePathAndSeriesName, wstring snpFolder, bool useOffsetVar = false, double Offset = 0.0);

	bool giveCombined(int ImageCount, int PlaneCount, wstring filePathAndSeriesName, wstring snpFolder, int PlaneIncrement = 1, bool useOffsetVar = false, double Offset = 0.0);

	bool isResultAvailable();
	CHRTImageDepthAndAngleData getResults();

private:
	CHRTImageDepthAndAngleData m_ImageData;

};


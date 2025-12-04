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
#include "Compare.h"



CCompare::CCompare()
{

}

CCompare::~CCompare()
{
}

bool CCompare::CompareDoubles(double nVal1, double nVal2, double nEpsilon)
{
	bool bRet = (((nVal2 - nEpsilon) <= nVal1) && (nVal1 <= (nVal2 + nEpsilon)));
	return bRet;
}

/*
*  Checks whether nVal1 and nVal2 differ only by 10^-nPrecision
*/
bool CCompare::CompareDoubles(double nVal1, double nVal2, int nPrecision)
{
	nPrecision = std::max(std::min(15, nPrecision), 0); //double hat maximale Genauigkeit von 15 Nachkommastellen
	double nEpsilon = 1.0;
	for (int i = 1; i <= nPrecision; i++)
	{
		nEpsilon *= 0.1;
	}
	bool bRet = (((nVal2 - nEpsilon) <= nVal1) && (nVal1 <= (nVal2 + nEpsilon)));
	return bRet;
}

bool CCompare::CompareFloat(float nVal1, float nVal2, float nEpsilon)
{
	bool bRet = (((nVal2 - nEpsilon) <= nVal1) && (nVal1 <= (nVal2 + nEpsilon)));
	return bRet;
}

bool CCompare::CompareFloat(float nVal1, float nVal2, int nPrecision)
{
	nPrecision = std::max(std::min(6, nPrecision), 0);//float hat maximale Genauigkeit von 6 Nachkommastellen
	double nEpsilon = 1.0;
	for (int i = 1; i <= nPrecision; i++)
	{
		nEpsilon *= 0.1;
	}
	bool bRet = (((nVal2 - nEpsilon) <= nVal1) && (nVal1 <= (nVal2 + nEpsilon)));
	return bRet;
}

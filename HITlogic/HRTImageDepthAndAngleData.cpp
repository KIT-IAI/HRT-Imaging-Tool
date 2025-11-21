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
#include "HRTImageDepthAndAngleData.h"



CHRTImageDepthAndAngleData::CHRTImageDepthAndAngleData()
{
}


CHRTImageDepthAndAngleData::~CHRTImageDepthAndAngleData()
{
}

double CHRTImageDepthAndAngleData::getImageDepthAt(int i) const
{
	if (i >= m_iValueCount) {
		return 0.0;
	}
	return m_ImageDepths.at(i);
}

/*double CHRTImageDepthAndAngleData::getImageAngleAt(int i)
{
	if (i >= m_iValueCount) {
		return 0.0;
	}
	return m_ImageAngles.at(i);
}*/

double CHRTImageDepthAndAngleData::getImageInclination(int i) const
{
	double incl = 0.0;
	if (hasInclination())
	{
		if (i == 0)
			incl = (getImageDepthAt(i + 1) - getImageDepthAt(i)) / (384 * (4 / 3));	//Bildzeilen + theoretische Zeilen in Pause
		else if (i == (getCount() + 1))
			incl = (getImageDepthAt(i) - getImageDepthAt(i - 1)) / (384 * (4 / 3));
		else
			incl = (getImageDepthAt(i + 1) - getImageDepthAt(i - 1)) / (384 * (4 / 3) * 2);	//gleicht rauschen in den Daten aus
	}
	return incl;
}



void CHRTImageDepthAndAngleData::setCombined(int i, double depth)//, double angle)
{
	setImageDepth(i, depth);
	//setImageAngle(i, angle);
}

void CHRTImageDepthAndAngleData::setImageDepth(int i, double depth)
{
	if (i >= m_ImageDepths.size()) {
		m_ImageDepths.resize(i + 1);
		m_iValueCount = i + 1;
		/*if (!isSimple()) {
			m_ImageAngles.resize(i+1);
		}*/
	}
	m_ImageDepths.at(i) = depth;
}

/*void CHRTImageDepthAndAngleData::setImageAngle(int i, double angle)
{
	if (i >= m_ImageAngles.size()) {
		if (i < m_iValueCount && m_ImageAngles.size() < m_iValueCount) {
			m_ImageAngles.resize(m_iValueCount);
		}
		else {
			m_ImageAngles.resize(i + 1);
			m_ImageDepths.resize(i + 1);
			m_iValueCount = i + 1;
		}
	}
	m_ImageDepths.at(i) = angle;
}
*/


void CHRTImageDepthAndAngleData::setCount(int i)
{
	if (i < m_iValueCount) {
		CLog::Log(CLog::eWarning, L"CHRTImageDepthAndAngleData: setCount", L"Input lower than previous count. Deleting elements!");
	}
	m_ImageDepths.resize(i);
	/*if (!isSimple()) {
		m_ImageAngles.resize(i);
	}*/
	m_iValueCount = i;
}

int CHRTImageDepthAndAngleData::getCount() const
{
	return m_iValueCount;
}

bool CHRTImageDepthAndAngleData::isAvailable() const
{
	if (m_ImageDepths.size() > 0) {
		return true;
	}
	return false;
}

bool CHRTImageDepthAndAngleData::hasInclination() const
{
	return m_hasInclination;
}

void CHRTImageDepthAndAngleData::setInclination(bool val)
{
	m_hasInclination = val;
}


/*bool CHRTImageDepthAndAngleData::isSimple()
{
/*	if (m_ImageAngles.size() > 0) {
		return false;
	}
	return true;
}*/

CHRTImageDepthAndAngleData CHRTImageDepthAndAngleData::filter(std::list<size_t>filter) const
{
	CHRTImageDepthAndAngleData ret;
	if (isAvailable()) {
		int i = 0;
		for (auto el : filter) {
			ret.setImageDepth(i, getImageDepthAt(el));
			ret.m_hasInclination = m_hasInclination;
			/*if (!isSimple()) {
				ret.setImageAngle(i, getImageAngleAt(el));
			}*/
			i++;

		}
	}
	return ret;
}

void CHRTImageDepthAndAngleData::setPiezoFilePath(wstring path)
{
	m_filePath = path;
}

void CHRTImageDepthAndAngleData::setSnpPath(wstring path)
{
	m_snpPath = path;
}

wstring CHRTImageDepthAndAngleData::getPiezoFilePath() const
{
	return m_filePath;
}

wstring CHRTImageDepthAndAngleData::getPiezoSnpPath() const
{
	return m_snpPath;
}

double CHRTImageDepthAndAngleData::getMaxDepth() const
{
	if (m_ImageDepths.size() > 0)
		return *std::max_element(m_ImageDepths.begin(), m_ImageDepths.end());
	return 0.0;
}

double CHRTImageDepthAndAngleData::getMinDepth() const
{
	if (m_ImageDepths.size() > 0)
		return *std::min_element(m_ImageDepths.begin(), m_ImageDepths.end());
	return 0.0;
}

double CHRTImageDepthAndAngleData::getMinInclination() const
{
	double minIncliation = 0.0;
	if (m_ImageDepths.size() > 0) {
		for (int i = 0; i < m_ImageDepths.size(); i++)
		{
			auto tmp = getImageInclination(i);

			if (tmp < minIncliation)
				minIncliation = tmp;
		}
	}
	return minIncliation;
}

//substracts given Value from all Depths
void CHRTImageDepthAndAngleData::depthMinus(double num)
{
	if (m_ImageDepths.size() > 0)
		std::for_each(m_ImageDepths.begin(), m_ImageDepths.end(), [&](double& d) { d -= num; });
}


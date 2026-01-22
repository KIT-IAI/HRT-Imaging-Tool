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
#include "DenseMatrix.h"

/*
This simple DPoint-Class (Careful not CPoint) war created because the MFC CPoint-Class doesn't have a damn "distanceFrom"-method. (+ I didn't want to use MFC)
Feel free to add as much functionallity as you please
*/
class DPoint
{
public:
	explicit DPoint() :DPoint(0.0, 0.0) {};
	explicit DPoint(double x, double y) :m_x(x), m_y(y) {};

	double distanceFrom(const DPoint& otherPoint) const;
	void moveTo(const DPoint& otherPoint);

	double m_x;
	double m_y;

	bool operator==(const DPoint& other) const;
};


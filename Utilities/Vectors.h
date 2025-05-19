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

template <class T> class CVector2d
{

public:

	// the vector's coordinates
	T x;
	T y;

	// constructors:
	CVector2d() : x(0), y(0) {}	// default constructor
	CVector2d(T x, T y) : x(x), y(y) {}	// constructor
	CVector2d(const CVector2d<T>& arg) : x(arg.x), y(arg.y) {}	// copy constructor
	template <typename U>
	explicit CVector2d<T>(const CVector2d<U>& arg) : x(static_cast<T>(arg.x)), y(static_cast<T>(arg.y)) {}	// copy constructor

	// arithmetic operators
	CVector2d<T> operator+ (const CVector2d<T>& arg) const { return CVector2d<T>(x + arg.x, y + arg.y); }
	CVector2d<T> operator- (const CVector2d<T>& arg) const { return CVector2d<T>(x - arg.x, y - arg.y); }

	CVector2d<T> operator* (T arg) const { return CVector2d<T>(x * arg, y * arg); }
	CVector2d<T> operator/ (T arg) const { return CVector2d<T>(x / arg, y / arg); }

	// assignment operators
	void operator+= (const CVector2d<T>& arg) { x += arg.x; y += arg.y; }
	void operator-= (const CVector2d<T>& arg) { x -= arg.x; y -= arg.y; }

	void operator*= (T arg) { x *= arg; y *= arg; }
	void operator/= (T arg) { x /= arg; y /= arg; }

	// relational operators
	bool operator== (const CVector2d<T>& arg) const { return ((x == arg.x) && (y == arg.y)); }
	bool operator!= (const CVector2d<T>& arg) const { return !(*this == arg); }

	/** \brief brechnet das Skalarprodukt eines Vektors */
	T Scalar() { return static_cast<T>(sqrt(pow(x, 2) + pow(y, 2))); };

	template <typename U> CVector2d<U> ToType() { return CVector2d<U>(static_cast<U>(x), static_cast<U>(y)); };
};



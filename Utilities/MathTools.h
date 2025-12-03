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


#define _USE_MATH_DEFINES

#include "DenseMatrix.h"
#include "Vectors.h"
#include <vector>
#include <math.h>

/* Definitions of useful mathematical constants (enthalten in math.h;
 *  nur nutzbar, wenn _USE_MATH_DEFINES definiert ist, bevor math.h includiert wird.)
 * M_E        - e
 * M_LOG2E    - log2(e)
 * M_LOG10E   - log10(e)
 * M_LN2      - ln(2)
 * M_LN10     - ln(10)
 * M_PI       - pi
 * M_PI_2     - pi/2
 * M_PI_4     - pi/4
 * M_1_PI     - 1/pi
 * M_2_PI     - 2/pi
 * M_2_SQRTPI - 2/sqrt(pi)
 * M_SQRT2    - sqrt(2)
 * M_SQRT1_2  - 1/sqrt(2)
 */

#define PI				M_PI   //3.1415926535
#define PI_DIV_2		M_PI_2 //(PI/2)
#define DEG_TO_RAD		(PI/180.0)
#define RAD_TO_DEG		(180.0/PI)

// inline double pow(__in int _X, __in int _Y)
// {
// 	return pow((double)_X, (double)_Y);
// }

// inline double  sqrt(__in int _X)
// {
// 	return sqrt((double)_X);
// }

// inline double  log(__in int _X)
// {
// 	return log((double)_X);
// }

// inline double  atan2(__in int _Y, __in int _X)
// {
// 	return atan2((double)_Y, (double)_X);
// }

// inline double  fabs(__in int _X)
// {
// 	return fabs((double)_X);
// }

class  CMathTools
{

public:

	CMathTools() = delete;
	virtual ~CMathTools() = delete;

public:

	static int GetRandomNumber(int nLowerBound, int nUpperBound);
	static double Round(double fValue, int nPrecision);
	static double StdDev(std::vector<double>* pArray);
	static float StdDev(std::vector<float>* pArray);
	static double Mean(std::vector<double>* pArray);
	static float Mean(std::vector<float>* pArray);
	static double SolveQuadraticEquation(double nX, double nA, double nB, double nC);

	static long MVRegr2n(CDenseMatrix& M, CDenseMatrix& V, int n, int nV, CDenseMatrix& R);
	static double CalculateDistance(const CVector2d<double>& vPt1, const CVector2d<double>& vPt2);
	static double CalculateAngle(const CVector2d<double>& vPt1, const CVector2d<double>& vPt2);

	static int RoundValue(float fValue);
	static int RoundValue(double fValue);
	static int RoundTowardsZero(float fValue);
	static int RoundTowardsZero(double fValue);

	static double Signum(double x);

	static double Modulo(double value, double divisor);
	static double Modulo(double value, int divisor);
	static float Modulo(float value, float divisor);
	static float Modulo(float value, int divisor);

	static double arsinh(double x);
	static double Square(double x);

	static int                NextPowOf2(int                nValue);
	static long               NextPowOf2(long               nValue);
	static long long          NextPowOf2(long long          nValue);
	static unsigned int       NextPowOf2(unsigned int       nValue);
	static unsigned long      NextPowOf2(unsigned long      nValue);
	static unsigned long long NextPowOf2(unsigned long long nValue);

	static bool IsPowerOf2(int                nValue);
	static bool IsPowerOf2(long               nValue);
	static bool IsPowerOf2(long long          nValue);
	static bool IsPowerOf2(unsigned int       nValue);
	static bool IsPowerOf2(unsigned long      nValue);
	static bool IsPowerOf2(unsigned long long nValue);

};

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
#include "MathTools.h"

using namespace std;



double CMathTools::StdDev(vector<double>* pArray)
{
	assert(pArray->size() > 1);

	double	sum = 0;
	double	mean = Mean(pArray);
	size_t	n = pArray->size();

	for (size_t i = 0; i < n; i++)
	{
		double	tmp = pArray->at(i) - mean;
		sum += tmp * tmp;
	}

	return sqrt(sum / static_cast<double>(n - 1));
}

float CMathTools::StdDev(vector<float>* pArray)
{
	assert(pArray->size() > 1);

	float	sum = 0;
	float	mean = Mean(pArray);
	size_t	n = pArray->size();

	for (size_t i = 0; i < n; i++)
	{
		auto tmp = pArray->at(i) - mean;
		sum += tmp * tmp;
	}

	return static_cast<float>(sqrt(sum / static_cast<float>(n - 1)));
}

double CMathTools::Mean(vector<double>* pArray)
{
	assert(pArray->size() > 0);

	double	mean = 0;
	size_t	n = pArray->size();

	for (size_t i = 0; i < n; i++)
	{
		mean += pArray->at(i);
	}

	return (mean / static_cast<double>(n));
}

float CMathTools::Mean(vector<float>* pArray)
{
	assert(pArray->size() > 0);

	float	mean = 0;
	size_t	n = pArray->size();

	for (size_t i = 0; i < n; i++)
	{
		mean += pArray->at(i);
	}

	return (mean / static_cast<float>(n));
}

/**	\brief Liefert die Polynomkoeffizienten für nV abhängige Vektoren, die in
 *	der Matrix V als Spaltenvektoren zusamengefasst sind.
 *
 *	\return 0, wenn die Berechnungen erfolgreich durchgeführt wurden.
 *	\return Ein negativer Wert, wenn ein Fehler aufgetreten ist.
 *
 *	\param[in] M Ein Feld, bei dem jede der (genau 2) Spalten mit einer der
 *		unabhängigen Variablen korrespondierende Daten repräsentiert.
 *	\param[in] V Zwei reele Vektoren, die für die Daten der abhängigen
 *		Variablen stehen.
 *	\param[in] n Grad der Polynomfunktion.
 *	\param[in] nV Anzahl der Zeilen von V.
 *	\param[out] R Ergebnismatrix.
 */
long CMathTools::MVRegr2n(CDenseMatrix& M, CDenseMatrix& V, int n, int nV, CDenseMatrix& R)
{
	int ms = -n - 2;
	int nVsp = 2; // ???
	int m = 0;
	int mL = 0;

	// ml ermitteln zum Allokieren von X ???
	for (int k = 0; k <= n; k++)
	{
		ms = ms + (n - k + 2);

		for (int j = 0; j <= (n - k); j++)
		{
			mL = j + ms;
		}
	}

	mL = mL + 1;

	CDenseMatrix X(nV, mL);
	X.Fill(0);

	// Werte zurücksetzen
	ms = -n - 2;

	for (int k = 0; k <= n; k++)
	{
		ms = ms + (n - k + 2);

		for (int j = 0; j <= (n - k); j++)
		{
			m = j + ms;

			for (int jv = 0; jv < nV; jv++)
			{
				X[jv][m] = (pow(M[jv][0], (double)j) * pow(M[jv][1], (double)k));
			}
		}
	}

	// Prüfen, ob soweit ok
	CDenseMatrix B(mL, nVsp);
	B.Fill(0);

	// Matrix S
	CDenseMatrix S(mL, mL);
	S.Fill(0);
	CDenseMatrix SI(mL, mL);
	SI.Fill(0);

	double lklVal = 0;

	for (int k = 0; k < mL; k++)
	{
		for (int j = 0; j < nVsp; j++)
		{
			for (int jv = 0; jv < nV; jv++)
			{
				lklVal = X[jv][k] * V[jv][j];
				lklVal = lklVal + B[k][j];
				B[k][j] = lklVal;
			}
		}

		for (int j = k; j < mL; j++)
		{
			for (int jv = 0; jv < nV; jv++)
			{
				lklVal = X[jv][k] * X[jv][j];
				lklVal = S[k][j] + lklVal;
				S[k][j] = lklVal;
			}

			S[j][k] = S[k][j];
		}

	}

	SI = S.Inverse();
	if (SI.IsNull())
	{
		return -1;
	}

	R = SI * B;

	return 0;
}

int CMathTools::RoundValue(float fValue)
{
	int retVal = 0;

	if (fValue < 0)
	{
		retVal = (int)(fValue - 0.5);
	}

	if (fValue > 0)
	{
		retVal = (int)(fValue + 0.5);
	}
	return retVal;
}

int CMathTools::RoundValue(double fValue)
{
	int retVal = 0;

	if (fValue < 0)
	{
		retVal = (int)(fValue - 0.5);
	}

	if (fValue > 0)
	{
		retVal = (int)(fValue + 0.5);
	}
	return retVal;
}

int CMathTools::RoundTowardsZero(float fValue)
{
	if (fValue > 0)
		return static_cast<int>(floor(fValue));
	return static_cast<int>(ceil(fValue));
}

int CMathTools::RoundTowardsZero(double fValue)
{
	if (fValue > 0)
		return static_cast<int>(floor(fValue));
	return static_cast<int>(ceil(fValue));
}

double CMathTools::Round(double fValue, int nPrecision)
{
	int nFaktor = 1;
	for (int i = 0; i < nPrecision; i++)
	{
		nFaktor = nFaktor * 10;
	}
	fValue = fValue * nFaktor;
	fValue += 0.5;
	fValue = floor(fValue);
	fValue /= nFaktor;
	return fValue;
}

int CMathTools::GetRandomNumber(int nLowerBound, int nUpperBound)
{
	srand((unsigned)time(NULL));
	return nLowerBound + rand() % (nUpperBound - nLowerBound + 1);
}

/** \brief liefert das Ergebnis einer quadritischen Gleichung.
 */
double CMathTools::SolveQuadraticEquation(double nX, double nA, double nB, double nC)
{
	return	(nA +
		(nB * (double)(nX)) +
		(nC * (double)pow(nX, 2.0))
		);
}

double CMathTools::Modulo(double value, double divisor)
{
	assert(divisor != 0);

	double temp = floor(value / divisor);
	value -= (temp * divisor);
	return value;
}

double CMathTools::Modulo(double value, int divisor)
{
	assert(divisor != 0);

	double temp = floor(value / divisor);
	value -= (temp * divisor);
	return value;
}

float CMathTools::Modulo(float value, float divisor)
{
	assert(divisor != 0);

	float temp = static_cast<float>(floor(value / divisor));
	value -= (temp * divisor);
	return value;
}

float CMathTools::Modulo(float value, int divisor)
{
	assert(divisor != 0);

	float temp = static_cast<float>(floor(value / divisor));
	value -= (temp * divisor);
	return value;
}

double CMathTools::Signum(double x)
{
	if (x < 0)
	{
		return -1;
	}

	if (x == 0)
	{
		return 0;
	}

	if (x > 0)
	{
		return 1;
	}

	return 0;
}

double CMathTools::arsinh(double x)
{
	return log(x + sqrt((x * x) + 1));
}

double CMathTools::Square(double x)
{
	return x * x;
}

/**	\brief Berechnet die kleinste Potenz von 2, die größer oder gleich
 *	\a nValue ist.
 *
 *	\return Die kleinste Potenz von 2, die größer oder gleich \a nValue ist.
 *
 *	\param[in] nValue Eine Zahl.
 */
int CMathTools::NextPowOf2(int nValue)
{
	int i = 0;
	for (; pow(2, i) < nValue; i++);
	return static_cast<int>(pow(2, i));
}

/**	\brief Berechnet die kleinste Potenz von 2, die größer oder gleich
 *	\a nValue ist.
 *
 *	\return Die kleinste Potenz von 2, die größer oder gleich \a nValue ist.
 *
 *	\param[in] nValue Eine Zahl.
 */
size_t CMathTools::NextPowOf2(size_t nValue)
{
	int i = 0;
	for (; pow(2, i) < nValue; i++);
	return static_cast<int>(pow(2, i));
}

/**	\brief Berechnet die kleinste Potenz von 2, die größer oder gleich
 *	\a nValue ist.
 *
 *	\return Die kleinste Potenz von 2, die größer oder gleich \a nValue ist.
 *
 *	\param[in] nValue Eine Zahl.
 */
long CMathTools::NextPowOf2(long nValue)
{
	long i = 0;
	for (; pow(2, i) < nValue; i++);
	return static_cast<long>(pow(2, i));
}

/**	\brief Berechnet die kleinste Potenz von 2, die größer oder gleich
 *	\a nValue ist.
 *
 *	\return Die kleinste Potenz von 2, die größer oder gleich \a nValue ist.
 *
 *	\param[in] nValue Eine Zahl.
 */
__int64 CMathTools::NextPowOf2(__int64 nValue)
{
	__int64 i = 0;
	for (; pow(2, static_cast<int>(i)) < nValue; i++);
	return static_cast<__int64>(pow(2, static_cast<int>(i)));
}

/**	\brief Berechnet die kleinste Potenz von 2, die größer oder gleich
 *	\a nValue ist.
 *
 *	\return Die kleinste Potenz von 2, die größer oder gleich \a nValue ist.
 *
 *	\param[in] nValue Eine Zahl.
 */
unsigned int CMathTools::NextPowOf2(unsigned int nValue)
{
	unsigned int i = 0;
	for (; pow(2, i) < nValue; i++);
	return static_cast<unsigned int>(pow(2, i));
}

bool CMathTools::IsPowerOf2(int nValue)
{
	return !(nValue == 0) && !(nValue & (nValue - 1));
}

bool CMathTools::IsPowerOf2(long nValue)
{
	return !(nValue == 0) && !(nValue & (nValue - 1));
}

bool CMathTools::IsPowerOf2(__int64 nValue)
{
	return !(nValue == 0) && !(nValue & (nValue - 1));
}

bool CMathTools::IsPowerOf2(unsigned int nValue)
{
	return !(nValue == 0) && !(nValue & (nValue - 1));
}

bool CMathTools::IsPowerOf2(unsigned long nValue)
{
	return !(nValue == 0) && !(nValue & (nValue - 1));
}

bool CMathTools::IsPowerOf2(size_t nValue)
{
	return !(nValue == 0) && !(nValue & (nValue - 1));
}

/**	\brief Berechnet den Abstand zwischen zwei Punkten.
 *
 *	Dabei spielt es keine Rolle, in welcher Einheit die Punktkoordinaten
 *	angegeben werden.
 *
 *	\return Den Abstand der beiden Punkte.
 *
 *	\param[in] vPt1 Der erste Punkt.
 *	\param[in] vPt2 Der zweite Punkt.
 *
 *	\author Matthias Uchdorf
 */
double CMathTools::CalculateDistance(const CVector2d<double>& vPt1, const CVector2d<double>& vPt2)
{
	double dDistX, dDistY, dDist;
	dDistX = vPt2.x - vPt1.x;
	dDistY = vPt2.y - vPt1.y;

	dDist = sqrt(pow(dDistX, 2.0) + pow(dDistY, 2.0));
	return dDist;
}

/**	\brief Berechnet den Winkel, den die durch zwei Punkte gegebene Linie mit
 *	der Horizontalen einschließt.
 *
 *	Dabei spielt es keine Rolle, in welcher Einheit die Punktkoordinaten
 *	angegeben werden. Jedoch ist die Reihenfolge der beiden Punkte wichtig.
 *
 *	\return Den Winkel zwischen der durch die beiden Punkte gegebenen Linie
 *		und der Horizontalen in Grad.
 *
 *	\param[in] vPt1 Der erste Punkt.
 *	\param[in] vPt2 Der zweite Punkt.
 *
 *	\author Matthias Uchdorf
 */
double CMathTools::CalculateAngle(const CVector2d<double>& vPt1, const CVector2d<double>& vPt2)
{
	//
	//	   (Punkt1)X_____________________________ 
	//				\   \           (Horizontale)
	//				 \	 ) <- Winkel (hier ein negativer Wert)
	//		  		  \	/
	//	   (Verbindung)\	
	//				    \	
	//				     \	
	//					  X (Punkt2) 
	//
	//
	double dDistX, dDistY, dAngle;
	dDistX = vPt2.x - vPt1.x;
	dDistY = vPt2.y - vPt1.y;
	dAngle = atan2(dDistY, dDistX);
	dAngle = -dAngle / PI * 180.0f;
	return dAngle;
}

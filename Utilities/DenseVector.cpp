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
#include "DenseVector.h"

#include "MathTools.h"



CDenseVector::CDenseVector()
	:CDenseVector(0)
{
}

CDenseVector::CDenseVector(size_t len)
{
	m_nVectorType = eDense;

	m_nSize = len;
	if (len == 0)
		vec = nullptr;
	else
		vec = new double[len];
}

CDenseVector::CDenseVector(const CDenseVector& v)
{
	m_nVectorType = eDense;

	m_nSize = 0;
	vec = nullptr;

	Copy(v);
}

CDenseVector::CDenseVector(const std::vector<double>& v)
	:CDenseVector(v.size())
{
	for (size_t i = 0; i < v.size(); i++)
		vec[i] = v[i];
}
CDenseVector::CDenseVector(const std::vector<int>& v)
	: CDenseVector(v.size())
{
	for (size_t i = 0; i < v.size(); i++)
		vec[i] = static_cast<double>(v[i]);
}

CDenseVector::~CDenseVector()
{
	FreeVector();
}

void CDenseVector::AllocVector(size_t newSize)
{
	assert(!m_bIsChild);
	FreeVector();
	vec = new double[newSize];
	m_nSize = newSize;
}
void CDenseVector::FreeVector()
{
	if (vec != nullptr && !m_bIsChild)
	{
		delete[] vec;
		vec = nullptr;
		m_nSize = 0;
	}
}

void CDenseVector::Copy(const CAbstractVector& av)
{
	if (av.GetVectorType() == eDense)
		return Copy(static_cast<const CDenseVector&>(av));
	else
		return CAbstractVector::Copy(av);
}

void CDenseVector::Copy(const CDenseVector& av)
{
	FreeVector();
	AllocVector(av.Size());
#ifdef _WIN32
	memcpy_s(vec, Size() * sizeof(double), av.vec, av.Size() * sizeof(double));
#else
	std::memcpy(vec, av.vec, av.Size() * sizeof(double));
#endif
}

void CDenseVector::Copy(const std::vector<double>& av)
{
	FreeVector();
	AllocVector(av.size());
#ifdef _WIN32
	memcpy_s(vec, Size() * sizeof(double), av.data(), av.size() * sizeof(double));
#else
	std::memcpy(vec, av.data(), av.size() * sizeof(double));
#endif
}

void CDenseVector::SetValueAt(size_t index, double value)
{
	vec[index] = value;
}
const double CDenseVector::GetValueAt(size_t index) const
{
	assert(index >= 0 && index < Size());
	return vec[index];
}
std::vector<double> CDenseVector::GetValuesAsArray() const
{
	std::vector<double> values(Size());
#ifdef _Win32
	memcpy_s(values.data(), Size() * sizeof(double), vec, Size() * sizeof(double));
#else
	std::memcpy(values.data(), vec, Size() * sizeof(double));
#endif
	return values;
}


double& CDenseVector::operator[](size_t ndx)
{
	assert((ndx >= 0) && (ndx < Size()));
	return vec[ndx];
}

const double& CDenseVector::operator[](size_t ndx) const
{
	assert((ndx >= 0) && (ndx < Size()));
	return vec[ndx];
}

CDenseVector& CDenseVector::operator=(const CDenseVector& v)
{
	Copy(v);
	return *this;
}


CDenseVector CDenseVector::operator+(const CAbstractVector& v) const
{
	assert(v.Size() == Size());

	CDenseVector vv(Size());
	for (size_t i = 0; i < Size(); i++)
	{
		vv.vec[i] = this->vec[i] + v.GetValueAt(i);
	}
	return vv;
}
CDenseVector CDenseVector::operator-(const CAbstractVector& v) const
{
	assert(v.Size() == Size());
	CDenseVector vv(v.Size());
	for (size_t i = 0; i < v.Size(); i++)
	{
		vv.vec[i] = this->vec[i] - v.GetValueAt(i);
	}
	return vv;
}
double CDenseVector::operator*(const CAbstractVector& v) const
{
	assert(v.Size() == Size());
	double sum = 0;
	for (size_t i = 0; i < v.Size(); i++)
	{
		sum += this->vec[i] * v.GetValueAt(i);
	}
	return sum;
}
CDenseVector CDenseVector::operator+(double s) const
{
	CDenseVector vv(Size());
	for (size_t i = 0; i < Size(); i++)
	{
		vv.vec[i] = vec[i] + s;
	}
	return vv;
}
CDenseVector CDenseVector::operator-(double s) const
{
	CDenseVector vv(Size());
	for (size_t i = 0; i < Size(); i++)
	{
		vv.vec[i] = vec[i] - s;
	}
	return vv;
}
CDenseVector CDenseVector::operator*(double s) const
{
	CDenseVector vv(Size());
	for (size_t i = 0; i < Size(); i++)
	{
		vv.vec[i] = vec[i] * s;
	}
	return vv;
}
CDenseVector CDenseVector::operator/(double s) const
{
	CDenseVector vv(Size());
	for (size_t i = 0; i < Size(); i++)
	{
		vv.vec[i] = vec[i] / s;
	}
	return vv;
}

CSparseVector CDenseVector::ToSparseVector() const
{
	CSparseVector sv;
	sv.Copy(*this);
	return sv;
}

void CDenseVector::Swap(CDenseVector& v)
{
	assert(v.Size() == Size());

	double* rp;
	rp = vec;
	vec = v.vec;
	v.vec = rp;
}
double CDenseVector::Norm2() const
{
	return sqrt((*this) * (*this));
}

/**	\brief Interpoliert den Vektor.
 *
 *	\param[in] k Die Anzahl der aus jedem Element des Vektors zu
 *		interpolierenden Werte. Es wird empfohlen, ungerade Werte zu
 *		verwenden.
 *	\param[out] vecDest Der Ergebnis-Vektor.
 */
void CDenseVector::QuadraticInterpolation(size_t k, CDenseVector& vecDest) const
{
	assert(this->Size() > 2);
	assert(k > 0);

	if (vecDest.Size() != (k * this->Size()))
	{
		vecDest.FreeVector();
		vecDest = CDenseVector(k * this->Size());
	}

	double a = 0.0;
	double b = 0.0;
	double c = 0.0;
	double x = 0.0;

	for (size_t nIndex = 1; nIndex <= (this->Size() - 2); nIndex++)
	{
		a = vec[nIndex];
		b = (vec[nIndex + 1] - vec[nIndex - 1]) / 2.0;
		//c = vec[nIndex+1] - 2 * vec[nIndex] + vec[nIndex-1];
		c = (vec[nIndex + 1] - 2 * vec[nIndex] + vec[nIndex - 1]) / 2;


		if ((nIndex > 1) && (nIndex < (this->Size() - 2)))
		{
			for (size_t nVar = 0; nVar < k; nVar++)
			{
				x = (nVar - (k / 2.0)) / k;
				vecDest[nVar + nIndex * k] = CMathTools::SolveQuadraticEquation(x, a, b, c);
			}
		}
		else if (nIndex == 1)
		{
			ptrdiff_t j = static_cast<ptrdiff_t>(k);
			for (ptrdiff_t nVar = -j; nVar < j; nVar++)
			{
				x = (nVar - (k / 2.0)) / k;
				vecDest[nVar + j] = CMathTools::SolveQuadraticEquation(x, a, b, c);
			}
		}
		else if (nIndex == (this->Size() - 2))
		{
			for (size_t nVar = 0; nVar < 2 * k; nVar++)
			{
				x = (nVar - (k / 2.0)) / k;
				vecDest[nVar + nIndex * k] = CMathTools::SolveQuadraticEquation(x, a, b, c);
			}
		}
	}
}

/**	\brief Interpoliert den Vektor.
 *
 *	\param[in] k Die Anzahl der aus jedem Element des Vektors zu
 *		interpolierenden Werte.
 *	\param[out] vecDest Der Ergebnis-Vektor.
 *
 *	\author Stephan Allgeier
 */
void CDenseVector::CubicSplineInterpolation(size_t k, CDenseVector& vecDest) const
{
	assert(k > 0);
	assert(Size() > 2);

	size_t nInSize = Size();
	size_t nOutSize = k * nInSize;

	if (vecDest.Size() != nOutSize)
	{
		vecDest.FreeVector();
		vecDest.AllocVector(nOutSize);
	}

	alglib::real_1d_array outval;
	alglib::real_1d_array outind;
	alglib::real_1d_array inval;
	alglib::real_1d_array inind;

	outval.setlength(nOutSize);
	outind.setlength(nOutSize);
	inval.setlength(nInSize);
	inind.setlength(nInSize);

	for (size_t i = 0; i < nOutSize; i++)
	{
		outind[i] = static_cast<double>(i);
	}

	for (size_t i = 0; i < nInSize; i++)
	{
		inval[i] = vec[i];
		inind[i] = i * k + ((k - 1) / 2.0);
	}

	alglib::spline1dconvcubic(inind, inval, nInSize, 0, 0, 0, 0, outind, nOutSize, outval);

	for (size_t i = 0; i < nOutSize; i++)
	{
		vecDest[i] = outval[i];
	}
}

void CDenseVector::Child1D(CDenseVector& source, size_t startElement, size_t size)
{
	FreeVector();

	m_bIsChild = true;
	vec = source.vec + startElement;
	m_nSize = size;
}

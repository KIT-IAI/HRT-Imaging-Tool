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


class CSparseVector;

#include "AbstractVector.h"
#include "SparseVector.h"

class CDenseVector : public CAbstractVector
{

private:
	double* vec;
	bool m_bIsChild = false;

public:

	CDenseVector();
	explicit CDenseVector(size_t len);
	CDenseVector(const CDenseVector& v);
	explicit CDenseVector(const std::vector<double>& v);
	explicit CDenseVector(const std::vector<int>& v);
	~CDenseVector();

	void AllocVector(size_t size) override;
	void FreeVector() override;
	void Copy(const CAbstractVector&) override;
	void Copy(const CDenseVector&);
	void Copy(const std::vector<double>&);

	void SetValueAt(size_t index, double value) override;
	const double GetValueAt(size_t index) const override;
	std::vector<double> GetValuesAsArray() const;

	double& operator[](size_t ndx);
	const double& operator[](size_t ndx) const;

	CDenseVector& operator	= (const CDenseVector& v);

	CDenseVector operator+(const CAbstractVector& v) const;
	CDenseVector operator-(const CAbstractVector& v) const;
	double operator*(const CAbstractVector& v) const;

	CDenseVector operator+(double s) const;
	CDenseVector operator-(double s) const;
	CDenseVector operator*(double s) const;
	CDenseVector operator/(double s) const;

	CSparseVector ToSparseVector() const;
	void Swap(CDenseVector& v);
	double Norm2() const;

	void QuadraticInterpolation(size_t k, CDenseVector& vecDest) const;
	void CubicSplineInterpolation(size_t k, CDenseVector& vecDest) const;

	void Child1D(CDenseVector& source, size_t startElement, size_t size);
};

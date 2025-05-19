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

#include <list>

class CDenseVector;
#include "DenseVector.h"

class CSparseVector : public CAbstractVector
{
private:
	size_t	m_nNonZeroSize;
	size_t*	m_pColumns;
	double*	m_pValues;

	size_t*	m_pCacheLineColumns;
	double*	m_pCacheLineValues;
	size_t	m_nCacheLineIndex;
	size_t	m_nCacheLength;

	bool	m_bIsChild = false;
	size_t	m_nChildOffset = 0;
	CSparseVector* m_pParent = nullptr;
	std::list<CSparseVector*> m_lChildren;

public:
	CSparseVector();
	CSparseVector(const CSparseVector& v);
	explicit CSparseVector(size_t size);
	CSparseVector(size_t size, size_t nonZeroSize, size_t* columns, double* values);
	~CSparseVector(void);

	void					Init(void);

	void					AllocVector(size_t size);
	void					FreeVector();
	void					Copy(const CAbstractVector&);
	void					Copy(const CSparseVector&);
	void					Copy(const CDenseVector&);

	void					SetValueAt(size_t index, double value);
	const double			GetValueAt(size_t column) const;
	std::vector<double>		GetValuesAsArray() const;

	const double			operator[](size_t ndx) const;

	CSparseVector& operator=(const CSparseVector& v);

	double					operator*(const CAbstractVector& v) const;
	CSparseVector			operator+(const CAbstractVector& v) const;
	CSparseVector			operator-(const CAbstractVector& v) const;

	CSparseVector			operator+(double s) const;
	CSparseVector			operator-(double s) const;
	CSparseVector			operator*(double s) const;
	CSparseVector			operator/(double s) const;

	size_t					GetNonZeroColumnAt(size_t index) const;
	double					GetNonZeroValueAt(size_t index) const;

	size_t					NonZeroSize() const;

	CDenseVector			ToDenseVector() const;

	void					Fill(double s);
	void					Swap(CSparseVector& sv);

	double					Norm2();

	bool					IsFull() const;
	bool					IsDense() const;
	bool					IsCacheFull() const;
	bool					IsFlushed() const;
	bool					IsZero() const;

	void					Flush();
	void					Child1D(CSparseVector& source, size_t startElement, size_t size);
	void					Expand(size_t size);

private:
	void					AllocVector(size_t nSize, size_t nNonZeroSize);

	bool					LinearSearchInCache(size_t inColumn, size_t& outColumn, double& outValue) const;
	bool					BinarySearchInValues(size_t inColumn, size_t& outColumn, double& outValue) const;


	void					InitCache();
	void					SortCache();
	size_t					MoveValueToCache(size_t column);
	size_t					CheckForProlungation() const;
	void					ModifyColumns(size_t enlargement);
	void					RecalculateOptimalCacheLength();

	void					NotifyChildren();
};

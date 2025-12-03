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

class CDenseMatrix;

#include "AbstractMatrix.h"
#include "SparseVector.h"
#include "DenseMatrix.h"

class CSparseMatrix : public CAbstractMatrix
{
private:
	std::vector<CSparseVector> m_pMat;
	bool m_bIsChild = false;

public:

	CSparseMatrix();
	CSparseMatrix(size_t rows, size_t cols);
	CSparseMatrix(const CAbstractVector& v, bool bColumnVector);
	CSparseMatrix(const CSparseMatrix&);
	explicit CSparseMatrix(const CAbstractMatrix&);
	~CSparseMatrix(void);

	void FreeMatrix() override;
	void AllocMatrix(size_t rows, size_t cols) override;

	CSparseMatrix& operator=(const CSparseMatrix& m);
	void Copy(const CAbstractMatrix&) override;
	void Copy(const CDenseMatrix&);
	void Copy(const CSparseMatrix&);

	CSparseMatrix& operator*=(const CAbstractMatrix& m);

	CSparseMatrix operator+(const CAbstractMatrix& m) const;
	CSparseMatrix operator-(const CAbstractMatrix& m) const;
	CSparseMatrix operator*(const CAbstractMatrix& m) const;
	CSparseVector operator*(const CAbstractVector& v) const;
	CDenseVector operator*(const CDenseVector& v) const;

#ifdef _WIN32
	CSparseMatrix ParallelProduct(const CAbstractMatrix& m) const;
	CSparseVector ParallelProduct(const CAbstractVector& m) const;
#endif

	CSparseMatrix operator+(double s) const;
	CSparseMatrix operator-(double s) const;
	CSparseMatrix operator*(double s) const;
	CSparseMatrix operator/(double s) const;

	CSparseVector& GetVectorAt(size_t row);
	const CSparseVector& GetVectorAt(size_t row) const;
	void					SetVectorAt(size_t row, const CAbstractVector& v);
	CSparseVector& operator[](size_t row);
	const CSparseVector& operator[](size_t row) const;

	CSparseVector Row(size_t r) const;
	CSparseVector Col(size_t c) const;
	CDenseVector Diagonal() const;

	CDenseMatrix ToDenseMatrix() const;
	void Flush();
	bool IsFlushed();

	CSparseMatrix Transpose() const;
	void Child2D(CSparseMatrix& source, size_t row, size_t col, size_t rowCount, size_t colCount);
	void Expand(size_t col, size_t row);
};
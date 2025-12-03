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

class CSparseMatrix;

#include "AbstractMatrix.h"
#include "SparseMatrix.h"
#include "DenseVector.h"


class CDenseMatrix : public CAbstractMatrix
{

private:
	CDenseVector* m_pMat;
	bool isChild = false;

public:

	CDenseMatrix();
	CDenseMatrix(size_t rows, size_t cols);
	CDenseMatrix(const CAbstractVector& v, bool bColumnVector);
	CDenseMatrix(const std::vector<CDenseVector>& v, bool bAsColumns);
	CDenseMatrix(const CDenseMatrix& m);
	~CDenseMatrix();

	void FreeMatrix() override;
	void AllocMatrix(size_t rows, size_t cols) override;

	CDenseMatrix& operator=(const CDenseMatrix& m);
	void Copy(const CAbstractMatrix& m) override;
	void Copy(const CDenseMatrix& m);
	void Copy(const CSparseMatrix& m);

	const CDenseVector& GetVectorAt(size_t row) const override;
	CDenseVector& GetVectorAt(size_t row) override;
	void			SetVectorAt(size_t row, const CAbstractVector& v) override;
	CDenseVector& operator[](size_t row);
	const CDenseVector& operator[](size_t row) const;

	CDenseMatrix& operator*=(const CAbstractMatrix& m);

	CDenseMatrix operator+(const CAbstractMatrix& m) const;
	CDenseMatrix operator-(const CAbstractMatrix& m) const;
	CDenseMatrix operator*(const CAbstractMatrix& m) const;
	CDenseVector operator*(const CAbstractVector& v) const;

#ifdef _WIN32
	CDenseMatrix ParallelProduct(const CAbstractMatrix& m) const;
	CDenseVector ParallelProduct(const CAbstractVector& m) const;
#endif

	CDenseMatrix operator+(double s) const;
	CDenseMatrix operator-(double s) const;
	CDenseMatrix operator*(double s) const;
	CDenseMatrix operator/(double s) const;

	CDenseVector Row(size_t r) const;
	CDenseVector Col(size_t c) const;
	CDenseVector Diagonal() const override;

	CSparseMatrix ToCSparseMatrix() const;

	CDenseMatrix Transpose() const;
	CDenseMatrix Inverse() const;
	CDenseMatrix Reshape(size_t rows, size_t cols) const;
	void SortByColumn(size_t col);
	void SortByRow(size_t row);

	static CDenseMatrix IdentityMatrix(size_t size);

	void QuadraticColInterpolation(size_t k, CDenseMatrix& matDest) const;
	void QuadraticRowInterpolation(size_t k, CDenseMatrix& matDest) const;
	void CubicSplineColInterpolation(size_t k, CDenseMatrix& matDest) const;
	void CubicSplineRowInterpolation(size_t k, CDenseMatrix& matDest) const;

	void Resize(size_t rows, size_t cols);

	bool SmoothMG(int redExp, bool smoothCols = false);
	void Child2D(CDenseMatrix& source, size_t row, size_t col, size_t rowCount, size_t colCount);

protected:
	bool SmoothMG(CDenseMatrix& X, int redExp);
};

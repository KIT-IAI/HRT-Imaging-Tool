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
#include "SparseMatrix.h"

#ifdef _WIN32
#include <ppl.h>
#endif



CSparseMatrix::CSparseMatrix()
{
	m_nMatrixType = eSparse;
	//m_pMat = nullptr;
	AllocMatrix(0, 0);
}

CSparseMatrix::CSparseMatrix(const CSparseMatrix& M)
{
	m_nMatrixType = eSparse;
	//m_pMat = nullptr;
	AllocMatrix(0, 0);
	Copy(M);
}

CSparseMatrix::CSparseMatrix(const CAbstractMatrix& M)
{
	m_nMatrixType = eSparse;
	//m_pMat = nullptr;
	AllocMatrix(0, 0);
	Copy(M);
}

CSparseMatrix::CSparseMatrix(const CAbstractVector& v, bool bColumnVector)
{
	assert(!v.IsNull());
	m_nMatrixType = eSparse;
	//m_pMat = nullptr;

	if (bColumnVector)
	{
		m_nRows = v.Size();
		m_nCols = 1;

		m_pMat.resize(m_nRows);
		//m_pMat = new CSparseVector[m_nRows];

		CSparseVector vec(1);

		for (size_t i = 0; i < Rows(); i++)
		{
			vec.SetValueAt(0, v.GetValueAt(i));
			m_pMat[i] = vec;
		}
	}
	else
	{
		m_nRows = 1;
		m_nCols = v.Size();
		m_pMat.resize(1);
		m_pMat[0].Copy(v);
	}
}

CSparseMatrix::CSparseMatrix(size_t rows, size_t cols)
{
	m_nMatrixType = eSparse;
	//m_pMat = nullptr;
	AllocMatrix(rows, cols);
}

CSparseMatrix::~CSparseMatrix(void)
{
	FreeMatrix();
}

void CSparseMatrix::AllocMatrix(size_t r, size_t c)
{
	FreeMatrix();
	m_nRows = r;
	m_nCols = c;
	if (m_nRows > 0)
	{
		m_pMat.resize(m_nRows);
		for (size_t i = 0; i < m_nRows; i++)
			m_pMat[i].AllocVector(c);
	}
	else
	{
		m_pMat.clear();
		//m_pMat = nullptr;
	}
}

void CSparseMatrix::FreeMatrix()
{
	Flush();
	m_pMat.clear();
	/*if (m_pMat != nullptr)
	{
		delete [] m_pMat;
		m_pMat = nullptr;
	}*/
	m_nRows = 0;
	m_nCols = 0;
}

CSparseMatrix& CSparseMatrix::operator=(const CSparseMatrix& m)
{
	Copy(m);
	return *this;
}

/*
*
* Creates a CSparseMatrix out of the CAbstractMatrix
*
*/
void CSparseMatrix::Copy(const CAbstractMatrix& am)
{
	if (am.GetMatrixType() == eDense)
		Copy(static_cast<const CDenseMatrix&>(am));
	else if (am.GetMatrixType() == eSparse)
		Copy(static_cast<const CSparseMatrix&>(am));
	else
		CAbstractMatrix::Copy(am);
}

void CSparseMatrix::Copy(const CDenseMatrix& m)
{
	if (m_nRows != m.Rows() || m_nCols != m.Cols())
		AllocMatrix(m.Rows(), m.Cols());

	for (size_t r = 0; r < m_nRows; r++)
	{
		m_pMat[r] = m.GetVectorAt(r).ToSparseVector();
	}
}

void CSparseMatrix::Copy(const CSparseMatrix& m)
{
	if (m_nRows != m.Rows() || m_nCols != m.Cols())
		AllocMatrix(m.Rows(), m.Cols());

	for (size_t r = 0; r < m_nRows; r++)
	{
		m_pMat[r] = m.GetVectorAt(r);
	}
}

CSparseMatrix& CSparseMatrix::operator*=(const CAbstractMatrix& m)
{
	assert(m_nRows == m_nCols && m.Rows() == m_nRows && m.Cols() == m_nCols);
	CSparseMatrix mm(*this);
	for (size_t r = 0; r < m_nRows; r++)
	{
		for (size_t c = 0; c < m_nCols; c++)
		{
			double sum = 0;
			for (size_t i = 0; i < m_nCols; i++)
			{
				sum += mm.m_pMat[r][i] * m.GetValueAt(i, c);
			}
			SetValueAt(r, c, sum);
		}
	}
	return *this;
}

CSparseMatrix CSparseMatrix::operator+(const CAbstractMatrix& m) const
{
	assert(m.Rows() == m_nRows && m.Cols() == m_nCols);
	CSparseMatrix mm(m.Rows(), m.Cols());

	for (size_t r = 0; r < m.Rows(); r++)
	{
		mm.m_pMat[r] = m_pMat[r] + m.GetVectorAt(r);
	}
	return mm;
}

CSparseMatrix CSparseMatrix::operator-(const CAbstractMatrix& m) const
{
	assert(m.Rows() == m_nRows && m.Cols() == m_nCols);
	CSparseMatrix mm(m.Rows(), m.Cols());

	for (size_t r = 0; r < m.Rows(); r++)
	{
		mm.m_pMat[r] = GetVectorAt(r) - m.GetVectorAt(r);
	}
	return mm;
}

CSparseMatrix CSparseMatrix::operator*(const CAbstractMatrix& m) const
{
	assert(m_nCols == m.Rows());

	CSparseMatrix mm(m_nRows, m.Cols());

	for (size_t r = 0; r < m_nRows; r++)
	{
		for (size_t c = 0; c < m.Cols(); c++)
		{

			CDenseVector mCol(m.Rows());

			for (size_t i = 0; i < m.Rows(); i++)
			{
				mCol[i] = m.GetValueAt(i, c);
			}

			mm.SetValueAt(r, c, m_pMat[r] * mCol);
		}
	}
	return mm;
}

CSparseVector CSparseMatrix::operator*(const CAbstractVector& v) const
{
	assert(m_nCols == v.Size());
	CSparseVector vv(m_nRows);
	for (size_t r = 0; r < m_nRows; r++)
	{
		vv.SetValueAt(r, m_pMat[r] * v);
	}
	return vv;
}
CDenseVector CSparseMatrix::operator*(const CDenseVector& v) const
{
	assert(m_nCols == v.Size());
	CDenseVector vv(m_nRows);
	for (size_t r = 0; r < m_nRows; r++)
	{
		vv.SetValueAt(r, m_pMat[r] * v);
	}
	return vv;
}

#ifdef _WIN32
/*
*
* This does the same the normal *-Operator also does, but uses parallelization through an omp-for.
* Because of its overhead it is not always useful.
*
*/
CSparseMatrix CSparseMatrix::ParallelProduct(const CAbstractMatrix& m) const
{
	assert(m_nCols == m.Rows());

	CSparseMatrix mm(m_nRows, m.Cols());

	concurrency::parallel_for(size_t(0), m_nRows, [&](size_t r)
		{
			for (size_t c = 0; c < m.Cols(); c++)
			{

				CDenseVector mCol(m.Rows());

				for (size_t i = 0; i < m.Rows(); i++)
				{
					mCol[i] = m.GetValueAt(i, c);
				}

				mm.SetValueAt(r, c, m_pMat[r] * mCol);
			}
		});
	return mm;
}

/*
*
* This does the same the normal *-Operator also does, but uses parallelization through an omp-for.
* Because of its overhead it is not always useful.
*
*/
CSparseVector CSparseMatrix::ParallelProduct(const CAbstractVector& v) const
{
	assert(m_nCols == v.Size());
	CSparseVector vv(m_nRows);

	concurrency::parallel_for(size_t(0), m_nRows, [&](size_t r)
		{
			vv.SetValueAt(r, m_pMat[r] * v);
		});
	return vv;
}
#endif // #ifdef _WIN32

CSparseMatrix CSparseMatrix::operator+(double s) const
{
	CSparseMatrix mm(m_nRows, m_nCols);
	for (size_t r = 0; r < m_nRows; r++)
	{
		mm.m_pMat[r] = m_pMat[r] + s;
	}
	return mm;
}

CSparseMatrix CSparseMatrix::operator-(double s) const
{
	CSparseMatrix mm(m_nRows, m_nCols);
	for (size_t r = 0; r < m_nRows; r++)
	{
		mm.m_pMat[r] = m_pMat[r] - s;
	}
	return mm;
}

CSparseMatrix CSparseMatrix::operator*(double s) const
{
	CSparseMatrix mm(m_nRows, m_nCols);
	for (size_t r = 0; r < m_nRows; r++)
	{
		mm.m_pMat[r] = m_pMat[r] * s;
	}
	return mm;
}

CSparseMatrix CSparseMatrix::operator/(double s) const
{
	assert(s != 0.0);

	CSparseMatrix mm(m_nRows, m_nCols);
	for (size_t r = 0; r < m_nRows; r++)
	{
		mm.m_pMat[r] = m_pMat[r] / s;
	}
	return mm;
}

const CSparseVector& CSparseMatrix::GetVectorAt(size_t row) const
{
	assert((row >= 0) && (row < m_nRows));
	return m_pMat[row];
}

CSparseVector& CSparseMatrix::GetVectorAt(size_t row)
{
	assert((row >= 0) && (row < m_nRows));
	return m_pMat[row];
}

void CSparseMatrix::SetVectorAt(size_t row, const CAbstractVector& v)
{
	assert((row >= 0) && (row < m_nRows));
	m_pMat[row].Copy(v);
}

CSparseVector& CSparseMatrix::operator[](size_t row)
{
	return GetVectorAt(row);
}

const CSparseVector& CSparseMatrix::operator[](size_t row) const
{
	return GetVectorAt(row);
}

CSparseVector CSparseMatrix::Row(size_t r) const
{
	return m_pMat[r];
}

CSparseVector CSparseMatrix::Col(size_t c) const
{
	CSparseVector v(m_nRows);
	for (size_t i = 0; i < m_nRows; i++)
	{
		v.SetValueAt(i, m_pMat[i][c]);
	}
	return v;
}

CDenseVector CSparseMatrix::Diagonal() const
{
	assert(Rows() == Cols());

	CDenseVector diag(Rows());

	for (size_t i = 0; i < Rows(); i++)
	{
		diag.SetValueAt(i, GetValueAt(i, i));
	}
	return diag;
}

/**	\brief Erzeugt die transponierte Matrix.
 *
 *	\return Die transponierte Matrix.
 */
CSparseMatrix CSparseMatrix::Transpose() const
{
	CSparseMatrix out(this->Cols(), this->Rows());
	for (size_t i = 0; i < out.Rows(); i++)
	{
		out[i] = this->Col(i);
	}
	return out;
}

/*
* Wandelt die Sparsematrix in eine reguläre Matrix um.
*/
CDenseMatrix CSparseMatrix::ToDenseMatrix() const
{
	CDenseMatrix m;
	m.Copy(*this);
	return m;
}

/*
*
* Flushes every CSparseVector included in this Matrix
*
*/
void CSparseMatrix::Flush()
{
	if (m_bIsChild || IsFlushed())
		return;
	for (size_t i = 0; i < Rows(); i++)
	{
		GetVectorAt(i).Flush();
	}
}

/*
*
* Checks if every CSparseVector included in this Matrix is Flushed
*
*/
bool CSparseMatrix::IsFlushed()
{
	bool isFlushed = true;
	for (size_t i = 0; isFlushed && i < Rows(); i++)
	{
		isFlushed &= GetVectorAt(i).IsFlushed();
	}
	return isFlushed;
}

void CSparseMatrix::Child2D(CSparseMatrix& source, size_t row, size_t col, size_t rowCount, size_t colCount)
{
	assert(row == 0 && col == 0);  //im Moment gibt es einen Fehler in der Indexberechnung des Sparsevectors (zusätzlich ist einfügen gesperrt)
	FreeMatrix();
	source.Flush();
	m_bIsChild = true;
	std::vector<CSparseVector> childMat;// = new CSparseVector[rowCount];
	for (size_t i = 0; i < rowCount; i++)
	{
		childMat[i].Child1D(source.m_pMat[row + i], col, colCount);
	}
	m_pMat = childMat;
	m_nCols = colCount;
	m_nRows = rowCount;
}

void CSparseMatrix::Expand(size_t row, size_t col)	//todo move Konstruktor in Sparcevector;
{
	assert(m_nCols <= col);
	assert(m_nRows <= row);
	m_nRows = row;
	m_nCols = col;
	if (m_nRows > 0)
	{
		m_pMat.resize(m_nRows);
		for (size_t i = 0; i < m_nRows; i++)
			m_pMat[i].Expand(col);
	}
}

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

#include "AbstractVector.h"
#include "DenseVector.h"

class CAbstractMatrix
{

protected:
	enum EMatrixType
	{
		eAbstract = 0,
		eDense = 1,
		eSparse = 2
	};

	size_t m_nRows;
	size_t m_nCols;
	EMatrixType m_nMatrixType;

public:
	class iterator;

	CAbstractMatrix(void);
	virtual ~CAbstractMatrix(void);

	int GetMatrixType() const;

	virtual void FreeMatrix() = 0;
	virtual void AllocMatrix(size_t rows, size_t cols) = 0;

	virtual CAbstractMatrix& operator=(const CAbstractMatrix& m);
	virtual void Copy(const CAbstractMatrix& m);

	virtual const CAbstractVector& GetVectorAt(size_t row) const = 0;
	virtual CAbstractVector& GetVectorAt(size_t row) = 0;
	virtual void					SetVectorAt(size_t row, const CAbstractVector& v) = 0;
	virtual double					GetValueAt(size_t row, size_t col) const;
	virtual void					SetValueAt(size_t row, size_t col, double value);

	CAbstractMatrix& operator+=(const CAbstractMatrix& m);
	CAbstractMatrix& operator-=(const CAbstractMatrix& m);

	CAbstractMatrix& operator+=(double s);
	CAbstractMatrix& operator-=(double s);
	CAbstractMatrix& operator*=(double s);
	CAbstractMatrix& operator/=(double s);


	bool operator==(const CAbstractMatrix& m) const;
	bool operator!=(const CAbstractMatrix& m) const;

	bool IsEqual(const CAbstractMatrix& m, int nPrecision) const;
	bool IsEqual(const CAbstractMatrix& m, double fEpsilon) const;
	bool IsNull() const;

	size_t Rows() const;
	size_t Cols() const;

	virtual void Fill(double s);

	double Mean() const;
	double Min() const;
	double Max() const;
	double MinAbs() const;
	double MaxAbs() const;
	double Min(size_t& row, size_t& col) const;
	double Max(size_t& row, size_t& col) const;
	double MinAbs(size_t& row, size_t& col) const;
	double MaxAbs(size_t& row, size_t& col) const;
	void MinMax(size_t& rowMin, size_t& colMin, size_t& rowMax, size_t& colMax) const;
	size_t GetNonZeroElementCount() const;

	bool PointwiseMultplication(const CAbstractMatrix& mat1, const CAbstractMatrix& mat2);
	bool ReduceMatrix(const CAbstractMatrix& source, size_t reductionRow, size_t reductionCol, bool doInterpolation = false);
	bool ProlongMatrix(const CAbstractMatrix& source, size_t prologationRow, size_t prolongationCol);

	virtual void SetCol(const CAbstractVector& v, size_t n);
	void CropMatrixY(CAbstractMatrix& source);
	virtual CDenseVector Diagonal() const = 0;

	std::wstring ToString(int nPrecision = 6) const;
	std::wstring ToString(const std::wstring& sElementDelimiter, const std::wstring& sRowDelimiter, int nPrecision = 6) const;

	bool AllocateAndReadMatrix(const std::wstring& sFilePath);
	bool ReadMatrix(const std::wstring& sFilePath);
	bool WriteMatrixMM(const std::wstring& sFilePath, int nPrecision = 6) const;
	bool WriteMatrix(const std::wstring& sFilePath, int nPrecision = 6) const;
	bool WriteMatrix(const std::wstring& sFilePath, const std::wstring& sElementDelimiter, const std::wstring& sRowDelimiter, int nPrecision = 6) const;

	void CopyToCSR(std::vector<double>& val, std::vector<size_t>& colInd, std::vector<size_t>& rowPtr);
//	void CopyToCSR(std::vector<double>& val, std::vector<int>& colInd, std::vector<int>& rowPtr);

	void CopyRoi(size_t targetrow, size_t targetcol, const CAbstractMatrix& source, size_t sourcerow, size_t sourcecol, size_t rowcount, size_t colcount);

	iterator begin() const;
	iterator end() const;

	class iterator
	{
	public:
		using iterator_category = std::output_iterator_tag;
		using value_type = CAbstractVector;
		using difference_type = std::ptrdiff_t;
		using pointer = CAbstractVector*;
		using reference = CAbstractVector&;
		iterator(const CAbstractMatrix& Matrix, size_t nIndex = 0);
		const CAbstractVector& operator*() const;
		iterator& operator++();
		iterator& operator++(int);
		bool operator!=(const iterator& rhs) const;
	private:
		size_t nIndex = 0;
		const CAbstractMatrix& Matrix;
	};
};

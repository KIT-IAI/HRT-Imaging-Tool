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


#include <string>
#include <vector>


class CAbstractVector
{
protected:
	enum EVectorType {
		eAbstract = 0,
		eDense = 1,
		eSparse = 2
	};

	size_t m_nSize;
	EVectorType m_nVectorType;

public:
	class iterator;

	CAbstractVector(void);
	virtual ~CAbstractVector(void);


	virtual void AllocVector(size_t size) = 0;
	virtual void FreeVector() = 0;
	virtual void Copy(const CAbstractVector& vs);

	virtual void SetValueAt(size_t index, double value) = 0;
	virtual const double GetValueAt(size_t index) const = 0;
	virtual std::vector<double>GetValuesAsArray() const;

	virtual CAbstractVector& operator	= (const CAbstractVector& v);
	bool						operator	==(const CAbstractVector& v) const;
	bool						operator	!=(const CAbstractVector& v) const;

	virtual CAbstractVector& operator+=(const CAbstractVector& v);
	virtual CAbstractVector& operator-=(const CAbstractVector& v);
	virtual CAbstractVector& operator+=(double s);
	virtual CAbstractVector& operator-=(double s);
	virtual CAbstractVector& operator*=(double s);
	virtual CAbstractVector& operator/=(double s);

	virtual void Fill(double s);

	virtual double Sum() const;
	virtual double Mean() const;
	double Min() const;
	double Max() const;
	std::tuple<double, double> MinMax() const;
	double MinAbs() const;
	double MaxAbs() const;
	virtual double Min(size_t& pos) const;
	virtual double Max(size_t& pos) const;
	virtual double MinAbs(size_t& pos) const;
	virtual double MaxAbs(size_t& pos) const;


	virtual bool	IsEqual(const CAbstractVector& v, int nPrecision) const;
	virtual bool	IsEqual(const CAbstractVector& v, double fEpsilon) const;
	bool			IsNull() const;
	virtual bool	IsZero() const;
	size_t			Size() const;
	virtual size_t NonZeroSize() const;
	int				GetVectorType() const;

	bool PointwiseMultiplication(const CAbstractVector& vector1, const CAbstractVector& vector2);
	bool PointwiseDivision(const CAbstractVector& vector1, const CAbstractVector& vector2);

	void ApplyFilter(const CAbstractVector& source, const std::vector<double>& filter, int filterHotSpot = -1);
	void ApplyFilter(const CAbstractVector& source, const CAbstractVector& filter, int filterHotSpot = -1);
	void SmoothVector(size_t nTimes = 1);

	std::wstring ToString(int nPrecision = 6) const;
	std::wstring ToString(const std::wstring& sDelimiter, int nPrecision = 6) const;

	bool AllocateAndReadVector(const std::wstring& sFilePath);
	bool ReadVector(const std::wstring& sFilePath);
	bool WriteVector(const std::wstring& sFilePath, int nPrecision = 6) const;
	bool WriteVector(const std::wstring& sFilePath, const std::wstring& sDelimiter, int nPrecision = 6) const;

	virtual iterator begin() const;
	virtual iterator end() const;

	class iterator 
	{
	public:
		using iterator_category = std::output_iterator_tag;
		using value_type = double;
		using difference_type = std::ptrdiff_t;
		using pointer = double*;
		using reference = double&;
		iterator(const CAbstractVector& Matrix, size_t nIndex = 0);
		double operator*() const;
		iterator& operator++();
		iterator& operator++(int);
		iterator& operator+(int);
		bool operator!=(const iterator& rhs) const;
	private:
		size_t nIndex = 0;
		const CAbstractVector& Vector;
	};
};

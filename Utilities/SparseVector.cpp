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
#include "SparseVector.h"



CSparseVector::CSparseVector(void) : CSparseVector(0)
{
}

CSparseVector::CSparseVector(size_t size)
{
	m_nVectorType = eSparse;
	Init();

	AllocVector(size, 0);
}

CSparseVector::CSparseVector(const CSparseVector& av)
{
	m_nVectorType = eSparse;
	Init();

	AllocVector(0, 0);
	Copy(av);
}

CSparseVector::CSparseVector(size_t size, size_t nonZeroSize, size_t* columns, double* values)
{
	m_nVectorType = eSparse;
	Init();

	m_nSize = size;
	m_nNonZeroSize = nonZeroSize;
	m_pColumns = columns;
	m_pValues = values;
	InitCache();
}

CSparseVector::~CSparseVector(void)
{
	FreeVector();

	if (m_lChildren.size() != 0)
	{
		assert(false); // Free allocated Children first!!!
	}
}

void CSparseVector::Init(void)
{
	m_pCacheLineColumns = 0;
	m_pCacheLineValues = 0;
	m_pColumns = 0;
	m_pValues = 0;
}

void CSparseVector::AllocVector(size_t nSize)
{
	AllocVector(nSize, 0);
}

/*
* This function will allocate the two arrays m_pColumns and m_pValues.
* Also the sizes are set.
*/
void CSparseVector::AllocVector(size_t nSize, size_t nNonZeroSize)
{
	FreeVector();
	m_nSize = nSize;
	m_nNonZeroSize = nNonZeroSize;
	if (nNonZeroSize != 0)
	{
		assert(m_pColumns == NULL);
		assert(m_pValues == NULL);

		m_pColumns = new size_t[nNonZeroSize];
		m_pValues = new double[nNonZeroSize];

		if (m_lChildren.size() > 0)
			NotifyChildren();
	}
	else
	{
		m_pColumns = NULL;
		m_pValues = NULL;
	}
	InitCache();
}

void CSparseVector::FreeVector()
{
	if (m_bIsChild)
	{
		auto pos = m_lChildren.begin();
		while ((pos != m_lChildren.end()) && (*pos != this)) pos++;
		assert(pos != m_lChildren.end());
		m_pParent->m_lChildren.erase(pos);
		m_bIsChild = false;
		m_pColumns = nullptr;
		m_pValues = nullptr;
		m_pCacheLineColumns = nullptr;
		m_pCacheLineValues = nullptr;
	}

	if (m_pColumns != nullptr)
	{
		delete[] m_pColumns;
		m_pColumns = nullptr;
	}
	if (m_pValues != nullptr)
	{
		delete[] m_pValues;
		m_pValues = nullptr;
	}
	if (m_pCacheLineColumns != nullptr)
	{
		delete[] m_pCacheLineColumns;
		m_pCacheLineColumns = nullptr;
	}
	if (m_pCacheLineValues != nullptr)
	{
		delete[] m_pCacheLineValues;
		m_pCacheLineValues = nullptr;
	}
	m_nSize = 0;
}

void CSparseVector::InitCache()
{
	assert(m_pCacheLineColumns == NULL);
	assert(m_pCacheLineValues == NULL);

	m_nCacheLength = 0;
	m_nCacheLineIndex = 0;
	RecalculateOptimalCacheLength();
}

void CSparseVector::Copy(const CAbstractVector& av)
{
	if (av.GetVectorType() == eDense)
		return Copy(static_cast<const CDenseVector&>(av));
	else if (av.GetVectorType() == eSparse)
		return Copy(static_cast<const CSparseVector&>(av));
	else
		return CAbstractVector::Copy(av);
}

/*
*
* More efficent reimplementation of the Copy method, when trying to copy one CSparseVector to another
*
*/
void CSparseVector::Copy(const CSparseVector& sv)
{
	FreeVector();

	assert(IsNull());


	// This Const-Cast is evil, but necessary to ensure the abstraction of the method Copy
	// The vector needs to be flushed in order to be able to use memcpy 
	// And since a simple Flush doesn't modify the object for the user
	// This const_cast is justified
	const_cast<CSparseVector&>(sv).Flush();

	AllocVector(sv.Size(), sv.NonZeroSize());

#ifdef _WIN32
	memcpy_s(m_pColumns, m_nNonZeroSize * sizeof(size_t), sv.m_pColumns, m_nNonZeroSize * sizeof(size_t));
	memcpy_s(m_pValues, m_nNonZeroSize * sizeof(double), sv.m_pValues, m_nNonZeroSize * sizeof(double));
#else
	std::memcpy(m_pColumns, sv.m_pColumns, m_nNonZeroSize * sizeof(size_t));
	std::memcpy(m_pValues, sv.m_pValues, m_nNonZeroSize * sizeof(double));
#endif
}

/*
*
* More efficent reimplementation of the Copy method, when trying to convert a DenseVector to a CSparseVector
*
*/
void CSparseVector::Copy(const CDenseVector& v)
{
	FreeVector();
	assert(IsNull());
	AllocVector(v.Size(), v.NonZeroSize());

	assert(m_pColumns != NULL);
	assert(m_pValues != NULL);
	assert(Size() == v.Size());
	assert(Size() != 0);


	size_t fillIndex = 0;

	for (size_t iterator = 0; iterator < Size(); iterator++)
	{
		if (v[iterator] != 0)
		{
			m_pColumns[fillIndex] = iterator;
			m_pValues[fillIndex] = v[iterator];
			fillIndex++;
		}
	}
}

const double CSparseVector::GetValueAt(size_t column) const
{
	assert((column >= m_nChildOffset) && (column < Size()));	// Out of Bounds

	size_t index;
	double  value;

	if (LinearSearchInCache(column + m_nChildOffset, index, value))					// Linear Searches Value in Cache
		return value;
	if (BinarySearchInValues(column + m_nChildOffset, index, value))				// Binary Searches Value in Array
		return value;
	return 0;																		// Value not found => 0
}

void CSparseVector::SetValueAt(size_t column, double value)
{
	assert((column >= 0) && (column < Size()));

	if (m_bIsChild)		// Children can not write
	{
		assert(false);
		return;
	}

	size_t index;
	double  oldValue;

	if (LinearSearchInCache(column, index, oldValue))
	{
		m_pCacheLineValues[index] = value;
	}
	else
	{
		auto cache_pos = MoveValueToCache(column);
		m_pCacheLineValues[cache_pos] = value;
		if (m_lChildren.size() > 0)
			NotifyChildren();
	}
}

std::vector<double> CSparseVector::GetValuesAsArray() const
{
	if (!IsFlushed())
		return CAbstractVector::GetValuesAsArray();

	std::vector<double> values(Size());
	std::fill(values.begin(), values.end(), 0.0);

	for (size_t i = 0; i < m_nNonZeroSize; i++)
	{
		values[m_pColumns[i]] = m_pValues[i];
	}

	return values;
}

const double CSparseVector::operator[](const size_t ndx) const
{
	return GetValueAt(ndx);
}

CSparseVector& CSparseVector::operator=(const CSparseVector& v)
{
	Copy(v);
	return *this;
}

double CSparseVector::operator*(const CAbstractVector& v) const
{
	assert(v.Size() == m_nSize);

	const_cast<CSparseVector*> (this)->Flush();

	double sum = 0;
	for (size_t i = 0; i < m_nNonZeroSize; i++)
	{
		sum += m_pValues[i] * v.GetValueAt(m_pColumns[i]);				// This can be done so smoothly because where one Vector is 0, the product must be 0 too.
	}

	return sum;
}

CSparseVector CSparseVector::operator+(const CAbstractVector& v) const
{
	assert(v.Size() == Size());
	CSparseVector vv(v.Size());
	for (size_t i = 0; i < v.Size(); i++)
	{
		vv.SetValueAt(i, GetValueAt(i) + v.GetValueAt(i));
	}
	return vv;
}

CSparseVector CSparseVector::operator-(const CAbstractVector& v) const
{
	assert(v.Size() == Size());
	CSparseVector vv(v.Size());
	for (size_t i = 0; i < v.Size(); i++)
	{
		vv.SetValueAt(i, GetValueAt(i) - v.GetValueAt(i));
	}
	return vv;
}

CSparseVector CSparseVector::operator+(double s) const
{
	CSparseVector vv(Size());
	for (size_t i = 0; i < Size(); i++)
	{
		vv.SetValueAt(i, GetValueAt(i) + s);
	}
	return vv;
}

CSparseVector CSparseVector::operator-(double s) const
{
	CSparseVector vv(Size());
	for (size_t i = 0; i < Size(); i++)
	{
		vv.SetValueAt(i, GetValueAt(i) - s);
	}
	return vv;
}

CSparseVector CSparseVector::operator*(double s) const
{
	const_cast<CSparseVector*> (this)->Flush();

	size_t* newColumns = new size_t[m_nNonZeroSize];
	double* newValues = new double[m_nNonZeroSize];


	for (size_t i = 0; i < m_nNonZeroSize; i++)
	{
		newColumns[i] = m_pColumns[i];
		newValues[i] = m_pValues[i] * s;
	}
	return CSparseVector(m_nSize, m_nNonZeroSize, newColumns, newValues);
}

CSparseVector CSparseVector::operator/(double s) const
{
	const_cast<CSparseVector*> (this)->Flush();

	size_t* newColumns = new size_t[m_nNonZeroSize];
	double* newValues = new double[m_nNonZeroSize];


	for (size_t i = 0; i < m_nNonZeroSize; i++)
	{
		newColumns[i] = m_pColumns[i];
		newValues[i] = m_pValues[i] / s;
	}
	return CSparseVector(m_nSize, m_nNonZeroSize, newColumns, newValues);
}

/*
* Returns a column directly from the m_pColumns Array.
* This is a lot faster than searching it using the access-operator.
*
* CAREFUL! This only works when the Cache is flushed!
*/
size_t CSparseVector::GetNonZeroColumnAt(size_t index) const
{
	assert(IsFlushed());
	return m_pColumns[index];
}

/*
* Returns a value directly from the m_pValues Array.
* This is a lot faster than searching it using the access-operator.
*
* CAREFUL! This only works when the Cache is flushed!
*/
double CSparseVector::GetNonZeroValueAt(size_t index) const
{
	assert(IsFlushed());
	return m_pValues[index];
}

/*
*
* Returns the number of elements that are not zero
*
* CAREFUL! This operation may take long ( O(M+log(N) )
* It is better to flush the Vector if possible
*/
size_t CSparseVector::NonZeroSize() const
{
	return m_nNonZeroSize + CheckForProlungation();
}

/*
*
* Converts the CSparseVector into a DenseVector
*
*/
CDenseVector CSparseVector::ToDenseVector() const
{
	CDenseVector newVector(Size());
	newVector.Fill(0.0);


	for (size_t i = 0; i < m_nNonZeroSize; i++)
	{
		newVector[m_pColumns[i]] = m_pValues[i];
	}
	for (size_t i = 0; i < m_nCacheLineIndex; i++)
	{
		newVector[m_pCacheLineColumns[i]] = m_pCacheLineValues[i];
	}

	return newVector;
}

/*
*
* Reimplementation of the CAbstractVector::Fill() operation for efficency purposes.
*
*/
void CSparseVector::Fill(double s)
{
	if (m_bIsChild)		// Children can not write
	{
		assert(false);
		return;
	}

	size_t oldSize = m_nSize;
	FreeVector();

	if (s == 0)
		AllocVector(oldSize, 0);
	else
	{
		AllocVector(oldSize, oldSize);

		for (size_t i = 0; i < oldSize; i++)
		{
			m_pColumns[i] = i;
			m_pValues[i] = s;
		}
	}
}

void CSparseVector::Swap(CSparseVector& sv)
{
	Flush();
	sv.Flush();

	double* tempVals = sv.m_pValues;
	size_t* tempCols = sv.m_pColumns;
	sv.m_pValues = m_pValues;
	sv.m_pColumns = m_pColumns;
	m_pValues = tempVals;
	m_pColumns = tempCols;
}

double CSparseVector::Norm2()
{
	return sqrt((*this) * (*this));
}

bool CSparseVector::IsFull() const
{
	return m_nSize == NonZeroSize();
}

bool CSparseVector::IsDense() const
{
	return m_nSize / 2 < NonZeroSize();
}

bool CSparseVector::IsCacheFull() const
{
	return (m_nCacheLineIndex == m_nCacheLength);
}

bool CSparseVector::IsFlushed() const
{
	return (m_nCacheLineIndex == 0);
}

bool CSparseVector::IsZero() const
{
	return NonZeroSize() == 0;
}

/*
*
* Writes back CacheLine into ValueLine
*
* Complexity:	O(getEnlargement) + O(ModifyColumns)
*				= O(M) + O(M^2+N)
*				= O(N)
*
* M = CacheLine-length
* Nz = Non-Zero-length
*/
void CSparseVector::Flush()
{
	if (IsFlushed())										// Does no operation if the cache is empty
		return;


	size_t enlargement = CheckForProlungation();

	ModifyColumns(enlargement);							// We have to adjust apply those additions and deletions

	m_nCacheLineIndex = 0;


	if (m_lChildren.size() > 0)
		NotifyChildren();
}

/*
*
* Checks the Cache and returns the difference in length of the Values-Array
*
* In other words, returns +1 for every added Column and -1 for every Removed Column
*
* Returns true if there were any modifications (This is needed, because 1 deleted and 1 added Column would return 0 es enlargement)
*
* Complexity: O(M+log(N))
*/
size_t CSparseVector::CheckForProlungation() const
{

	size_t outProlungation = 0;

	for (size_t i = 0; i < m_nCacheLineIndex; i++)
	{
		size_t columnIndex;
		double oldValue;
		bool bSuccess = BinarySearchInValues(m_pCacheLineColumns[i], columnIndex, oldValue);
		double newValue = m_pCacheLineValues[i];


		if (!bSuccess && newValue != 0)			// New Column is being inserted		(TranslateColumnToIndex returned -1 because the column is not in m_pColumns yet)
		{
			outProlungation++;
		}

		else if (bSuccess && newValue == 0)		// Column is being deleted			(since the new Value is Zero and doesnt need to be saved)
		{
			outProlungation--;
		}

		else if (bSuccess && newValue != 0)
		{
		}
	}
	return outProlungation;
}

/*
* Moves a Value from the Values-Arrays to the Cache-Array and returns it's index in the Cache
*/
size_t CSparseVector::MoveValueToCache(size_t column)
{
	if (IsCacheFull())
		Flush();

	m_pCacheLineColumns[m_nCacheLineIndex] = column;

	return m_nCacheLineIndex++;									// God that's a line you could read in a book! It returns the OLD index and then increses it by 1 _afterwards_.
}

/*
*
* Searches for a value in the cacheLine using a linear search
* Room for improvement (MoveToFront or Transpose - Algorithm (?))
*
* Complexity: O(M)
*
* M = Cachelength
*/
bool CSparseVector::LinearSearchInCache(size_t inColumn, size_t& outColumn, double& outValue) const
{
	for (ptrdiff_t i = m_nCacheLineIndex - 1; i >= 0; i--)
	{
		if (m_pCacheLineColumns[i] == inColumn)
		{
			outColumn = i;
			outValue = m_pCacheLineValues[i];
			return true;
		}
	}
	return false;
}

/*
*
* Searches for a column in the values-array using binary search
*
* Returns true and sets outColumn and outValue if the column is found
* Returns false and sets Value to 0 and column to -1 if the column is not found => is Zero
*
* Complexity O(log2(N))
*
* N = NonZeroSize
*/
bool CSparseVector::BinarySearchInValues(size_t inColumn, size_t& outColumn, double& outValue) const
{
	assert(inColumn < Size());

	if (m_nNonZeroSize == 0 || inColumn > m_pColumns[m_nNonZeroSize - 1] || inColumn < m_pColumns[0])
	{
		outColumn = 0;
		outValue = 0;
		return false;
	}

	size_t links = 0;
	size_t rechts = m_nNonZeroSize;
	size_t mitte = 0;
	while (links < rechts)
	{
		mitte = (rechts + links) / 2;

		if (inColumn > m_pColumns[mitte])
			links = mitte + 1;
		else
			rechts = mitte;
	}

	if (links < m_nNonZeroSize && m_pColumns[links] == inColumn)
	{
		outColumn = mitte;
		outValue = m_pValues[links];
		return true;
	}

	outColumn = 0;
	outValue = 0;
	return false;
}

/*
*
* Applies enlargement of columns to the ValueLine and mixes the Cache with it
*
* Complexity: O(Sort) + O(N)
*
* N = Non-Zero-Length
*/
void CSparseVector::ModifyColumns(size_t enlargement)
{
	size_t newSize = m_nNonZeroSize + enlargement;

	if (newSize == 0)
		return;

	SortCache();					// Sorts all columns in the Cache to be able to merge them properly

	double* newValues = new double[newSize];
	size_t* newColumns = new size_t[newSize];

	size_t newIndex = 0;			// Index for the new ValueLine
	size_t cacheIndex = 0;			// Index for the Cacheline
	size_t valueIndex = 0;			// Index for the old ValueLine

	while (newIndex < newSize)		// Merges two sorted arrays (As e.g. Mergesort would do)
	{
		if (valueIndex == m_nNonZeroSize)									// The Values array is done, now place the cache array
		{
			if (m_pCacheLineValues[cacheIndex] != 0)						// If the new value is zero (therefore its been removed) skip it
			{
				newValues[newIndex] = m_pCacheLineValues[cacheIndex];
				newColumns[newIndex] = m_pCacheLineColumns[cacheIndex];
				newIndex++;
			}
			cacheIndex++;
		}
		else if (cacheIndex == m_nCacheLineIndex)							// The Cache array is done, now place the values array
		{
			newValues[newIndex] = m_pValues[valueIndex];
			newColumns[newIndex] = m_pColumns[valueIndex];
			newIndex++;
			valueIndex++;
		}
		else if (m_pColumns[valueIndex] == m_pCacheLineColumns[cacheIndex])	// A cell has been changed therefore the value form the cache should be used
		{
			if (m_pCacheLineValues[cacheIndex] != 0)						// If the new value is zero (therefore its been removed) skip it
			{
				newValues[newIndex] = m_pCacheLineValues[cacheIndex];
				newColumns[newIndex] = m_pCacheLineColumns[cacheIndex];
				newIndex++;
			}
			cacheIndex++;
			valueIndex++;
		}
		else if (m_pColumns[valueIndex] < m_pCacheLineColumns[cacheIndex])	// The value-column comes before the cache-column
		{
			newValues[newIndex] = m_pValues[valueIndex];
			newColumns[newIndex] = m_pColumns[valueIndex];
			newIndex++;
			valueIndex++;
		}
		else if (m_pColumns[valueIndex] > m_pCacheLineColumns[cacheIndex])	// The cache-column comes before the value-column
		{
			if (m_pCacheLineValues[cacheIndex] != 0)						// If the new value is zero (therefore its been removed) skip it
			{
				newValues[newIndex] = m_pCacheLineValues[cacheIndex];
				newColumns[newIndex] = m_pCacheLineColumns[cacheIndex];
				newIndex++;
			}
			cacheIndex++;
		}
	}

	delete[] m_pColumns;
	delete[] m_pValues;
	m_pColumns = newColumns;
	m_pValues = newValues;
	m_nNonZeroSize = newSize;
	RecalculateOptimalCacheLength();
}

/*
*
* Sorts the Cache using Bubblesort
* To be improved with more elaborated Algorithms
*
* Bubblesort is short neat and easy. And since the most elements we should have in a vector is < 50, it is completly sufficent
* Complexity O(M^2)
*/
void CSparseVector::SortCache()
{
	double tempVal;
	size_t tempCol;

	size_t earlyEnd = 1;
	bool swapsHappened = true;

	while (swapsHappened)
	{
		swapsHappened = false;
		for (size_t i = 0; i < m_nCacheLineIndex - earlyEnd; i++)
		{
			if (m_pCacheLineColumns[i] > m_pCacheLineColumns[i + 1])
			{
				tempCol = m_pCacheLineColumns[i];
				tempVal = m_pCacheLineValues[i];
				m_pCacheLineColumns[i] = m_pCacheLineColumns[i + 1];
				m_pCacheLineValues[i] = m_pCacheLineValues[i + 1];
				m_pCacheLineColumns[i + 1] = tempCol;
				m_pCacheLineValues[i + 1] = tempVal;
				swapsHappened = true;
			}
		}
		earlyEnd++;
	}
}

void CSparseVector::RecalculateOptimalCacheLength()
{
	// Optimal Cache length ~sqrt(N)
	size_t oldLength = m_nCacheLength;
	m_nCacheLength = max(static_cast<int>(sqrt(static_cast<double>(m_nNonZeroSize))), 1);  // The Max(X,1) is needed if Nz = 0 to ensure, that the Cache has at least length 1

	if (oldLength != m_nCacheLength)
	{
		delete m_pCacheLineColumns;
		delete m_pCacheLineValues;
		m_pCacheLineColumns = new size_t[m_nCacheLength];
		m_pCacheLineValues = new double[m_nCacheLength];

		if (m_lChildren.size() > 0)
			NotifyChildren();
	}
}

void CSparseVector::Child1D(CSparseVector& source, size_t startElement, size_t size)
{
	FreeVector();
	source.Flush();

	m_bIsChild = true;
	m_pParent = &source;
	source.m_lChildren.push_front(this);

	m_nChildOffset = startElement;
	m_nSize = size;

	m_pValues = source.m_pValues;
	m_pColumns = source.m_pColumns;
	m_pCacheLineColumns = source.m_pCacheLineColumns;
	m_pCacheLineValues = source.m_pCacheLineValues;
	m_nCacheLineIndex = source.m_nCacheLineIndex;

	m_nNonZeroSize = source.m_nNonZeroSize;
	for (size_t i = 0; i < source.m_nNonZeroSize; i++)
	{
		if (source.m_pColumns[i] < m_nChildOffset)
			m_nNonZeroSize--;
		else if (source.m_pColumns[i] >= m_nChildOffset + m_nSize)
		{
			m_nNonZeroSize -= (source.m_nNonZeroSize - i);
			break;
		}
	}
}

void CSparseVector::NotifyChildren()
{
	for (auto element : m_lChildren)
	{
		element->m_pValues = m_pValues;
		element->m_pColumns = m_pColumns;
		element->m_pCacheLineColumns = m_pCacheLineColumns;
		element->m_pCacheLineValues = m_pCacheLineValues;
		element->m_nCacheLineIndex = m_nCacheLineIndex;

		element->m_nNonZeroSize = m_nNonZeroSize;
		for (size_t i = 0; i < m_nNonZeroSize; i++)
		{
			if (m_pColumns[i] < element->m_nChildOffset)
				element->m_nNonZeroSize--;
			else if (m_pColumns[i] >= element->m_nChildOffset + element->m_nSize)
			{
				element->m_nNonZeroSize -= (m_nNonZeroSize - i);
				break;
			}
		}
	}
}

void CSparseVector::Expand(size_t size)
{
	assert(m_nSize <= size);
	Flush();
	m_nSize = size;
	RecalculateOptimalCacheLength();
}

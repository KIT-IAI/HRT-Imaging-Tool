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
#include <vector>

class CArrayUtilities
{

public:

	template<typename T> static void InitArray(T* pArray, const T& initValue, size_t nSize);
	template<typename T> static void InitArray(T** pArray, const T& initValue, size_t nSize1, size_t nSize2);
	template<typename T> static void CopyArray(const T* pArraySource, T* pArrayDest, size_t nSize);
	template<typename T> static void FindMin(const T* pArray, size_t nSize, T* pMin, size_t* pMinIndex);
	template<typename T> static void FindMax(const T* pArray, size_t nSize, T* pMax, size_t* pMaxIndex);
	template<typename T> static void FindExtremes(const T* pArray, size_t nSize, T* pMin, T* pMax, size_t* pMinIndex, size_t* pMaxIndex);
	template<typename T> static void MovingAverage(T* pArray, size_t nSize, size_t nWidth);
	template<typename T> static std::vector<T> FilterByIndex(const std::vector<T>& Input, const std::list<size_t>& Indices);
	template<typename T> static std::vector<T*> SmartToRaw(const std::vector<std::shared_ptr<T>>& Input);
	template<typename T> static std::vector<T*> SmartToRaw(const std::vector<std::unique_ptr<T>>& Input);
};

/**	\brief Initialisiert das eindimensionale Array \a pArray mit dem Wert
 *	\a initValue.
 *
 *	\param[in] pArray Ein Zeiger auf das zu initialisierende Array.
 *	\param[in] initValue Der Wert, mit dem das Array initialisiert wird.
 *	\param[in] nSize Die Größe des Arrays.
 *
 *	\author Stephan Allgeier
 */
template<typename T> void CArrayUtilities::InitArray(T* pArray, const T& initValue, size_t nSize)
{
	for (size_t nIndex = 0; nIndex < nSize; nIndex++)
	{
		pArray[nIndex] = initValue;
	}
}

/**	\brief Initialisiert das zweidimensionale Array \a pArray mit dem Wert
 *	\a initValue.
 *
 *	\param[in] pArray Ein Zeiger auf das zu initialisierende Array.
 *	\param[in] initValue Der Wert, mit dem das Array initialisiert wird.
 *	\param[in] nSize1 Die Größe des Arrays in der ersten Dimension.
 *	\param[in] nSize2 Die Größe des Arrays in der zweiten Dimension.
 *
 *	\author Stephan Allgeier
 */
template<typename T> void CArrayUtilities::InitArray(T** pArray, const T& initValue, size_t nSize1, size_t nSize2)
{
	for (size_t nIndex1 = 0; nIndex1 < nSize1; nIndex1++)
	{
		for (size_t nIndex2 = 0; nIndex2 < nSize2; nIndex2++)
		{
			pArray[nIndex1][nIndex2] = initValue;
		}
	}
}

/**	\brief Kopiert alle Elemente des Arrays \a pArraySource in das Array
 *	\a pArrayDest.
 *
 *	Die beiden Arrays müssen die gleiche Größe \a nSize besitzen.
 *
 *	\param[in] pArraySource Das Quell-Array.
 *	\param[in] pArraySource Das Ziel-Array.
 *	\param[in] nSize Die Größe der beiden Arrays.
 *
 *	\author Stephan Allgeier
 */
template<typename T> void CArrayUtilities::CopyArray(const T* pArraySource, T* pArrayDest, size_t nSize)
{
	for (size_t nIndex = 0; nIndex < nSize; nIndex++)
	{
		pArrayDest[nIndex] = pArraySource[nIndex];
	}
}

/**	\brief Sucht das (erste) Minimum in dem Array \a pArray.
 *
 *	Für die Parameter \a pMin und \a pMinIndex kann auch \c NULL übergeben
 *	werden, wenn das jeweilige Ergebnis nicht benötigt wird.
 *
 *	\param[in] pArray Ein Array.
 *	\param[in] nSize Die Größe des Arrays \a pArray.
 *	\param[out] pMin Enthält nach dem Methodenaufruf das (erste) Minimum des
 *		Arrays \a pArray (siehe Beschreibung).
 *	\param[out] pMinIndex Enthält nach dem Methodenaufruf den Index des
 *		(ersten) Minimums des Arrays \a pArray (siehe Beschreibung).
 *
 *	\author Stephan Allgeier
 */
template<typename T> void CArrayUtilities::FindMin(const T* pArray, size_t nSize, T* pMin, size_t* pMinIndex)
{
	FindExtremes(pArray, nSize, pMin, static_cast<T*>(NULL), pMinIndex, static_cast<size_t*>(NULL));
}

/**	\brief Sucht das (erste) Maximum in dem Array \a pArray.
 *
 *	Für die Parameter \a pMax und \a pMaxIndex kann auch \c NULL übergeben
 *	werden, wenn das jeweilige Ergebnis nicht benötigt wird.
 *
 *	\param[in] pArray Ein Array.
 *	\param[in] nSize Die Größe des Arrays \a pArray.
 *	\param[out] pMax Enthält nach dem Methodenaufruf das (erste) Maximum des
 *		Arrays \a pArray (siehe Beschreibung).
 *	\param[out] pMaxIndex Enthält nach dem Methodenaufruf den Index des
 *		(ersten) Maximums des Arrays \a pArray (siehe Beschreibung).
 *
 *	\author Stephan Allgeier
 */
template<typename T> void CArrayUtilities::FindMax(const T* pArray, size_t nSize, T* pMax, size_t* pMaxIndex)
{
	FindExtremes(pArray, nSize, static_cast<T*>(NULL), pMax, static_cast<size_t*>(NULL), pMaxIndex);
}

/**	\brief Sucht das (erste) Minimum und das (erste) Maximum in dem Array
 *	\a pArray.
 *
 *	Für die Parameter \a pMin, \a pMinIndex, \a pMax und \a pMaxIndex kann
 *	auch \c NULL übergeben werden, wenn das jeweilige Ergebnis nicht benötigt
 *	wird.
 *
 *	\param[in] pArray Ein Array.
 *	\param[in] nSize Die Größe des Arrays \a pArray.
 *	\param[out] pMin Enthält nach dem Methodenaufruf das (erste) Minimum des
 *		Arrays \a pArray (siehe Beschreibung).
 *	\param[out] pMinIndex Enthält nach dem Methodenaufruf den Index des
 *		(ersten) Minimums des Arrays \a pArray (siehe Beschreibung).
 *	\param[out] pMax Enthält nach dem Methodenaufruf das (erste) Maximum des
 *		Arrays \a pArray (siehe Beschreibung).
 *	\param[out] pMaxIndex Enthält nach dem Methodenaufruf den Index des
 *		(ersten) Maximums des Arrays \a pArray (siehe Beschreibung).
 *
 *	\author Stephan Allgeier
 */
template<typename T> void CArrayUtilities::FindExtremes(const T* pArray, size_t nSize, T* pMin, T* pMax, size_t* pMinIndex, size_t* pMaxIndex)
{
	assert(pArray != NULL);
	assert(nSize > 0);

	T nMin = pArray[0];
	T nMax = pArray[0];
	size_t nMinIndex = 0;
	size_t nMaxIndex = 0;

	for (size_t i = 0; i < nSize; i++)
	{
		if (nMin > pArray[i])
		{
			nMin = pArray[i];
			nMinIndex = i;
		}
		if (nMax < pArray[i])
		{
			nMax = pArray[i];
			nMaxIndex = i;
		}
	}

	if (pMin != NULL)
	{
		*pMin = nMin;
	}
	if (pMax != NULL)
	{
		*pMax = nMax;
	}

	if (pMinIndex != NULL)
	{
		*pMinIndex = nMinIndex;
	}
	if (pMaxIndex != NULL)
	{
		*pMaxIndex = nMaxIndex;
	}
}

/**	\brief Führt eine Glättung des Arrays \a pArray mit dem gleitenden
 *	Mittelwert der Breite \a nWidth durch.
 *
 *	Die Breite des gleitenden Mittelwerts sollte in der Regel ungerade
 *	gewählt werden. Andernfalls ist die Mittelwertberechnung nicht invariant
 *	bezüglich einer Spiegelung des Arrays.
 *
 *	Die Berechnung des gleitenden Mittelwerts wird wesentlich effizienter
 *	als andere Glättungsoperatoren. Die Laufzeit steigt linear mit der Größe
 *	des Arrays und der Breite des Mittelwerts (O(nSize+nNeighbors)).
 *
 *	\param[in] pArray Das zu glättende Array.
 *	\param[in] nSize Die Größe des Arrays \a pArray.
 *	\param[out] nWidth Die Breite des gleitenden Durchschnitts.
 *
 *	\author Stephan Allgeier
 */
template<typename T> void CArrayUtilities::MovingAverage(T* pArray, size_t nSize, size_t nWidth)
{
	size_t nHalfWidth = nWidth / 2;

	// Check for odd-sized width
	assert(nWidth == (2 * nHalfWidth + 1));

	double fSum = 0.0;

	// If the array is smaller than the moving average width the whole array
	// is set to its mean value. This is a little more restrictive than
	// necessary as this behavior would only be required with array sizes
	// less than or equal to half the width. Change if required!
	if (nSize < nWidth)
	{
		for (size_t i = 0; i < nSize; i++)
		{
			fSum += pArray[i];
		}
		fSum /= nSize;
		for (size_t i = 0; i < nSize; i++)
		{
			pArray[i] = static_cast<T>(fSum);
		}
		return;
	}

	T* pTemp = new T[nSize];

	size_t nCount = 0;

	// Initialize the sum
	for (size_t i = 0; i < nHalfWidth; i++)
	{
		nCount++;
		fSum += pArray[i];
	}

	// The first part handles the left section, before the full width is used
	for (size_t i = 0; i < nHalfWidth; i++)
	{
		nCount++;
		fSum += pArray[i + nHalfWidth];

		assert(nCount == nHalfWidth + i + 1);
		pTemp[i] = static_cast<T>(fSum / nCount);
	}

	// The second part handles the center section, where the full width is used
	for (size_t i = nHalfWidth; i < nSize - nHalfWidth; i++)
	{
		nCount++;
		fSum += pArray[i + nHalfWidth];

		assert(nCount == nWidth);
		pTemp[i] = static_cast<T>(fSum / nCount);

		fSum -= pArray[i - nHalfWidth];
		nCount--;
	}

	// The third part handles the right section, after the full width is used
	for (size_t i = nSize - nHalfWidth; i < nSize; i++)
	{
		assert(nCount == nHalfWidth + nSize - i);
		pTemp[i] = static_cast<T>(fSum / nCount);

		fSum -= pArray[i - nHalfWidth];
		nCount--;
	}

	CopyArray(pTemp, pArray, nSize);

	delete[] pTemp;
}

// Yes there definitly is a nicer way of implementing this
namespace std
{
	template<typename T> void append(vector<T>& Where, const vector<T>& WhatToAppend)
	{
		Where.insert(Where.end(), WhatToAppend.begin(), WhatToAppend.end());
	};
	template<typename T> void append(list<T>& Where, const list<T>& WhatToAppend)
	{
		Where.insert(Where.end(), WhatToAppend.begin(), WhatToAppend.end());
	};
	template<typename T> void append(list<T>& Where, const vector<T>& WhatToAppend)
	{
		Where.insert(Where.end(), WhatToAppend.begin(), WhatToAppend.end());
	};
	template<typename T> void append(vector<T>& Where, const list<T>& WhatToAppend)
	{
		Where.insert(Where.end(), WhatToAppend.begin(), WhatToAppend.end());
	};
};

template<typename T> static std::vector<T> CArrayUtilities::FilterByIndex(const std::vector<T>& Input, const std::list<size_t>& Indices)
{
	// This could be done with an std::copy_if
	std::vector<T> Out;
	for (const auto& Index : Indices)
		Out.push_back(Input[Index]);
	return Out;
}

template <typename T>
std::vector<T*> CArrayUtilities::SmartToRaw(const std::vector<std::shared_ptr<T>>& Input)
{
	std::vector<T*> out;

	for (const auto& ptr : Input)
		out.push_back(ptr.get());
	return out;
}

template <typename T>
std::vector<T*> CArrayUtilities::SmartToRaw(const std::vector<std::unique_ptr<T>>& Input)
{
	std::vector<T*> out;

	for (const auto& ptr : Input)
		out.push_back(ptr.get());
	return out;
}

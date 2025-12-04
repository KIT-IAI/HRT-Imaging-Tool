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
#include "AbstractVector.h"

#include "Compare.h"
#include "FileUtilities.h"



CAbstractVector::CAbstractVector(void)
{
	m_nSize = 0;
	m_nVectorType = eAbstract;
}
CAbstractVector::~CAbstractVector(void)
{
}

void CAbstractVector::Copy(const CAbstractVector& av)
{
	FreeVector();
	AllocVector(av.Size());

	for (size_t i = 0; i < Size(); i++)
	{
		SetValueAt(i, av.GetValueAt(i));
	}
}

std::vector<double> CAbstractVector::GetValuesAsArray() const
{
	std::vector<double> values(Size());
	for (size_t i = 0; i < Size(); i++)
	{
		values[i] = GetValueAt(i);
	}
	return values;
}


CAbstractVector& CAbstractVector::operator	= (const CAbstractVector& v)
{
	Copy(v);
	return *this;
}
bool CAbstractVector::operator	== (const CAbstractVector& v) const
{
	return IsEqual(v, 0);
}
bool CAbstractVector::operator	!= (const CAbstractVector& v) const
{
	return !IsEqual(v, 0);
}

CAbstractVector& CAbstractVector::operator+=(const CAbstractVector& v)
{
	assert(v.Size() == Size());
	for (size_t i = 0; i < v.Size(); i++)
	{
		SetValueAt(i, GetValueAt(i) + v.GetValueAt(i));
	}
	return *this;
}

CAbstractVector& CAbstractVector::operator-=(const CAbstractVector& v)
{
	assert(v.Size() == Size());
	for (size_t i = 0; i < v.Size(); i++)
	{
		SetValueAt(i, GetValueAt(i) - v.GetValueAt(i));
	}
	return *this;
}

CAbstractVector& CAbstractVector::operator+=(double s)
{
	for (size_t i = 0; i < Size(); i++)
	{
		SetValueAt(i, GetValueAt(i) + s);
	}
	return *this;
}

CAbstractVector& CAbstractVector::operator-=(double s)
{
	for (size_t i = 0; i < Size(); i++)
	{
		SetValueAt(i, GetValueAt(i) - s);
	}
	return *this;
}

CAbstractVector& CAbstractVector::operator*=(double s)
{
	for (size_t i = 0; i < Size(); i++)
	{
		SetValueAt(i, GetValueAt(i) * s);
	}
	return *this;
}

CAbstractVector& CAbstractVector::operator/=(double s)
{
	for (size_t i = 0; i < Size(); i++)
	{
		SetValueAt(i, GetValueAt(i) / s);
	}
	return *this;
}



void CAbstractVector::Fill(double s)
{
	for (size_t i = 0; i < Size(); i++)
	{
		SetValueAt(i, s);
	}
}

/**	\brief Berechnet die Summe aller Vektorelemente.
 *
 *	\return Die Summe aller Vektorelemente.
 */
double CAbstractVector::Sum() const
{
	double sum = 0;
	for (size_t i = 0; i < Size(); i++)
	{
		sum += GetValueAt(i);
	}
	return sum;
}

/**	\brief Berechnet den Mittelwert aller Vektorelemente.
 *
 *	\return Den Mittelwert aller Vektorelemente.
 */
double CAbstractVector::Mean() const
{
	if (Size() == 0)
	{
		return 0.0;
	}


	return (Sum() / Size());
}

/**	\brief Sucht das kleinste Vektorelement.
 *
 *	\return Das kleinste Vektorelement.
 *
 *	\see Min(size_t &)
 *	\see Max()
 *	\see MinAbs()
 */
double CAbstractVector::Min() const
{
	size_t minIndex = 0;
	return Min(minIndex);
}

/**	\brief Sucht das größte Vektorelement.
 *
 *	\return Das größte Vektorelement.
 *
 *	\see Max(size_t &)
 *	\see Min()
 *	\see MaxAbs()
 */
double CAbstractVector::Max() const
{
	size_t maxIndex = 0;
	return Max(maxIndex);
}

std::tuple<double, double> CAbstractVector::MinMax() const
{
	assert(Size() > 0);

	auto minVal = GetValueAt(0);
	auto maxVal = GetValueAt(0);

	for (size_t i = 1; i < Size(); i++)
	{
		auto val = GetValueAt(i);
		if (val < minVal)
			minVal = val;
		if (val > maxVal)
			maxVal = val;
	}
	return std::make_tuple(minVal, maxVal);
}


/**	\brief Sucht das kleinste Vektorelement.
 *
 *	\return Das kleinste Vektorelement.
 *
 *	\param[out] pos Enthält nach dem Methodenaufruf den Index des kleinsten
 *		Vektorelements.
 *
 *	\see Min()
 *	\see Max(size_t &)
 *	\see MinAbs(size_t &)
 */
double CAbstractVector::Min(size_t& pos) const
{
	assert(Size() > 0);

	double min = GetValueAt(0);
	pos = 0;
	for (size_t i = 1; i < Size(); i++)
	{
		double value = GetValueAt(i);
		if (value < min)
		{
			min = value;
			pos = i;
		}
	}

	return min;
}

/**	\brief Sucht das größte Vektorelement.
 *
 *	\return Das größte Vektorelement.
 *
 *	\param[out] pos Enthält nach dem Methodenaufruf den Index des größten
 *		Vektorelements.
 *
 *	\see Max()
 *	\see Min(size_t &)
 *	\see MaxAbs(size_t &)
 */
double CAbstractVector::Max(size_t& pos) const
{
	assert(Size() > 0);

	double max = GetValueAt(0);
	pos = 0;
	for (size_t i = 1; i < Size(); i++)
	{
		double value = GetValueAt(i);
		if (value > max)
		{
			max = value;
			pos = i;
		}
	}

	return max;
}

/**	\brief Sucht das betragsmäßig kleinste Vektorelement.
 *
 *	\return Das betragsmäßig kleinste Vektorelement.
 *
 *	\see MinAbs(size_t &)
 *	\see MaxAbs()
 *	\see Min()
 */
double CAbstractVector::MinAbs() const
{
	size_t minIndex = 0;
	return MinAbs(minIndex);
}

/**	\brief Sucht das betragsmäßig größte Vektorelement.
 *
 *	\return Das betragsmäßig größte Vektorelement.
 *
 *	\see MaxAbs(size_t &)
 *	\see MinAbs()
 *	\see Max()
 */
double CAbstractVector::MaxAbs() const
{
	size_t maxIndex = 0;
	return MaxAbs(maxIndex);
}

/**	\brief Sucht das betragsmäßig kleinste Vektorelement.
 *
 *	\return Das betragsmäßig kleinste Vektorelement.
 *
 *	\param[out] pos Enthält nach dem Methodenaufruf den Index des
 *		betragsmäßig kleinsten Vektorelements.
 *
 *	\see MinAbs()
 *	\see MaxAbs(size_t &)
 *	\see Min(size_t &)
 */
double CAbstractVector::MinAbs(size_t& pos) const
{
	assert(Size() > 0);

	double min = fabs(GetValueAt(0));
	pos = 0;
	for (size_t i = 1; i < Size(); i++)
	{
		double value = fabs(GetValueAt(i));
		if (value < min)
		{
			min = value;
			pos = i;
		}
	}

	return min;
}

/**	\brief Sucht das betragsmäßig größte Vektorelement.
 *
 *	\return Das betragsmäßig größte Vektorelement.
 *
 *	\param[out] pos Enthält nach dem Methodenaufruf den Index des
 *		betragsmäßig größten Vektorelements.
 *
 *	\see MaxAbs()
 *	\see MinAbs(size_t &)
 *	\see Max(size_t &)
 */
double CAbstractVector::MaxAbs(size_t& pos) const
{
	assert(Size() > 0);

	double max = fabs(GetValueAt(0));
	pos = 0;
	for (size_t i = 1; i < Size(); i++)
	{
		double value = fabs(GetValueAt(i));
		if (value > max)
		{
			max = value;
			pos = i;
		}
	}

	return max;
}


bool CAbstractVector::IsEqual(const CAbstractVector& v, int nPrecision) const
{
	if (v.Size() != Size())
	{
		return false;
	}

	if (nPrecision == 0)
	{
		return IsEqual(v, 0.0);
	}

	double fEpsilon = 1.0;

	for (int i = 0; i <= nPrecision; i++)
	{
		fEpsilon *= 0.1;
	}

	return IsEqual(v, fEpsilon);
}
bool CAbstractVector::IsEqual(const CAbstractVector& v, double fEpsilon) const
{
	if (v.Size() != Size())
	{
		return false;
	}

	for (size_t i = 0; i < Size(); i++)
	{
		if (!CCompare::CompareDoubles(GetValueAt(i), v.GetValueAt(i), fEpsilon))
		{
			return false;
		}
	}

	return true;
}
bool CAbstractVector::IsNull() const
{
	return (Size() == 0);
}
bool CAbstractVector::IsZero() const
{
	for (size_t i = 0; i < Size(); i++)
	{
		if (GetValueAt(i) != 0)
			return false;
	}
	return true;
}
size_t	CAbstractVector::Size() const
{
	return m_nSize;
}
/*
*  Returns the number of Cells that are NOT Zero.
*/
size_t CAbstractVector::NonZeroSize() const
{
	size_t size = 0;
	for (size_t iterator = 0; iterator < Size(); iterator++)
	{
		if (GetValueAt(iterator) != 0) size++;
	}
	return size;
}
int CAbstractVector::GetVectorType() const
{
	return m_nVectorType;
}

/**	\brief Berechnet eine punktweise Multiplikation zweier CDenseVectoren.
 *
 *	Das Ergebnis der punktweisen Multiplikation wird im Objekt des Aufrufes
 *	gespeichert.
 *
 *	\return \c true, wenn die Berechnung ausgeführt wurde.
 *	\return \c false, wenn die Vektoren nicht die gleiche Größe haben.
 *
 *	\param[in] vector1 1. Eingabe-CDenseVector.
 *	\param[in] vector2 2. Eingabe-CDenseVector.
 */
bool CAbstractVector::PointwiseMultiplication(const CAbstractVector& vector1, const CAbstractVector& vector2)
{
	if (vector1.Size() != vector2.Size() || vector1.Size() != this->Size())
	{
		return false;
	}

	for (size_t i = 0; i < vector1.Size(); i++)
	{
		SetValueAt(i, vector1.GetValueAt(i) * vector2.GetValueAt(i));
	}

	return true;
}

bool CAbstractVector::PointwiseDivision(const CAbstractVector& vector1, const CAbstractVector& vector2)
{
	if (vector1.Size() != vector2.Size() || vector1.Size() != this->Size())
	{
		return false;
	}

	for (size_t i = 0; i < vector1.Size(); i++)
	{
		SetValueAt(i, vector1.GetValueAt(i) / vector2.GetValueAt(i));
	}

	return true;
}


void CAbstractVector::ApplyFilter(const CAbstractVector& source, const std::vector<double>& filter, int filterHotSpot/* = -1*/)
{
	assert(Size() == source.Size());

	if (filterHotSpot < 0)
		filterHotSpot = static_cast<int>(filter.size() / 2);

	assert(static_cast<size_t>(filterHotSpot) < filter.size() && filterHotSpot >= 0);

	size_t completeSourceSize = source.Size() + filter.size() - 1;
	std::vector<double> sourceValues(completeSourceSize);

	// Copies the Values from the Vector into the array
	// Prolonging the first and last value of the array as long as the filter needs it
	for (size_t i = 0; i < completeSourceSize; i++)
	{
		if (i < size_t(filterHotSpot))
			sourceValues[i] = source.GetValueAt(0);
		else if (i >= source.Size() + filterHotSpot)
			sourceValues[i] = source.GetValueAt(source.Size() - 1);
		else
			sourceValues[i] = source.GetValueAt(i - filterHotSpot);
	}


	// Applies the filter
	for (size_t i = 0; i < Size(); i++)
	{
		double newValue = 0;

		for (size_t fi = 0; fi < filter.size(); fi++)
		{
			newValue += sourceValues[i + fi] * filter[fi];
		}
		SetValueAt(i, newValue);
	}
}
void CAbstractVector::ApplyFilter(const CAbstractVector& source, const CAbstractVector& filter, int filterHotSpot/* = -1*/)
{
	auto filterValues = filter.GetValuesAsArray();

	return ApplyFilter(source, filterValues, filterHotSpot);
}
void CAbstractVector::SmoothVector(size_t nTimes)
{
	std::vector<double> filter = { 0.25, 0.5, 0.25 };

	for (size_t i = 0; i < nTimes; i++)
		ApplyFilter(*this, filter);
}

/**	\brief Schreibt den Vektor in einen Zeichenkette.
 *
 *	Die Vektorelemente werden in ihrer Darstellung als Dezimalzahl mit
 *	\a nPrecision Nachkommastellen ausgegeben. Vektorelemente werden durch
 *	Tabulatorzeichen (<tt>L'\\t'</tt>) voneinander getrennt.
 *
 *	\return Die generierte Zeichenkette.
 *
 *	\param[in] nPrecision Die Anzahl der Nachkommastellen für die
 *		Vektorelemente.
 *
 *	\see ToString(const std::wstring &, int)
 */
std::wstring CAbstractVector::ToString(int nPrecision) const
{
	assert(nPrecision >= 0);
	return ToString(L"\t", nPrecision);
}

/**	\brief Schreibt den Vektor in einen Zeichenkette.
 *
 *	Die Vektorelemente werden in ihrer Darstellung als Dezimalzahl mit
 *	\a nPrecision Nachkommastellen ausgegeben. Vektorelemente werden durch
 *	die Zeichenkette \a sDelimiter voneinander getrennt.
 *
 *	\return Die generierte Zeichenkette.
 *
 *	\param[in] sDelimiter Die Zeichenkette zur Trennung der einzelnen
 *		Vektorelemente.
 *	\param[in] nPrecision Die Anzahl der Nachkommastellen für die
 *		Vektorelemente.
 *
 *	\see ToString(int)
 */
std::wstring CAbstractVector::ToString(const std::wstring& sDelimiter, int nPrecision) const
{
	std::wostringstream buffer;
	buffer.precision(nPrecision);
	buffer << std::fixed;

	for (size_t index = 0; index < Size() - 1; index++)
	{
		buffer << GetValueAt(index) << sDelimiter;
	}
	buffer << GetValueAt(Size() - 1);

	return buffer.str();
}

/**	\brief Importiert den Vektor aus einer Textdatei.
 *
 *	Der Vektor muss vor dem Dateiimport mit der korrekten Größe initialisiert
 *	werden; andernfalls bricht die Methode mit einem Fehler ab.
 *
 *	Beim Dateiimport wird die Datei zunächst vollständig in eine Zeichenkette
 *	eingelesen (und sofort wieder geschlossen). Die Zeichenkette wird in
 *	einzelne Elemente zerlegt. Als Trennzeichen zwischen zwei Elementen
 *	werden Zeilenumbrüche (<tt>L'\\n'</tt>), Leerzeichen oder Tabulatorzeichen
 *	(<tt>L'\\t'</tt>) erwartet. Mehrere direkt aufeinander folgende
 *	Trennzeichen werden dabei wie ein einziges behandelt. Auf diese Weise
 *	können sowohl als Zeilenvektor wie auch als Spaltenvektor gespeicherte
 *	Vektoren importiert werden.
 *
 *	\return \c true, wenn der Dateiimport erfolgreich durchgeführt wird.
 *	\return \c false, wenn beim Dateiimport ein Fehler auftritt.
 *
 *	\param[in] sFilePath Der komplette Dateipfad der Textdatei.
 *
 *	\see WriteVector()
 *	\see ReadMatrix()
 */
bool CAbstractVector::ReadVector(const std::wstring& sFilePath)
{
	typedef std::wstring string_t;
	typedef boost::char_separator<string_t::value_type> tokenizerfunc_t;
	typedef boost::tokenizer<tokenizerfunc_t, string_t::const_iterator, string_t> tokenizer_t;

	tokenizerfunc_t delimiters(L" \t\n");

	size_t index = 0;

	double number;

	std::wifstream file;
	std::wstring line;

	file.open(std::filesystem::path(sFilePath));

	if (!file.good())
	{
		// File inaccessible
		return false;
	}

	try
	{
		std::getline(file, line, L'\0');

		tokenizer_t tokens(line, delimiters);
		for (auto token : tokens)
		{
			number = std::stod(token);

			if ((index >= Size()) || (number == HUGE_VAL) || (number == -HUGE_VAL))
			{
				// Too many entries in file, or invalid numeric entry
				return false;
			}

			SetValueAt(index, number);
			index++;
		}

		if (index != Size())
		{
			// Too few entries in file
			return false;
		}
	}
	catch (const std::exception&)
	{
		// Non-numeric entry
		return false;
	}

	return true;
}
bool CAbstractVector::AllocateAndReadVector(const std::wstring& sFilePath)
{
	auto nColCount = CFileUtilities::CountNumberOfColumns(sFilePath);
	auto nRowCount = CFileUtilities::CountNumberOfLines(sFilePath, true);
	AllocVector(nColCount * nRowCount);
	return ReadVector(sFilePath);
}

/**	\brief Exportiert den Vektor in eine Textdatei.
*
*	Die Vektorelemente werden in ihrer Darstellung als Dezimalzahl mit
*	\a nPrecision Nachkommastellen ausgegeben. Die Vektorelemente werden
*	jeweils durch Zeilenumbrüche (<tt>L'\\n'</tt>) voneinander getrennt.
*
*	\return \c true, wenn der Dateiexport erfolgreich durchgeführt wird.
*	\return \c false, wenn beim Dateiexport ein Fehler auftritt.
*
*	\param[in] sFilePath Der komplette Dateipfad der Textdatei.
*	\param[in] nPrecision Die Anzahl der Nachkommastellen für die
*		Vektorelemente.
*
*	\see WriteVector(const std::wstring &, const std::wstring &, int)
*	\see ReadVector()
*	\see WriteMatrix()
*/
bool CAbstractVector::WriteVector(const std::wstring& sFilePath, int nPrecision) const
{
	assert(nPrecision >= 0);
	return WriteVector(sFilePath, L"\n", nPrecision);
}

/**	\brief Exportiert den Vektor in eine Textdatei.
 *
 *	Die Vektorelemente werden in ihrer Darstellung als Dezimalzahl mit
 *	\a nPrecision Nachkommastellen ausgegeben. Die Vektorelemente werden
 *	jeweils durch die Zeichenkette \a sDelimiter voneinander getrennt.
 *
 *	\return \c true, wenn der Dateiexport erfolgreich durchgeführt wird.
 *	\return \c false, wenn beim Dateiexport ein Fehler auftritt.
 *
 *	\param[in] sFilePath Der komplette Dateipfad der Textdatei.
 *	\param[in] sDelimiter Die Zeichenkette zur Trennung der einzelnen
 *		Vektorelemente.
 *	\param[in] nPrecision Die Anzahl der Nachkommastellen für die
 *		Vektorelemente.
 *
 *	\see WriteVector(const std::wstring &, int)
 *	\see ReadVector()
 *	\see WriteMatrix()
 */
bool CAbstractVector::WriteVector(const std::wstring& sFilePath, const std::wstring& sDelimiter, int nPrecision) const
{
	assert(nPrecision >= 0);

	std::wofstream file;

	file.open(std::filesystem::path(sFilePath));

	if (!file.good())
	{
		// File inaccessible
		return false;
	}

	try
	{
		file.precision(nPrecision);
		file << std::fixed;

		for (const auto& val : *this)
		{
			file << val << sDelimiter;
		}
	}
	catch (const std::exception&)
	{
		return false;
	}

	return true;
}

CAbstractVector::iterator CAbstractVector::begin() const
{
	return iterator(*this, 0);
}
CAbstractVector::iterator CAbstractVector::end() const
{
	return iterator(*this, Size());
}
CAbstractVector::iterator::iterator(const CAbstractVector& Vector, size_t nIndex)
	:Vector(Vector)
	, nIndex(nIndex)
{
}

double CAbstractVector::iterator::operator*() const
{
	return Vector.GetValueAt(nIndex);
}

CAbstractVector::iterator& CAbstractVector::iterator::operator++()
{
	nIndex++;
	return *this;
}

CAbstractVector::iterator& CAbstractVector::iterator::operator++(int)
{
	nIndex++;
	return *this;
}

bool CAbstractVector::iterator::operator!=(const iterator& rhs) const
{
	return nIndex != rhs.nIndex;
}

CAbstractVector::iterator& CAbstractVector::iterator::operator+(int i)
{
	nIndex += i;
	nIndex = std::min(nIndex, Vector.Size());
	return *this;
}

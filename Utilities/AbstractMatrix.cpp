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
#include "AbstractMatrix.h"

#include <fstream>
#include <sstream>

#include <boost/tokenizer.hpp>

#include "FileUtilities.h"



CAbstractMatrix::CAbstractMatrix(void)
{
	m_nCols = 0;
	m_nRows = 0;
	m_nMatrixType = eAbstract;
}
CAbstractMatrix::~CAbstractMatrix(void)
{
}

int CAbstractMatrix::GetMatrixType() const
{
	return m_nMatrixType;
}

/*
*
* Copies every CDenseMatrix into every other CDenseMatrix by iterating over all elements. But that is really slow
*
*/
void CAbstractMatrix::Copy(const CAbstractMatrix& m)
{
	assert(((Cols() == m.Cols()) && (Rows() == m.Rows())) || ((Cols() == 0) && (Rows() == 0)));

	if (Rows() == 0 && Cols() == 0)
		AllocMatrix(m.Rows(), m.Cols());

	for (size_t r = 0; r < Rows(); r++)
	{
		for (size_t c = 0; c < Cols(); c++)
		{
			SetValueAt(r, c, GetValueAt(r, c));
		}
	}
}
CAbstractMatrix& CAbstractMatrix::operator=(const CAbstractMatrix& m)
{
	Copy(m);
	return *this;
}


double CAbstractMatrix::GetValueAt(size_t row, size_t col) const
{
	return GetVectorAt(row).GetValueAt(col);
}
void CAbstractMatrix::SetValueAt(size_t row, size_t col, double value)
{
	GetVectorAt(row).SetValueAt(col, value);
}

CAbstractMatrix& CAbstractMatrix::operator+=(const CAbstractMatrix& m)
{
	assert(m.m_nRows == m_nRows && m.m_nCols == m_nCols);
	for (size_t r = 0; r < m.m_nRows; r++)
	{
		GetVectorAt(r) += m.GetVectorAt(r);
	}
	return *this;
}
CAbstractMatrix& CAbstractMatrix::operator-=(const CAbstractMatrix& m)
{
	assert(m.m_nRows == m_nRows && m.m_nCols == m_nCols);
	for (size_t r = 0; r < m.m_nRows; r++)
	{
		GetVectorAt(r) -= m.GetVectorAt(r);
	}
	return *this;
}

CAbstractMatrix& CAbstractMatrix::operator+=(double s)
{
	for (size_t r = 0; r < m_nRows; r++)
	{
		GetVectorAt(r) += s;
	}
	return *this;
}
CAbstractMatrix& CAbstractMatrix::operator-=(double s)
{
	for (size_t r = 0; r < m_nRows; r++)
	{
		GetVectorAt(r) -= s;
	}
	return *this;
}
CAbstractMatrix& CAbstractMatrix::operator*=(double s)
{
	for (size_t r = 0; r < m_nRows; r++)
	{
		GetVectorAt(r) *= s;
	}
	return *this;
}
CAbstractMatrix& CAbstractMatrix::operator/=(double s)
{
	assert(s != 0.0);
	for (size_t r = 0; r < m_nRows; r++)
	{
		GetVectorAt(r) /= s;
	}
	return *this;
}



bool CAbstractMatrix::operator==(const CAbstractMatrix& m) const
{
	return IsEqual(m, 0.0);
}

bool CAbstractMatrix::operator!=(const CAbstractMatrix& m) const
{
	return !(*this == m);
}

bool CAbstractMatrix::IsEqual(const CAbstractMatrix& m, int nPrecision) const
{
	assert(nPrecision >= 0);

	if ((m.Cols() != Cols()) || (m.Rows() != Rows()))
	{
		return false;
	}

	if (nPrecision == 0)
	{
		return IsEqual(m, 0.0);
	}

	double fEpsilon = 1.0;

	for (int i = 0; i <= nPrecision; i++)
	{
		fEpsilon *= 0.1;
	}

	return IsEqual(m, fEpsilon);
}
bool CAbstractMatrix::IsEqual(const CAbstractMatrix& m, double fEpsilon) const
{
	if ((m.Cols() != Cols()) || (m.Rows() != Rows()))
	{
		return false;
	}

	for (size_t nRow = 0; nRow < Rows(); nRow++)
	{
		if (!GetVectorAt(nRow).IsEqual(m.GetVectorAt(nRow), fEpsilon))
		{
			return false;
		}
	}

	return true;
}
/**	\brief Gibt an, ob die Matrix leer ist.
 *
 *	Eine Matrix ist genau dann leer, wenn sie in mindestens einer ihrer
 *	Dimensionen keine Ausdehnung (d.h. die Größe 0) hat.
 *
 *	Diese Methode kann z.B. verwendet werden, um nach einem Aufruf der
 *	Methode Inverse() zu prüfen, ob die Ausgangsmatrix singulär war.
 *
 *	\return \c true, wenn die Matrix leer ist.
 *	\return \c false, wenn die Matrix nicht leer ist.
 */
bool CAbstractMatrix::IsNull() const
{
	return (Cols() == 0 || Rows() == 0);
}

size_t CAbstractMatrix::Rows() const
{
	return m_nRows;
}
size_t CAbstractMatrix::Cols() const
{
	return m_nCols;
}

void CAbstractMatrix::Fill(double s)
{
	assert((Rows() > 0) && (Cols() > 0));
	for (size_t r = 0; r < Rows(); r++)
	{
		GetVectorAt(r).Fill(s);
	}
}

/**	\brief Berechnet den Mittelwert aller Matrixelemente.
 *
 *	\return Den Mittelwert aller Matrixelemente.
 */
double CAbstractMatrix::Mean() const
{
	if ((Rows() == 0) || (Cols() == 0))
	{
		return 0.0;
	}

	double sum = 0;

	for (size_t r = 0; r < Rows(); r++)
	{
		for (size_t c = 0; c < Cols(); c++)
		{
			sum += GetValueAt(r, c);
		}
	}

	return (sum / (Rows() * Cols()));
}
/**	\brief Sucht das kleinste Matrixelement.
 *
 *	\return Das kleinste Matrixelement.
 *
 *	\see Min(size_t &, size_t &)
 *	\see Max()
 *	\see MinAbs()
 */
double CAbstractMatrix::Min() const
{
	size_t r = 0;
	size_t c = 0;
	return Min(r, c);
}
/**	\brief Sucht das größte Matrixelement.
 *
 *	\return Das größte Matrixelement.
 *
 *	\see Max(size_t &, size_t &)
 *	\see Min()
 *	\see MaxAbs()
 */
double CAbstractMatrix::Max() const
{
	size_t r = 0;
	size_t c = 0;
	return Max(r, c);
}
/**	\brief Sucht das kleinste Matrixelement.
 *
 *	\return Das kleinste Matrixelement.
 *
 *	\param[out] row Enthält nach dem Methodenaufruf den Zeilenindex des
 *		kleinsten Matrixelements.
 *	\param[out] col Enthält nach dem Methodenaufruf den Spaltenindex des
 *		kleinsten Matrixelements.
 *
 *	\see Min()
 *	\see Max(size_t &, size_t &)
 *	\see MinAbs(size_t &, size_t &)
 */
double CAbstractMatrix::Min(size_t& row, size_t& col) const
{
	assert((Rows() > 0) && (Cols() > 0));

	double min = GetValueAt(0, 0);
	row = 0;
	col = 0;
	for (size_t r = 0; r < Rows(); r++)
	{
		for (size_t c = 0; c < Cols(); c++)
		{
			double value = GetValueAt(r, c);
			if (value < min)
			{
				min = value;
				row = r;
				col = c;
			}
		}
	}

	return min;
}
/**	\brief Sucht das größte Matrixelement.
 *
 *	\return Das größte Matrixelement.
 *
 *	\param[out] row Enthält nach dem Methodenaufruf den Zeilenindex des
 *		größten Matrixelements.
 *	\param[out] col Enthält nach dem Methodenaufruf den Spaltenindex des
 *		größten Matrixelements.
 *
 *	\see Max()
 *	\see Min(size_t &, size_t &)
 *	\see MaxAbs(size_t &, size_t &)
 */
double CAbstractMatrix::Max(size_t& row, size_t& col) const
{
	assert((Rows() > 0) && (Cols() > 0));

	double max = GetValueAt(0, 0);
	row = 0;
	col = 0;
	for (size_t r = 0; r < Rows(); r++)
	{
		for (size_t c = 0; c < Cols(); c++)
		{
			double value = GetValueAt(r, c);
			if (value > max)
			{
				max = value;
				row = r;
				col = c;
			}
		}
	}

	return max;
}
/**	\brief Sucht das betragsmäßig kleinste Matrixelement.
 *
 *	\return Das betragsmäßig kleinste Matrixelement.
 *
 *	\see MinAbs(size_t &, size_t &)
 *	\see MaxAbs()
 *	\see Min()
 */
double CAbstractMatrix::MinAbs() const
{
	size_t r = 0;
	size_t c = 0;
	return MinAbs(r, c);
}
/**	\brief Sucht das betragsmäßig größte Matrixelement.
 *
 *	\return Das betragsmäßig größte Matrixelement.
 *
 *	\see MaxAbs(size_t &, size_t &)
 *	\see MinAbs()
 *	\see Max()
 */
double CAbstractMatrix::MaxAbs() const
{
	size_t r = 0;
	size_t c = 0;
	return MaxAbs(r, c);
}
/**	\brief Sucht das betragsmäßig kleinste Matrixelement.
 *
 *	\return Das betragsmäßig kleinste Matrixelement.
 *
 *	\param[out] row Enthält nach dem Methodenaufruf den Zeilenindex des
 *		betragsmäßig kleinsten Matrixelements.
 *	\param[out] col Enthält nach dem Methodenaufruf den Spaltenindex des
 *		betragsmäßig kleinsten Matrixelements.
 *
 *	\see MinAbs()
 *	\see MaxAbs(size_t &, size_t &)
 *	\see Min(size_t &, size_t &)
 */
double CAbstractMatrix::MinAbs(size_t& row, size_t& col) const
{
	assert((Rows() > 0) && (Cols() > 0));

	double min = fabs(GetValueAt(0, 0));
	row = 0;
	col = 0;
	for (size_t r = 0; r < Rows(); r++)
	{
		for (size_t c = 0; c < Cols(); c++)
		{
			double value = fabs(GetValueAt(r, c));
			if (value < min)
			{
				min = value;
				row = r;
				col = c;
			}
		}
	}

	return min;
}
/**	\brief Sucht das betragsmäßig größte Matrixelement.
 *
 *	\return Das betragsmäßig größte Matrixelement.
 *
 *	\param[out] row Enthält nach dem Methodenaufruf den Zeilenindex des
 *		betragsmäßig größten Matrixelements.
 *	\param[out] col Enthält nach dem Methodenaufruf den Spaltenindex des
 *		betragsmäßig größten Matrixelements.
 *
 *	\see MaxAbs()
 *	\see MinAbs(size_t &, size_t &)
 *	\see Max(size_t &, size_t &)
 */
double CAbstractMatrix::MaxAbs(size_t& row, size_t& col) const
{
	assert((Rows() > 0) && (Cols() > 0));

	double max = fabs(GetValueAt(0, 0));
	row = 0;
	col = 0;
	for (size_t r = 0; r < Rows(); r++)
	{
		for (size_t c = 0; c < Cols(); c++)
		{
			double value = fabs(GetValueAt(r, c));
			if (value > max)
			{
				max = value;
				row = r;
				col = c;
			}
		}
	}

	return max;
}

/**	\brief Sucht das absolut kleinste und das absolut größte Element in einem Durchlauf.
*
*	\return
*
*	\param[out] rowMin Enthält nach dem Methodenaufruf den Zeilenindex des
*		kleinsten Matrixelements.
*	\param[out] colMin Enthält nach dem Methodenaufruf den Spaltenindex des
*		kleinsten Matrixelements.
*	\param[out] rowMax Enthält nach dem Methodenaufruf den Zeilenindex des
*		größten Matrixelements.
*	\param[out] colMax Enthält nach dem Methodenaufruf den Spaltenindex des
*		größten Matrixelements.
*
*	\see MaxAbs()
*	\see MinAbs(size_t &, size_t &)
*	\see Max(size_t &, size_t &)
*/
void CAbstractMatrix::MinMax(size_t& rowMin, size_t& colMin, size_t& rowMax, size_t& colMax) const
{
	assert((Rows() > 0) && (Cols() > 0));

	double max = GetValueAt(0, 0);
	double min = GetValueAt(0, 0);
	rowMin = 0;
	colMin = 0;
	rowMax = 0;
	colMax = 0;
	for (size_t r = 0; r < Rows(); r++)
	{
		for (size_t c = 0; c < Cols(); c++)
		{
			double value = GetValueAt(r, c);
			if (value > max)
			{
				max = value;
				rowMax = r;
				colMax = c;
			}
			else if (value < min)
			{
				min = value;
				rowMin = r;
				colMin = c;
			}
		}
	}
}



/**	\brief Berechnet eine punktweise Multiplikation zweier Matrizen.
 *
 *	Das Ergebnis der punktweisen Multiplikation wird im Objekt des Aufrufes
 *	gespeichert.
 *
 *	\return \c true, wenn die Berechnung ausgeführt wurde.
 *	\return \c false, wenn die Matrizen unterschiedliche Größen haben.
 *
 *	\param[in] mat1,mat2 Die Eingabematrizen.
 */
bool CAbstractMatrix::PointwiseMultplication(const CAbstractMatrix& mat1, const CAbstractMatrix& mat2)
{
	if ((mat1.Rows() != mat2.Rows()) ||
		(mat1.Cols() != mat2.Cols()) ||
		(Rows() != mat2.Rows()) ||
		(Cols() != mat2.Cols()))
	{
		return false;
	}

	for (size_t i = 0; i < mat1.Rows(); i++)
	{
		for (size_t j = 0; j < mat1.Cols(); j++)
		{
			SetValueAt(i, j, mat1.GetValueAt(i, j) * mat2.GetValueAt(i, j));
		}
	}

	return true;
}

/*
*
* Reduces a matrix by given factors in X and Y directions.
* The destinationmatrix must be allocated properly
*
*/
bool CAbstractMatrix::ReduceMatrix(const CAbstractMatrix& source, size_t reductionRow, size_t reductionCol, bool doInterpolation/* = false */)
{
	assert(!source.IsNull());
	assert(reductionCol >= 1 && reductionRow >= 1);
	assert(Rows() >= source.Rows() / reductionRow && Cols() >= source.Cols() / reductionCol);	// Destination too small

	for (size_t row = 0; row < Rows(); row++)
	{
		for (size_t col = 0; col < Cols(); col++)
		{
			double reducedValue = 0;

			if (doInterpolation)
			{
				size_t actRow = row * reductionRow;
				size_t actCol = col * reductionCol;

				for (size_t interpolationRow = actRow; interpolationRow < actRow + reductionRow; interpolationRow++)
				{
					for (size_t interpolationCol = actCol; interpolationCol < actCol + reductionCol; interpolationCol++)
					{
						reducedValue += source.GetValueAt(interpolationRow, interpolationCol);
					}
				}
				reducedValue /= reductionRow * reductionCol;
			}
			else
			{
				reducedValue = source.GetValueAt(row * reductionRow, col * reductionCol);
			}

			SetValueAt(row, col, reducedValue);
		}
	}

	return true;
}
bool CAbstractMatrix::ProlongMatrix(const CAbstractMatrix& source, size_t prologationRow, size_t prolongationCol)
{
	assert(!source.IsNull());
	assert(prologationRow >= 1 && prolongationCol >= 1);
	assert(Rows() >= source.Rows() * prologationRow && Cols() >= source.Cols() * prolongationCol);	// Destination too small

	for (size_t row = 0; row < Rows(); row++)
	{
		for (size_t col = 0; col < Cols(); col++)
		{
			SetValueAt(row, col, source.GetValueAt(row / prologationRow, col / prolongationCol));
		}
	}

	return true;
}


void CAbstractMatrix::SetCol(const CAbstractVector& v, size_t n)
{
	assert((n >= 0) && (n < Cols()));
	assert(Rows() == v.Size());

	for (size_t r = 0; r < Rows(); r++)
	{
		SetValueAt(r, n, v.GetValueAt(r));
	}
}

/*
*
* Creates a new Matrix without any rows that only contain zeros in the beginnin or end.
*
* 0 0 0 0
* 1 0 2 0		1 0 2 0
* 0 0 0 0	->	0 0 0 0
* 2 0 3 0		2 0 3 0
* 0 0 0 0
*
*/
void CAbstractMatrix::CropMatrixY(CAbstractMatrix& source)
{
	assert(source != *this);			// Sorry, but you cannot pass the source as destination, because that would cause trouble with creating an abstract object
	// It's probably possible somehow, but would cause more work to fix, than simply adding a line somewhere else...

	size_t firstNonZeroRow = 0;
	size_t lastNonZeroRow = source.Rows() - 1;

	for (firstNonZeroRow = 0; firstNonZeroRow < source.Rows(); firstNonZeroRow++)
	{
		if (!source.GetVectorAt(firstNonZeroRow).IsZero())
			break;
	}
	for (lastNonZeroRow = source.Rows(); lastNonZeroRow > 0; lastNonZeroRow--)
	{
		if (!source.GetVectorAt(lastNonZeroRow - 1).IsZero())
			break;
	}

	if (lastNonZeroRow < firstNonZeroRow) // Matrix = 0
		return FreeMatrix();

	AllocMatrix(lastNonZeroRow - firstNonZeroRow + 1, source.Cols());
	CopyRoi(0, 0, source, firstNonZeroRow, 0, Rows(), Cols());
}



/**	\brief Kopiert einen Ausschnitt einer Matrix.
 *
 *	Für das Kopieren ganzer Zeilen wird eine effizientere Methode eingesetzt.
 *	Insbesondere bei der Verwendung mit CSparseMatrix wird der Kopiervorgang
 *	dadurch signifikant beschleunigt.
 *
 *	\param[in] targetrow,targetcol Der jeweils erste Zeilen- und Spaltenindex
 *		des Ausschnitts in der Zielmatrix.
 *	\param[in] source Die Quellmatrix.
 *	\param[in] sourcerow,sourcecol Der jeweils erste Zeilen- und Spaltenindex
 *		des Ausschnitts in der Quellmatrix.
 *	\param[in] rowcount,colcount Die Anzahl der zu kopierenden Zeilen und
 *		Spalten.
 *
 *	\author Lorenzo Toso
 */
void CAbstractMatrix::CopyRoi(size_t targetrow, size_t targetcol, const CAbstractMatrix& source, size_t sourcerow, size_t sourcecol, size_t rowcount, size_t colcount)
{
	assert(targetrow + rowcount <= Rows() && targetcol + colcount <= Cols());

	if (!(targetcol == 0 && colcount == Cols() && Cols() == source.Cols()))
	{
		for (size_t i = 0; i < rowcount; i++)
		{
			for (size_t j = 0; j < colcount; j++)
			{
				if ((targetrow + i >= 0) && (targetrow + i < Rows()) &&
					(targetcol + j >= 0) && (targetcol + j < Cols()) &&
					(sourcerow + i >= 0) && (sourcerow + i < source.Rows()) &&
					(sourcecol + j >= 0) && (sourcecol + j < source.Cols()))
				{
					SetValueAt(targetrow + i, targetcol + j, source.GetValueAt(sourcerow + i, sourcecol + j));
				}
			}
		}
	}
	else // This is a more efficent way, that simply copies the vectors. It is only used if whole rows are copied
	{
		for (size_t i = 0; i < rowcount; i++)
		{
			if ((targetrow + i >= 0) && (targetrow + i < Rows()) &&
				(sourcerow + i >= 0) && (sourcerow + i < source.Rows()))
			{
				GetVectorAt(targetrow + i).Copy(source.GetVectorAt(sourcerow + i));
			}
		}
	}
}

CAbstractMatrix::iterator CAbstractMatrix::begin() const
{
	return iterator(*this, 0);
}

CAbstractMatrix::iterator CAbstractMatrix::end() const
{
	return iterator(*this, Rows());
}

/**	\brief Schreibt die Matrix in einen Zeichenkette.
*
*	Die Matrixelemente werden in ihrer Darstellung als Dezimalzahl mit
*	\a nPrecision Nachkommastellen ausgegeben. Matrixelemente werden durch
*	Tabulatorzeichen (<tt>L'\\t'</tt>) voneinander getrennt, Matrixzeilen
*	durch Zeilenumbrüche (<tt>L'\\n'</tt>).
*
*	\return Die generierte Zeichenkette.
*
*	\param[in] nPrecision Die Anzahl der Nachkommastellen für die
*		Matrixelemente.
*
*	\see ToString(const std::wstring &, const std::wstring &, int)
*/
std::wstring CAbstractMatrix::ToString(int nPrecision) const
{
	assert(nPrecision >= 0);
	return ToString(L"\t", L"\n", nPrecision);
}

/**	\brief Schreibt die Matrix in einen Zeichenkette.
*
*	Die Matrixelemente werden in ihrer Darstellung als Dezimalzahl mit
*	\a nPrecision Nachkommastellen ausgegeben. Matrixelemente werden durch
*	die Zeichenkette \a sElementDelimiter voneinander getrennt, Matrixzeilen
*	durch die Zeichenkette \a sRowDelimiter.
*
*	\return Die generierte Zeichenkette.
*
*	\param[in] sElementDelimiter Die Zeichenkette zur Trennung der einzelnen
*		Matrixelemente.
*	\param[in] sRowDelimiter Die Zeichenkette zur Trennung der Matrixzeilen.
*	\param[in] nPrecision Die Anzahl der Nachkommastellen für die
*		Matrixelemente.
*
*	\see ToString(int)
*/
std::wstring CAbstractMatrix::ToString(const std::wstring& sElementDelimiter, const std::wstring& sRowDelimiter, int nPrecision) const
{
	std::wstring sOut;
	std::wostringstream buffer;
	buffer.precision(nPrecision);
	buffer << std::fixed;

	for (size_t row = 0; row < Rows(); row++)
	{
		for (size_t col = 0; col < Cols() - 1; col++)
		{
			buffer << GetValueAt(row, col) << sElementDelimiter;
		}
		buffer << GetValueAt(row, Cols() - 1) << sRowDelimiter;
	}

	sOut = buffer.str().substr(0, buffer.str().size() - sRowDelimiter.size());

	return sOut;
}

bool CAbstractMatrix::AllocateAndReadMatrix(const std::wstring& sFilePath)
{
	auto nLineCount = CFileUtilities::CountNumberOfLines(sFilePath, true);
	auto nColumnCount = CFileUtilities::CountNumberOfColumns(sFilePath, L" \t");

	if (nLineCount <= 0 || nColumnCount <= 0)
		return false;

	AllocMatrix(nLineCount, nColumnCount);
	return ReadMatrix(sFilePath);
}

/**	\brief Importiert die Matrix aus einer Textdatei.
 *
 *	Die Matrix muss vor dem Dateiimport mit der korrekten Größe initialisiert
 *	werden; andernfalls bricht die Methode mit einem Fehler ab.
 *
 *	\return \c true, wenn der Dateiimport erfolgreich durchgeführt wird.
 *	\return \c false, wenn beim Dateiimport ein Fehler auftritt.
 *
 *	\param[in] sFilePath Der komplette Dateipfad der Textdatei.
 *
 *	\see WriteMatrix()
 *	\see ReadVector()
 */
bool CAbstractMatrix::ReadMatrix(const std::wstring& sFilePath)
{
	typedef std::wstring string_t;
	typedef boost::char_separator<string_t::value_type> tokenizerfunc_t;
	typedef boost::tokenizer<tokenizerfunc_t, string_t::const_iterator, string_t> tokenizer_t;

	tokenizerfunc_t delimiters(L" \t");

	size_t row = 0;
	size_t col = 0;

	double number;

	std::wifstream file;
	std::wstring line;

	file.open(sFilePath);

	if (!file.good())
	{
		// File inaccessible
		return false;
	}

	try
	{
		while (file.good())
		{
			std::getline(file, line);

			if (line.empty())
			{
				// Empty lines are skipped
				continue;
			}

			tokenizer_t tokens(line, delimiters);
			for (auto token : tokens)
			{
				number = std::stod(token);

				if ((row >= Rows()) || (col >= Cols()) || (number == HUGE_VAL) || (number == -HUGE_VAL))
				{
					// Too many rows in file, too many entries in current row, or invalid numeric entry
					return false;
				}

				SetValueAt(row, col, number);
				col++;
			}

			if (col != Cols())
			{
				// Too few entries in current row
				return false;
			}

			row++;
			col = 0;
		}

		if (row != Rows())
		{
			// Too few rows in file
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

/**	\brief Exportiert die Matrix in eine Textdatei.
 *
 *	Die Matrixelemente werden in ihrer Darstellung als Dezimalzahl mit
 *	\a nPrecision Nachkommastellen ausgegeben. Der Dateiexport geschieht
 *	zeilenweise, Matrixelemente werden durch Tabulatorzeichen
 *	(<tt>L'\\t'</tt>) voneinander getrennt, Matrixzeilen durch Zeilenumbrüche
 *	(<tt>L'\\n'</tt>).
 *
 *	\return \c true, wenn der Dateiexport erfolgreich durchgeführt wird.
 *	\return \c false, wenn beim Dateiexport ein Fehler auftritt.
 *
 *	\param[in] sFilePath Der komplette Dateipfad der Textdatei.
 *	\param[in] nPrecision Die Anzahl der Nachkommastellen für die
 *		Matrixelemente.
 *
 *	\warning Die Matrixdatei darf maximal 2 GB groß werden. Bei größeren
 *		Dateien bricht die Methode mit einem Fehler ab.
 *
 *	\warning Da vor dem eigentlichen Dateiexport die resultierende Dateigröße
 *		zunächst abgeschätzt wird, kann der Abbruch der Methode tatsächlich
 *		bereits bei Matrizen auftreten, die nach dem Dateiexport unterhalb
 *		des Dateigrößenlimits liegen würden. Es kann also passieren, dass
 *		eine Matrix aus einer zulässig großen Datei geladen aber anschließend
 *		nicht wieder gespeichert werden kann.
 *
 *	\see WriteMatrix(const std::wstring &, const std::wstring &, const std::wstring &, int)
 *	\see ReadMatrix()
 *	\see WriteVector()
 */
bool CAbstractMatrix::WriteMatrix(const std::wstring& sFilePath, int nPrecision) const
{
	assert(nPrecision >= 0);
	return WriteMatrix(sFilePath, L"\t", L"\n", nPrecision);
}

/**	\brief Exportiert die Matrix in eine Textdatei.
 *
 *	Die Matrixelemente werden in ihrer Darstellung als Dezimalzahl mit
 *	\a nPrecision Nachkommastellen ausgegeben. Der Dateiexport geschieht
 *	zeilenweise, Matrixelemente werden durch die Zeichenkette
 *	\a sElementDelimiter voneinander getrennt, Matrixzeilen durch die
 *	Zeichenkette \a sRowDelimiter.
 *
 *	\return \c true, wenn der Dateiexport erfolgreich durchgeführt wird.
 *	\return \c false, wenn beim Dateiexport ein Fehler auftritt.
 *
 *	\param[in] sFilePath Der komplette Dateipfad der Textdatei.
 *	\param[in] sElementDelimiter Die Zeichenkette zur Trennung der einzelnen
 *		Matrixelemente.
 *	\param[in] sRowDelimiter Die Zeichenkette zur Trennung der Matrixzeilen.
 *	\param[in] nPrecision Die Anzahl der Nachkommastellen für die
 *		Matrixelemente.
 *
 *	\warning Die Matrixdatei darf maximal 2 GB groß werden. Bei größeren
 *		Dateien bricht die Methode mit einem Fehler ab.
 *
 *	\warning Da vor dem eigentlichen Dateiexport die resultierende Dateigröße
 *		zunächst abgeschätzt wird, kann der Abbruch der Methode tatsächlich
 *		bereits bei Matrizen auftreten, die nach dem Dateiexport unterhalb
 *		des Dateigrößenlimits liegen würden. Es kann also passieren, dass
 *		eine Matrix aus einer zulässig großen Datei geladen aber anschließend
 *		nicht wieder gespeichert werden kann.
 *
 *	\see WriteMatrix(const std::wstring &, int)
 *	\see ReadMatrix()
 *	\see WriteVector()
 */
bool CAbstractMatrix::WriteMatrix(const std::wstring& sFilePath, const std::wstring& sElementDelimiter, const std::wstring& sRowDelimiter, int nPrecision) const
{
	std::wofstream file;

	file.open(sFilePath);

	if (!file.good())
	{
		// File inaccessible
		return false;
	}

	try
	{
		file.precision(nPrecision);
		file << std::fixed;

		for (size_t row = 0; row < Rows(); row++)
		{
			for (size_t col = 0; col < Cols() - 1; col++)
			{
				file << GetValueAt(row, col) << sElementDelimiter;
			}
			file << GetValueAt(row, Cols() - 1) << sRowDelimiter;
		}
	}
	catch (const std::exception&)
	{
		return false;
	}

	return true;
}
void CAbstractMatrix::CopyToCSR(std::vector<double>& val, std::vector<size_t>& colInd, std::vector<size_t>& rowPtr)
{
	auto numberNZ = GetNonZeroElementCount();
	val.resize(numberNZ);
	colInd.resize(numberNZ);
	rowPtr.resize(Rows() + 1);

	size_t val_index = 0;
	size_t row_index = 0;
	rowPtr[0] = 0;

	for (size_t row = 0; row < Rows(); row++)
	{
		row_index++;
		rowPtr[row_index] = rowPtr[row_index - 1];
		for (size_t col = 0; col < Cols(); col++)
		{
			if (auto value = GetValueAt(row, col); value != 0)
			{
				val[val_index] = value;
				colInd[val_index] = col;
				val_index++;
				rowPtr[row_index]++;
			}
		}
	}
	rowPtr[Rows()] = numberNZ;
}

//void CAbstractMatrix::CopyToCSR(std::vector<double>& val, std::vector<int>& colInd, std::vector<int>& rowPtr)
//{
//	auto numberNZ = GetNonZeroElementCount();
//	val.resize(numberNZ);
//	colInd.resize(numberNZ);
//	rowPtr.resize(Rows() + 1);
//
//	size_t val_index = 0;
//	size_t row_index = 0;
//	rowPtr[0] = 0;
//
//	for (size_t row = 0; row < Rows(); row++)
//	{
//		row_index++;
//		rowPtr[row_index] = rowPtr[row_index - 1];
//		for (size_t col = 0; col < Cols(); col++)
//		{
//			if (auto value = GetValueAt(row, col); value != 0)
//			{
//				val[val_index] = value;
//				colInd[val_index] = col;
//				val_index++;
//				rowPtr[row_index]++;
//			}
//		}
//	}
//	rowPtr[Rows()] = numberNZ;
//}

/**	\brief Exportiert die Matrix in eine Textdatei im Matrix Market Exchange Format.
 *
 *	Die Matrixelemente werden in ihrer Darstellung als Dezimalzahl mit
 *	\a nPrecision Nachkommastellen ausgegeben.
 *
 *	\return \c true, wenn der Dateiexport erfolgreich durchgeführt wird.
 *	\return \c false, wenn beim Dateiexport ein Fehler auftritt.
 *
 *	\param[in] sFilePath Der komplette Dateipfad der Textdatei.
 *	\param[in] nPrecision Die Anzahl der Nachkommastellen für die
 *		Matrixelemente.
 *
 *	\warning Die Matrixdatei darf maximal 2 GB groß werden. Bei größeren
 *		Dateien bricht die Methode mit einem Fehler ab.
 *
 *	\warning Da vor dem eigentlichen Dateiexport die resultierende Dateigröße
 *		zunächst abgeschätzt wird, kann der Abbruch der Methode tatsächlich
 *		bereits bei Matrizen auftreten, die nach dem Dateiexport unterhalb
 *		des Dateigrößenlimits liegen würden. Es kann also passieren, dass
 *		eine Matrix aus einer zulässig großen Datei geladen aber anschließend
 *		nicht wieder gespeichert werden kann.
 *
 *	\see WriteMatrix(const std::wstring &, int)
 *	\see ReadMatrix()
 *	\see WriteVector()
 */
bool CAbstractMatrix::WriteMatrixMM(const std::wstring& sFilePath, int nPrecision) const
{
	std::wofstream file;

	file.open(sFilePath);

	if (!file.good())
	{
		// File inaccessible
		return false;
	}

	try
	{
		file.precision(nPrecision);
		file << std::scientific;
		file << L"%%MatrixMarket matrix coordinate real general" << std::endl;
		file << m_nRows << L" " << m_nCols << L" " << GetNonZeroElementCount() << std::endl;
		for (size_t row = 0; row < Rows(); row++)
		{
			for (size_t col = 0; col < Cols(); col++)
			{
				auto val = GetValueAt(row, col);
				if (val != 0)
				{
					file << col + 1 << L" " << row + 1 << L" " << val << std::endl;
				}
			}
		}
	}
	catch (const std::exception&)
	{
		return false;
	}

	return true;
}
size_t CAbstractMatrix::GetNonZeroElementCount() const
{
	size_t res = 0;
	for (size_t row = 0; row < Rows(); row++)
	{
		for (size_t col = 0; col < Cols(); col++)
		{
			auto val = GetValueAt(row, col);
			if (val != 0)
			{
				res++;
			}
		}
	}
	return res;

}

//void CAbstractMatrix::SaveMatrixToSQLite(CSQLiteDatabase & Database, std::wstring sTableName) const
//{
//	std::vector<std::wstring> ColumnNames;
//	for (size_t i = 0; i < Cols(); i++)
//		ColumnNames.push_back(std::to_wstring(i));
//
//	Database.DropTableIfExists(sTableName);
//	Database.CreateTable(sTableName, ColumnNames);
//
//	Database.InsertInto(sTableName,CSQLiteTableRow(GetVectorAt(0)));
//
//	//std::vector<CSQLiteTableRow> allRows;
//	//for (const CAbstractVector & row : *this)
//	//{
//	//	allRows.push_back( row.GenerateSQLiteRow() );
//	//}
//	//Database.InsertIntoBatch(sTableName, allRows);
//}

CAbstractMatrix::iterator::iterator(const CAbstractMatrix& Matrix, size_t nIndex)
	:Matrix(Matrix)
	, nIndex(nIndex)
{
}

const CAbstractVector& CAbstractMatrix::iterator::operator*() const
{
	return Matrix.GetVectorAt(nIndex);
}

CAbstractMatrix::iterator& CAbstractMatrix::iterator::operator++()
{
	nIndex++;
	return *this;
}

CAbstractMatrix::iterator& CAbstractMatrix::iterator::operator++(int)
{
	nIndex++;
	return *this;
}

bool CAbstractMatrix::iterator::operator!=(const iterator& rhs) const
{
	return nIndex != rhs.nIndex;
}

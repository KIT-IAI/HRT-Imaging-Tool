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
#include "DenseMatrix.h"

#include "MathTools.h"



CDenseMatrix::CDenseMatrix()
{
	m_nMatrixType = eDense;
	m_pMat = nullptr;
}

CDenseMatrix::CDenseMatrix(size_t rows, size_t cols)
{
	m_nMatrixType = eDense;
	m_pMat = nullptr;
	CDenseMatrix::AllocMatrix(rows, cols);
}

CDenseMatrix::CDenseMatrix(const CAbstractVector& v, bool bColumnVector)
{
	assert(!v.IsNull());
	m_nMatrixType = eDense;

	if (bColumnVector)
	{
		m_nRows = v.Size();
		m_nCols = 1;
		m_pMat = new CDenseVector[m_nRows];

		CDenseVector vec(1);

		for (size_t i = 0; i < Rows(); i++)
		{
			vec[0] = v.GetValueAt(i);
			m_pMat[i] = vec;
		}
	}
	else
	{
		m_nRows = 1;
		m_nCols = v.Size();
		m_pMat = new CDenseVector[1];
		m_pMat[0].Copy(v);
	}
}
CDenseMatrix::CDenseMatrix(const std::vector<CDenseVector>& v, bool bAsColumns)
	:CDenseMatrix()
{
	assert(!v.empty());
	assert(std::all_of(v.begin(), v.end(), [&](const auto& vec) {return vec.Size() == v[0].Size();}));
	if (!bAsColumns)
	{
		AllocMatrix(v.size(), v[0].Size());
		for (size_t i = 0; i < v.size(); i++)
			(*this)[i].Copy(v[i]);
	}
	else
	{
		AllocMatrix(v[0].Size(), v.size());
		for (size_t i = 0; i < v.size(); i++)
			SetCol(v[i], i);
	}
}
CDenseMatrix::CDenseMatrix(const CDenseMatrix& m)
{
	m_nMatrixType = eDense;

	m_nRows = m.Rows();
	m_nCols = m.Cols();
	m_pMat = new CDenseVector[m_nRows];

	for (size_t i = 0; i < Rows(); i++)
	{
		m_pMat[i] = m.GetVectorAt(i);
	}
}

CDenseMatrix::~CDenseMatrix()
{
	CDenseMatrix::FreeMatrix();
}

void CDenseMatrix::FreeMatrix()
{
	isChild = false;
	if (m_pMat != nullptr)
	{
		delete[] m_pMat;
		m_pMat = nullptr;
		m_nRows = 0;
		m_nCols = 0;
	}
}
void CDenseMatrix::AllocMatrix(size_t rows, size_t cols)
{
	FreeMatrix();
	assert(!isChild);

	m_nRows = rows;
	m_nCols = cols;
	m_pMat = new CDenseVector[rows];

	CDenseVector v(cols);

	for (size_t n = 0; n < rows; n++)
	{
		m_pMat[n] = v;
	}
}

CDenseVector& CDenseMatrix::operator[](size_t row)
{
	return GetVectorAt(row);
}

const CDenseVector& CDenseMatrix::operator[](size_t row) const
{
	return GetVectorAt(row);
}
const CDenseVector& CDenseMatrix::GetVectorAt(size_t row) const
{
	assert((row >= 0) && (row < m_nRows));
	return m_pMat[row];
}
CDenseVector& CDenseMatrix::GetVectorAt(size_t row)
{
	assert((row >= 0) && (row < m_nRows));
	return m_pMat[row];
}
void CDenseMatrix::SetVectorAt(size_t row, const CAbstractVector& v)
{
	assert((row >= 0) && (row < m_nRows));
	m_pMat[row].Copy(v);
}

CDenseMatrix& CDenseMatrix::operator=(const CDenseMatrix& m)
{
	Copy(m);
	return *this;
}
void CDenseMatrix::Copy(const CAbstractMatrix& am)
{
	if (am.GetMatrixType() == eDense)
		Copy(static_cast<const CDenseMatrix&>(am));
	else if (am.GetMatrixType() == eSparse)
		Copy(static_cast<const CSparseMatrix&>(am));
	else
		CAbstractMatrix::Copy(am);
}
/**
Copies a matrix.
Reallocates if necessary
*/
void CDenseMatrix::Copy(const CDenseMatrix& m)
{
	if (m_nRows != m.Rows() || m_nCols != m.Cols())
		AllocMatrix(m.Rows(), m.Cols());

	for (size_t r = 0; r < m_nRows; r++)
	{
		m_pMat[r] = m.m_pMat[r];
	}
}
void CDenseMatrix::Copy(const CSparseMatrix& m)
{
	if (m_nRows != m.Rows() || m_nCols != m.Cols())
		AllocMatrix(m.Rows(), m.Cols());

	for (size_t r = 0; r < m_nRows; r++)
	{
		m_pMat[r] = m.GetVectorAt(r).ToDenseVector();
	}
}

CDenseMatrix& CDenseMatrix::operator*=(const CAbstractMatrix& m)
{
	assert(m_nRows == m_nCols && m.Rows() == m_nRows && m.Cols() == m_nCols);
	CDenseMatrix mm(*this);
	for (size_t r = 0; r < m_nRows; r++)
	{
		for (size_t c = 0; c < m_nCols; c++)
		{
			double sum = 0;
			for (size_t i = 0; i < m_nCols; i++)
			{
				sum += mm.m_pMat[r][i] * m.GetValueAt(i, c);
			}
			m_pMat[r][c] = sum;
		}
	}
	return *this;
}

CDenseMatrix CDenseMatrix::operator+(const CAbstractMatrix& m) const
{
	assert(m.Rows() == m_nRows && m.Cols() == m_nCols);
	CDenseMatrix mm(m.Rows(), m.Cols());
	for (size_t r = 0; r < m.Rows(); r++)
	{
		mm.m_pMat[r] = m_pMat[r] + m.GetVectorAt(r);
	}
	return mm;
}
CDenseMatrix CDenseMatrix::operator-(const CAbstractMatrix& m) const
{
	assert(m.Rows() == m_nRows && m.Cols() == m_nCols);
	CDenseMatrix mm(m.Rows(), m.Cols());
	for (size_t r = 0; r < m.Rows(); r++)
	{
		mm.m_pMat[r] = m_pMat[r] - m.GetVectorAt(r);
	}
	return mm;
}
CDenseMatrix CDenseMatrix::operator*(const CAbstractMatrix& m) const
{
	assert(m_nCols == m.Rows());
	CDenseMatrix mm(m_nRows, m.Cols());
	for (size_t r = 0; r < m_nRows; r++)
	{
		for (size_t c = 0; c < m.Cols(); c++)
		{
			double sum = 0;
			for (size_t i = 0; i < m_nCols; i++)
			{
				sum += m_pMat[r][i] * m.GetValueAt(i, c);
			}
			mm[r][c] = sum;
		}
	}
	return mm;
}
CDenseVector CDenseMatrix::operator*(const CAbstractVector& v) const
{
	assert(m_nCols == v.Size());
	CDenseVector vv(m_nRows);
	for (size_t r = 0; r < m_nRows; r++)
	{
		vv[r] = m_pMat[r] * v;
	}
	return vv;
}

#ifdef _WIN32
CDenseMatrix CDenseMatrix::ParallelProduct(const CAbstractMatrix& m) const
{
	assert(m_nCols == m.Rows());
	CDenseMatrix mm(m_nRows, m.Cols());

	concurrency::parallel_for(size_t(0), m_nRows, [&](size_t r)
		{
			for (size_t c = 0; c < m.Cols(); c++)
			{
				double sum = 0;
				for (size_t i = 0; i < m_nCols; i++)
				{
					sum += m_pMat[r][i] * m.GetValueAt(i, c);
				}
				mm[r][c] = sum;
			}
		});
	return mm;
}

CDenseVector CDenseMatrix::ParallelProduct(const CAbstractVector& v) const
{
	assert(m_nCols == v.Size());
	CDenseVector vv(m_nRows);

	concurrency::parallel_for(size_t(0), m_nRows, [&](size_t r)
		{
			vv[r] = m_pMat[r] * v;
		});
	return vv;
}
#endif // #ifdef _WIN32

CDenseMatrix CDenseMatrix::operator+(double s) const
{
	CDenseMatrix mm(m_nRows, m_nCols);
	for (size_t r = 0; r < m_nRows; r++)
	{
		mm.m_pMat[r] = m_pMat[r] + s;
	}
	return mm;
}
CDenseMatrix CDenseMatrix::operator-(double s) const
{
	CDenseMatrix mm(m_nRows, m_nCols);
	for (size_t r = 0; r < m_nRows; r++)
	{
		mm.m_pMat[r] = m_pMat[r] - s;
	}
	return mm;
}
CDenseMatrix CDenseMatrix::operator*(double s) const
{
	CDenseMatrix mm(m_nRows, m_nCols);
	for (size_t r = 0; r < m_nRows; r++)
	{
		mm.m_pMat[r] = m_pMat[r] * s;
	}
	return mm;
}
CDenseMatrix CDenseMatrix::operator/(double s) const
{
	assert(s != 0.0);

	CDenseMatrix mm(m_nRows, m_nCols);
	for (size_t r = 0; r < m_nRows; r++)
	{
		mm.m_pMat[r] = m_pMat[r] / s;
	}
	return mm;
}



CDenseVector CDenseMatrix::Row(size_t r) const
{
	assert((r >= 0) && (r < m_nRows));
	return m_pMat[r];
}

CDenseVector CDenseMatrix::Col(size_t c) const
{
	assert((c >= 0) && (c < m_nCols));
	CDenseVector vv(m_nRows);
	for (size_t r = 0; r < m_nRows; r++)
	{
		vv[r] = m_pMat[r][c];
	}
	return vv;
}
CDenseVector CDenseMatrix::Diagonal() const
{
	assert(Rows() == Cols());

	CDenseVector diag(Rows());

	for (size_t i = 0; i < Rows(); i++)
	{
		diag.SetValueAt(i, GetValueAt(i, i));
	}
	return diag;
}


CSparseMatrix CDenseMatrix::ToCSparseMatrix() const
{
	CSparseMatrix sm;
	sm.Copy(*this);
	return sm;
}



/**	\brief Erzeugt die transponierte Matrix.
 *
 *	\return Die transponierte Matrix.
 */
CDenseMatrix CDenseMatrix::Transpose() const
{
	CDenseMatrix out(this->Cols(), this->Rows());
	for (size_t i = 0; i < out.Rows(); i++)
	{
		out[i] = this->Col(i);
	}
	return out;
}

/**	\brief Berechnung der Inversen einer Matrix.
 *
 *	\details Die Berechung der Inversen erfolgt nach dem
 *	Gauss-Jordan-Verfahren (siehe Helmut Selder, "Einführung in die
 *	numerische Mathematik für Ingenieure", HANSER).
 *
 *	\details Das Verfahren löst die \c n Gleichungssysteme (für je eine
 *	Spalte der Einheitsvektoren) in einem gemeinsamen Eliminationsverfahren.
 *	Statt nur einem Vektor für die rechte Seite erweitert man die Matrix um
 *	alle \c n Einheitsvektoren
 *
\verbatim
	a11 a12 a13 |  1   0   0
	a21 a22 a23 |  0   1   0
	a31 a32 a33 |  0   0   1
\endverbatim
 *
 *	\details und eliminiert in der Matrix \a matrixIn alle Elemente außerhalb
 *	der Diagonalen. Zusätzlich sorgt man durch eine Division dafür, dass in
 *	der Diagonalen überall der Wert 1 steht. Das Ergebnis sieht
 *	folgendermaßen aus:
 *
\verbatim
	x1  x2  x3

	 1   0   0  | b11 b12 b13
	 0   1   0  | b21 b22 b23
	 0   0   1  | b31 b32 b33
\endverbatim
 *
 *	\details Dadurch fällt die Berechnung der Lösungen aus der Dreiecksmatrix
 *	<tt>(xn, xn-1, ..., x1)</tt> weg, weil die Lösungen sofort abgelesen
 *	werden können:
 *
\verbatim
	x1 = b11  bzw. b12 oder b13
	x2 = b21  bzw. b22 oder b23
	x3 = b31  bzw. b23 oder b33
\endverbatim
 *
 *	\details Die Einheitsmatrix ist also bei der Elimination in die inverse
 *	Matrix \a matrixOut übergegangen.
 *
 *	\details Durchgeführte Änderungen: Anpassung Übergabe der Parameter vom
 *	Typ CDenseMatrix und Ergänzung der Eingabematrix \a matrixIn um die
 *	Einheitsmatrix.
 *
 *	\details Quelle der Implementierung: http://www.zeiner.at/c/Matrix.html
 *	(Stand Mai 2004).
 *
 *
 *	\return invertedMatrix: Ermittlung der inversen Matrix erfolgreich.
 *	\return Eine leere durch CDenseMatrix() erzeugte Matrix: Eingangsmatrix a ist singulär.
 */
CDenseMatrix CDenseMatrix::Inverse() const
{
	CDenseMatrix invertedMatrix(this->m_nRows, this->m_nCols);


	size_t	pzeile;				// Pivotzeile
	double	f;						// Multiplikationsfaktor
	double	Maximum;				// Zeilenpivotisierung

	const double Epsilon = 0.001;	// Genauigkeit

	size_t n = m_nCols;

	CDenseMatrix aE(m_nRows, m_nCols * 2);

	// ergänze die Matrix matrixIn um eine Einheitsmatrix (rechts anhängen)
	for (size_t i = 0; i < n; i++)
	{
		for (size_t j = 0; j < n; j++)
		{
			aE[i][j] = m_pMat[i][j];
			aE[i][n + j] = 0.0;

			if (i == j)
			{
				aE[i][n + j] = 1.0;
			}
		}
	}

	// die einzelnen Eliminationsschritte
	size_t s = 0;
	do
	{
		// Pivotisierung vermeidet unnötigen Abbruch bei einer Null in der
		// Diagnonalen und erhöht die Rechengenauigkeit
		Maximum = fabs(aE[s][s]);
		pzeile = s;
		for (size_t i = s + 1; i < n; i++)
		{
			if (fabs(aE[i][s]) > Maximum)
			{
				Maximum = fabs(aE[i][s]);
				pzeile = i;
			}
		}

		if (Maximum < Epsilon)
		{
			return CDenseMatrix(); // nicht lösbar
		}

		if (pzeile != s) // falls erforderlich, Zeilen tauschen
		{
			double h;
			for (size_t j = s; j < 2 * n; j++)
			{
				h = aE[s][j];
				aE[s][j] = aE[pzeile][j];
				aE[pzeile][j] = h;
			}
		}

		// Eliminationszeile durch Pivot-Koeffizienten f = aE[s][s] dividieren
		f = aE[s][s];
		for (size_t j = s; j < 2 * n; j++)
		{
			aE[s][j] = aE[s][j] / f;
		}

		// Elimination --> Nullen in Spalte s oberhalb und unterhalb der Diagonalen
		// durch Addition der mit f multiplizierten Zeile s zur jeweiligen Zeile i
		for (size_t i = 0; i < n; i++)
		{
			if (i != s)
			{
				f = -aE[i][s];                 // Multiplikationsfaktor
				for (size_t j = s; j < 2 * n; j++)    // die einzelnen Spalten
				{
					aE[i][j] += f * aE[s][j];       // Addition der Zeilen i, s
				}
			}
		}
		s++;
	} while (s < n);

	// Die angehängte Einheitsmatrix Matrix hat sich jetzt in die inverse Matrix umgewandelt
	// Umkopieren auf die Zielmatrix
	for (size_t i = 0; i < n; i++)
	{
		for (size_t j = 0; j < n; j++)
		{
			invertedMatrix[i][j] = aE[i][n + j];
		}
	}

	return invertedMatrix;
}

/**	\brief Kopiert die Matrixelemente in eine neue Matrix mit \a rows Zeilen
 *	und \a cols Spalten.
 *
 *	\details Die Elemente werden zeilenweise ausgelesen und kopiert, das
 *	folgende Beispiel veranschaulicht die zugrundeliegende Reihenfolge der
 *	Matrixelemente:
 *
\verbatim
							  / a11 a12 \
	/ a11 a12 a13 a14 \       | a13 a14 |
	| a21 a22 a23 a24 |  ==>  | a21 a22 |
	\ a31 a32 a33 a34 /       | a23 a24 |
							  | a31 a32 |
							  \ a33 a34 /
\endverbatim
 *
 *	\details Die Gesamtzahl der Elemente <tt>rows*cols</tt> bzw.
 *	<tt>this->Rows()*this->Cols()</tt> muss bei beiden Matrizen identisch
 *	sein.
 *
 *	\return Die erzeugte Matrix.
 *
 *	\param[in] rows Die Anzahl der Zeilen der erzeugten Matrix.
 *	\param[in] cols Die Anzahl der Spalten der erzeugten Matrix.
 */
CDenseMatrix CDenseMatrix::Reshape(size_t rows, size_t cols) const
{
	assert(rows > 0);
	assert(cols > 0);
	assert(rows * cols == this->Rows() * this->Cols());

	if (this->IsNull())
	{
		return CDenseMatrix();
	}
	else
	{
		CDenseMatrix out(rows, cols);

		size_t rowIn = 0;
		size_t colIn = 0;
		for (size_t rowOut = 0; rowOut < out.Rows(); rowOut++)
		{
			for (size_t colOut = 0; colOut < out.Cols(); colOut++)
			{
				assert((rowIn < this->Rows()) && (colIn < this->Cols()));
				out[rowOut][colOut] = (*this)[rowIn][colIn++];
				if (colIn == this->Cols())
				{
					rowIn++;
					colIn = 0;
				}
			}
		}
		assert((rowIn == this->Rows()) && (colIn == 0));

		return out;
	}
}

/**	\brief Sortiert die Matrix aufsteigend nach den Werten in Spalte \a col.
 *
 *	Verwendet den Selection-Sort-Algorithmus (O(n²)) um die Anzahl der
 *	erforderlichen Kopier-Operationen zu minimieren.
 *
 *	\source http://de.wikibooks.org/wiki/Algorithmensammlung:_Sortierverfahren:_Selectionsort#C
 *		(mit einigen Änderungen)
 *
 *	\param[in] col Die Spalte, nach deren Werte die Matrix sortiert wird.
 */
void CDenseMatrix::SortByColumn(size_t col)
{
	size_t minRow;
	double minValue;

	if (Rows() == 0)
		return;

	for (size_t i = 0; i < Rows() - 1; i++)
	{
		minRow = i;
		minValue = GetValueAt(i, col);

		for (size_t k = i + 1; k < Rows(); k++)
		{
			double val = GetValueAt(k, col);
			if (val < minValue)
			{
				minRow = k;
				minValue = val;
			}
		}

		CDenseVector t = m_pMat[minRow];
		m_pMat[minRow] = m_pMat[i];
		m_pMat[i] = t;
	}
}

/**	\brief Sortiert die Matrix aufsteigend nach den Werten in Zeile \a row.
 *
 *	Verwendet den Selection-Sort-Algorithmus (O(n²)) um die Anzahl der
 *	erforderlichen Kopier-Operationen zu minimieren.
 *
 *	\source http://de.wikibooks.org/wiki/Algorithmensammlung:_Sortierverfahren:_Selectionsort#C
 *		(mit einigen Änderungen)
 *
 *	\param[in] row Die Zeile, nach deren Werte die Matrix sortiert wird.
 */
void CDenseMatrix::SortByRow(size_t row)
{
	size_t minCol;
	double minValue;

	for (size_t i = 0; i < Cols() - 1; i++)
	{
		minCol = i;
		minValue = GetValueAt(row, i);

		for (size_t k = i + 1; k < Cols(); k++)
		{
			double val = GetValueAt(row, k);
			if (val < minValue)
			{
				minCol = k;
				minValue = val;
			}
		}

		CDenseVector t = Col(minCol);
		SetCol(Col(i), minCol);
		SetCol(t, i);
	}
}

CDenseMatrix CDenseMatrix::IdentityMatrix(size_t size)
{

	CDenseMatrix iM = CDenseMatrix(size, size);
	iM.Fill(0);

	for (size_t n = 0; n < size; n++)
	{
		iM[n][n] = 1;
	}
	return iM;
}

/**	\brief Interpoliert die Matrix spaltenweise.
 *
 *	\param[in] k Die Anzahl der aus jedem Element der Matrix zu
 *		interpolierenden Werte. Es wird empfohlen, ungerade Werte zu
 *		verwenden.
 *	\param[out] matDest Die Ergebnis-Matrix.
 */
void CDenseMatrix::QuadraticColInterpolation(size_t k, CDenseMatrix& matDest) const
{
	assert(!this->IsNull());
	assert(k > 0);

	if ((matDest.Rows() != (k * this->Rows())) || (matDest.Cols() != this->Cols()))
	{
		matDest.FreeMatrix();
		matDest = CDenseMatrix(k * this->Rows(), this->Cols());
	}

	CDenseVector vecCol(k * this->Rows());

	for (size_t nCol = 0; nCol < this->Cols(); nCol++)
	{
		this->Col(nCol).QuadraticInterpolation(k, vecCol);

		matDest.SetCol(vecCol, nCol);
	}
}

/**	\brief Interpoliert die Matrix zeilenweise.
 *
 *	\param[in] k Die Anzahl der aus jedem Element der Matrix zu
 *		interpolierenden Werte. Es wird empfohlen, ungerade Werte zu
 *		verwenden.
 *	\param[out] matDest Die Ergebnis-Matrix.
 */
void CDenseMatrix::QuadraticRowInterpolation(size_t k, CDenseMatrix& matDest) const
{
	assert(!this->IsNull());
	assert(k > 0);

	if ((matDest.Rows() != this->Rows()) || (matDest.Cols() != (k * this->Cols())))
	{
		matDest.FreeMatrix();
		matDest = CDenseMatrix(this->Rows(), k * this->Cols());
	}

	for (size_t nRow = 0; nRow < this->Rows(); nRow++)
	{
		(*this)[nRow].QuadraticInterpolation(k, matDest[nRow]);
	}
}

/**	\brief Interpoliert die Matrix spaltenweise.
 *
 *	\param[in] k Die Anzahl der aus jedem Element der Matrix zu
 *		interpolierenden Werte.
 *	\param[out] matDest Die Ergebnis-Matrix.
 *
 *	\author Stephan Allgeier
 */
void CDenseMatrix::CubicSplineColInterpolation(size_t k, CDenseMatrix& matDest) const
{
	assert(!this->IsNull());
	assert(k > 0);

	if ((matDest.Rows() != (k * this->Rows())) || (matDest.Cols() != this->Cols()))
	{
		matDest.FreeMatrix();
		matDest = CDenseMatrix(k * this->Rows(), this->Cols());
	}

	CDenseVector vecCol(k * this->Rows());

	for (size_t nCol = 0; nCol < this->Cols(); nCol++)
	{
		this->Col(nCol).CubicSplineInterpolation(k, vecCol);

		matDest.SetCol(vecCol, nCol);
	}
}

/**	\brief Interpoliert die Matrix zeilenweise.
 *
 *	\param[in] k Die Anzahl der aus jedem Element der Matrix zu
 *		interpolierenden Werte.
 *	\param[out] matDest Die Ergebnis-Matrix.
 *
 *	\author Stephan Allgeier
 */
void CDenseMatrix::CubicSplineRowInterpolation(size_t k, CDenseMatrix& matDest) const
{
	assert(!this->IsNull());
	assert(k > 0);

	if ((matDest.Rows() != this->Rows()) || (matDest.Cols() != (k * this->Cols())))
	{
		matDest.FreeMatrix();
		matDest = CDenseMatrix(this->Rows(), k * this->Cols());
	}

	for (size_t nRow = 0; nRow < this->Rows(); nRow++)
	{
		(*this)[nRow].CubicSplineInterpolation(k, matDest[nRow]);
	}
}

void CDenseMatrix::Resize(size_t rows, size_t cols)
{

	if (cols == m_nCols)
	{
		if (rows == m_nRows)
		{
			return;
		}
		else
		{
			CDenseVector* matnew = new CDenseVector[rows];
			CDenseVector v(cols);

			for (size_t n = 0; n < std::min(rows, m_nRows); n++)
			{
				matnew[n] = m_pMat[n];
			}
			for (size_t n = std::min(rows, m_nRows); n < rows; n++)
			{
				matnew[n] = v;
			}
			delete[] m_pMat;
			m_pMat = matnew;
			m_nRows = rows;
			m_nCols = cols;
		}
	}
	else
	{
		CDenseVector* matnew = new CDenseVector[rows];

		CDenseVector v(cols);

		for (size_t n = 0; n < rows; n++)
		{
			matnew[n] = v;
		}
		for (size_t i = 0; i < std::min(rows, m_nRows); i++)
		{
			for (size_t j = 0; j < std::min(cols, m_nCols); j++)
			{
				matnew[i][j] = m_pMat[i][j];
			}
		}

		m_nRows = rows;
		m_nCols = cols;
		delete[] m_pMat;
		m_pMat = matnew;
	}
}

bool CDenseMatrix::SmoothMG(int redExp, bool smoothCols /*= false*/)
{
	if (smoothCols)
	{
		CDenseMatrix A = Transpose();
		SmoothMG(A, redExp);
		Copy(A.Transpose());
		return true;
	}
	else
		return SmoothMG(*this, redExp);
}

bool CDenseMatrix::SmoothMG(CDenseMatrix& X, int redExp)
{
	assert(redExp > 0);

	if (redExp == 1)
		return true;

	for (size_t row = 0; row < X.Rows(); row++)
	{
		X[row].SmoothVector(2);
	}

	CDenseMatrix reducedX(X.Rows(), X.Cols() / 2);
	reducedX.ReduceMatrix(X, 1, 2, true);

	SmoothMG(reducedX, redExp - 1);

	for (size_t row = 0; row < X.Rows(); row++)
	{
		reducedX[row].SmoothVector(2);
	}

	X.ProlongMatrix(reducedX, 1, 2);
	return true;
}

void CDenseMatrix::Child2D(CDenseMatrix& source, size_t row, size_t col, size_t rowCount, size_t colCount)
{
	assert(source.Rows() >= (row + rowCount));
	assert(source.Cols() >= (col + colCount));

	FreeMatrix();
	isChild = true;
	CDenseVector* childMat = new CDenseVector[rowCount];
	for (size_t i = 0; i < rowCount; i++)
	{
		childMat[i].Child1D(source.m_pMat[row + i], col, colCount);
	}
	m_pMat = childMat;
	m_nCols = colCount;
	m_nRows = rowCount;
}

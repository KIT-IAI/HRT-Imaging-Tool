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
#include "SLESolver.h"

#include <mutex>

#include <ppl.h>

#include "solvers.h"

#include "StringUtilities.h"
#include "Log.h"



std::map<CSLESolver::EAlgorithm, std::wstring> CSLESolver::EAlgorithmMap = {
	{ EAlgorithm::eGauss,					L"eGauss"					},
	{ EAlgorithm::eInverse,					L"eInverse"					},
	{ EAlgorithm::eRigidStartIteration,		L"eRigidStartIteration"		},
	{ EAlgorithm::eJacobi,					L"eJacobi"					},
	{ EAlgorithm::eCG,						L"eCG"						},
	{ EAlgorithm::eCGStartIteration,		L"eCGStartIteration"		},
	{ EAlgorithm::eCGALGLIB,				L"eCGALGLIB"				},
	{ EAlgorithm::eCGALGLIBStartIteration,	L"eCGALGLIBStartIteration"	}
};

CSLESolver::CSLESolver()
{
}

CSLESolver::~CSLESolver()
{
}

std::wstring CSLESolver::GetAlgorithmDescription(EAlgorithm eAlgorithm)
{
	switch (eAlgorithm)
	{
	case EAlgorithm::eGauss:
		return L"Gaußsches Eliminationsverfahren mit vollständiger Pivotsuche";
	case EAlgorithm::eJacobi:
		return L"Jacobi-Verfahren ohne Startvektor";
	case EAlgorithm::eInverse:
		return L"Gauß-Jordan-Verfahren (Inverse)";
	case EAlgorithm::eRigidStartIteration:
		return L"Jacobi-Verfahren mit Startvektor aus Grobregistrierung";
	case EAlgorithm::eCG:
		return L"(CG) Verfahren der konjugierenden Gradienten";
	case EAlgorithm::eCGStartIteration:
		return L"(CG-SV) CG-Verfahren mit Startvektor aus Grobregistrierung";
	case EAlgorithm::eCGALGLIB:
		return L"(CG-ALGLIB) Verfahren der konjugierenden Gradienten";
	case EAlgorithm::eCGALGLIBStartIteration:
		return L"(CG-SV-ALGLIB) CG-Verfahren mit Startvektor aus Grobregistrierung";
	default:
		assert(false);
		return L"No description found.";
	}
}

std::wstring CSLESolver::GetAlgorithmName(EAlgorithm eAlgorithm)
{
	return EAlgorithmMap[eAlgorithm];
}

CSLESolver::EAlgorithm CSLESolver::GetAlgorithmID(const std::wstring& sAlgorithm)
{
	for (auto e : EAlgorithmMap)
	{
		if (CStringUtilities::CompareNoCase(e.second, sAlgorithm))
		{
			return e.first;
		}
	}
	//use as error value
	return EAlgorithm::eAlgorithmCount;
}

CSLESolver::CIterationParameters CSLESolver::CreateIterationParameters(CSLESolver::EAlgorithm eAlgorithm)
{
	CSLESolver::CIterationParameters param;
	param.m_nMaxIterations = 0;

	if (eAlgorithm == CSLESolver::EAlgorithm::eJacobi || eAlgorithm == CSLESolver::EAlgorithm::eRigidStartIteration)
	{
		param.m_fEpsilon = 0.000015;
	}
	else
	{
		param.m_fEpsilon = 0.0;
		// This is the "CGUnlimited" setting.
		// Previously the break condition (Epsilon) was set to non-zero
		// values, depending on m_eAlgorithm:
	}

	return param;
}

/**	\brief Löst die durch die Matrix und die Spaltenvektoren in \a matInput
 *	definierten linearen Gleichungssysteme.
 *
 *	Die linearen Gleichungssysteme (LGSe) sind definiert als
 *	<em>A * x_i = b_i</em>, mit der Matrix \c this als Koeffizientenmatrix
 *	<em>A</em>, den Spalten der Matrix \a matInput als Konstantenvektoren
 *	<em>b_i</em> und den Spalten der Matrix \a matResult als Vektoren der
 *	gesuchten Unbekannten <em>x_i</em>.
 *
 *	Zur Lösung der LGSe wird der Gaussalgorithmus mit vollständiger
 *	Pivot-Suche verwendet.
 *
 *	\return \c true, wenn die Lösungen der LGSe existieren.
 *	\return \c false, wenn die Matrix singulär ist und die LGSe nicht
 *		(eindeutig) lösbar sind.
 *
 *	\param[in] matInput Die Konstantenvektoren der LGSe (als Spaltenvektoren
 *		von \a matInput).
 *	\param[out] matResult Die Vektoren der gesuchten Unbekannten der LGSe
 *		(als Spaltenvektoren von \a matResult).
 */
bool CSLESolver::SolveEquationGauss(const CAbstractMatrix& A, const CDenseMatrix& matInput, CDenseMatrix& matResult)
{
	assert(A.Rows() > 0);
	assert(A.Rows() == A.Cols());
	assert(A.Rows() == matInput.Rows());
	assert(A.Rows() == matResult.Rows());
	assert(matInput.Cols() > 0);
	assert(matInput.Cols() == matResult.Cols());

	size_t nCount = A.Rows();
	size_t nCount2 = matInput.Cols();

	// a matrix to copy the input matrix; this will be worked on
	CDenseMatrix extA(nCount, nCount + nCount2);

	// a vector to log the column swappings
	size_t* order = new size_t[nCount];

	// copy the input matrix; the constants are put into the last columns
	concurrency::parallel_for(size_t(0), nCount, [&](size_t nRow)
		{
			for (size_t nCol = 0; nCol < nCount; nCol++)
			{
				extA[nRow][nCol] = A.GetValueAt(nRow, nCol);
			}

			for (size_t nCol2 = 0; nCol2 < nCount2; nCol2++)
			{
				extA[nRow][nCount + nCol2] = matInput[nRow][nCol2];
			}

			order[nRow] = nRow;
		});

	// solve equations using _T("gauss-elimination"):
	// We want to transform the system:
	//	( a11  a12 ... a1n | b11 ... b1m )
	//	(          ...     |     ...     )
	//	( an1  an2 ... ann | bn1 ... bnm )
	// to:
	//	 ( a11_T('  a12') ... a1n_T(' | b11') ... b1m' )
	//   (  0    a22_T(' ... a2n') | b21_T(' ... b2m') )
	//	 (            ...      |      ...      )
	//	 (  0     0   ... ann_T(' | bn1') ... bnm' )   (upper triangular matrix)
	//
	// method:
	// (A;B)_1 := (A;B)
	// (A;B)_(i+1) = L_i * (A;B)_i
	// L_i = E - m_i * (e_i)^T
	// m_i = (0,...,0,a_T('_(i+1)i/a')_ii,...,a_T('_ni/a_ii)^T  where a') is from A_i
	// e_i = (0,...,0,1,0,...,0)

	size_t	nColMax;
	size_t	nRowMax;
	size_t	nTemp;
	double	fMaxValue;

	for (size_t nIteration = 0; nIteration < nCount - 1; nIteration++)
	{
		// use _T("total pivot search") to get a_ii as the elemement in the current
		// column having the biggest value
		// we have:
		//  ( a11  a12 ...    a1i ... a1n )
		//  (  0   a22 ...    a2i ... a2n )
		//  (          ...                )
		//  (       0  ... 0  aii ... ain )
		//  (          ...                )
		//  (  0    0  ... 0  ani ... ann )
		// We search for a x,y in {i,...,n}, so that |axy| > |aii|

		fMaxValue = fabs(extA[nIteration][nIteration]);
		nColMax = nIteration;
		nRowMax = nIteration;

		for (size_t nRow = nIteration; nRow < nCount; nRow++)
		{
			for (size_t nCol = nIteration; nCol < nCount; nCol++)
			{
				double fTemp = fabs(extA[nRow][nCol]);
				if (fTemp > fMaxValue)
				{
					fMaxValue = fTemp;
					nColMax = nCol;
					nRowMax = nRow;
				}
			}
		}

		// if the value of fMaxValue is 0, the matrix is singulary; return false
		if (fMaxValue == 0)
		{
			delete[] order;
			return false;
		}

		// swap rows if nRowMax != nIteration
		if (nRowMax != nIteration)
		{
			//Do not Copy the Data, just swap the pointers
			extA[nIteration].Swap(extA[nRowMax]);
		}

		// swap columns if nColMax != nIteration
		if (nColMax != nIteration)
		{
			// swap positions in the _T("order")-vector
			nTemp = order[nIteration];
			order[nIteration] = order[nColMax];
			order[nColMax] = nTemp;

			// swap columns in the matrix
			concurrency::parallel_for(size_t(0), nCount, [&](size_t nRow)
				{
					std::swap(extA[nRow][nIteration], extA[nRow][nColMax]);
				});
		}

		// recalculate the matrix using
		// (A;B)_(i+1) = L_i * (A;B)_i with
		//
		// L_i = E - m_i * (e_i)^T
		//
		// m_i = (0 , ... , 0 , a_T('_(i+1)i/a')_ii , ... , a_T('_ni/a_ii)^T  (where a') is from A_i)
		//        \_________/
		//          i times
		//
		// e_i = (0 , ... , 0 , 1 , 0 , ... , 0)
		//        \_________/
		//         i-1 times
		//
		// the vectors m_i and e_i are not actually used; the matrix L_i is a
		// matrix with 1's on the diagonal and only the i-th column below the
		// diagonal containing non-zero values. The matrix multiplication L_i * A_i
		// can therefore be calculated much faster by the following code.

		concurrency::parallel_for(size_t(nIteration + 1), nCount, [&](size_t nRow)
			{
				double fTemp = -(extA[nRow][nIteration] / extA[nIteration][nIteration]);
				for (size_t nCol = nIteration + 1; nCol < nCount + nCount2; nCol++)
				{
					extA[nRow][nCol] += fTemp * extA[nIteration][nCol];
				}
				extA[nRow][nIteration] = 0;
			});
	}

	// if the value of fMaxValue is 0, the matrix is singulary; return false
	if (fabs(extA[nCount - 1][nCount - 1]) == 0)
	{
		delete[] order;
		return false;
	}

	// get the results
	// We've got a triangular matrix of the form:
	//    (a11  a12 ... a1n | b11 ... b1m )
	//    ( 0   a21 ... a2n | b21 ... b2m )
	//    (         ...     |     ...     )
	//    ( 0    0  ... ann | bn1 ... bnm )
	//
	// Now we compute x11 to xnm
	// 1. xni      =  bni / ann
	// 2. x(n-1)i  =  (b(n-1)i - a(n-1)n*xni) / a(n-1)(n-1)
	// 			  ...
	// n. x1i      =  (b1i - a12*x2i - a13*x3i - ... - a1nxn) / a11

	size_t lower = nCount;

	for (ptrdiff_t nRow = nCount - 1; nRow >= 0; nRow--)
	{
		for (size_t nCol2 = 0; nCol2 < nCount2; nCol2++)
		{
			double fTemp = 0.0;
			for (size_t nCol = lower; nCol < nCount; nCol++)
			{
				fTemp += extA[nRow][nCol] * matResult[order[nCol]][nCol2];
			}

			matResult[order[nRow]][nCol2] = (extA[nRow][nCount + nCol2] - fTemp) / extA[nRow][nRow];
		}
		lower--;
	}
	delete[] order;

	return true;
}

bool CSLESolver::SolveEquationCG(const CSparseMatrix& A, const CDenseMatrix& B, CDenseMatrix& x, const CIterationParameters& param, bool opt)
{
	bool bReturn = true;
	std::mutex criticalMutex;

	concurrency::parallel_for(size_t(0), x.Cols(), [&](size_t i)
		{
			CDenseVector xCol;
			CDenseVector bCol;

			{
				criticalMutex.lock();
				xCol = x.Col(i);
				bCol = B.Col(i);
				criticalMutex.unlock();
			}

			if (!SolveEquationCG(A, bCol, xCol, param, opt))
			{
				bReturn = false;
			}

			{
				criticalMutex.lock();
				x.SetCol(xCol, i);
				criticalMutex.unlock();
			}
		});

	return bReturn;
}

bool CSLESolver::SolveEquationCGALGLIB(const CSparseMatrix& A, const CDenseMatrix& B, CDenseMatrix& x, const CIterationParameters& param)
{
	bool bReturn = true;
	std::mutex criticalMutex;

	concurrency::parallel_for(size_t(0), x.Cols(), [&](size_t i)
		{
			CDenseVector xCol;
			CDenseVector bCol;

			{
				criticalMutex.lock();
				xCol = x.Col(i);
				bCol = B.Col(i);
				criticalMutex.unlock();
			}

			if (!SolveEquationCGALGLIB(A, bCol, xCol, param))
			{
				bReturn = false;
			}

			{
				criticalMutex.lock();
				x.SetCol(xCol, i);
				criticalMutex.unlock();
			}
		});

	return bReturn;
}

bool CSLESolver::SolveEquationJacobi(CSparseMatrix& A, const CDenseMatrix& B, CDenseMatrix& x, const CIterationParameters& param)
{
	const double fEpsilon = param.m_fEpsilon;
	const double alpha = 0.6;
	const size_t resultCheck = 100;

	// Calculate Reference-Defect for Abort-Criteria

	double* referenceDefect = new double[B.Cols()];

	CDenseVector	diagA = A.Diagonal();
	CDenseMatrix reference = B.Transpose();
	(*static_cast<CAbstractMatrix*>(&reference)) *= alpha;	// This is probably the worst line of code ever written, but causes the compiler to call the right *= operator...

	for (size_t i = 0; i < B.Cols(); i++)
	{
		reference[i].PointwiseDivision(reference[i], diagA);
		referenceDefect[i] = sqrt(reference[i] * reference[i]);
	}
	reference.FreeMatrix();
	diagA.FreeVector();

	// Initialize the rest

	CDenseMatrix	temp(x.Transpose()); // This transposing is done because CDenseMatrix.Col() takes ages, while CDenseMatrix.Row() is super fast
	CDenseMatrix	temp2 = CDenseMatrix(temp);

	CDenseMatrix* newGuess = &temp2;
	CDenseMatrix* oldGuess = &temp;
	CDenseMatrix* tempPTR = 0;

	bool done = false;

	double currentDefect = 0;

	size_t iterations = 0;
	do
	{
		for (size_t row = 0; row < A.Rows(); row++)
		{
			CSparseVector& actualRow = A.GetVectorAt(row);
			double mRowRow = actualRow.GetValueAt(row);

			for (size_t resCol = 0; resCol < x.Cols(); resCol++)
			{
				double b = B[row][resCol];
				b -= actualRow * (*oldGuess)[resCol];
				b *= alpha / mRowRow;
				(*newGuess)[resCol][row] = b;
			}
		}

		if (iterations % resultCheck == 0)
		{
			double currentXDefect = (newGuess->Row(0).Norm2()) / referenceDefect[0];
			double currentYDefect = (newGuess->Row(1).Norm2()) / referenceDefect[1];
			currentDefect = max(currentXDefect, currentYDefect);

			done = currentDefect < fEpsilon;
		}
		if (iterations > param.m_nMaxIterations)
			done = true;

		(*newGuess) += (*oldGuess);

		iterations++;

		tempPTR = newGuess;
		newGuess = oldGuess;
		oldGuess = tempPTR;
	} while (!done);

	if (oldGuess == &temp)
	{
		x = temp.Transpose();
	}
	else
	{
		x = temp2.Transpose();
	}

	delete[] referenceDefect;

	return true;
}

bool CSLESolver::SolveEquationCG(const CSparseMatrix& A, const CDenseVector& b, CDenseVector& x, const CIterationParameters& param, bool opt)
{
	assert(A.Rows() == A.Cols());
	assert(A.Rows() == b.Size());
	assert(A.Rows() == x.Size());
	assert(param.m_fEpsilon >= 0.0);

	size_t maxIterations = param.m_nMaxIterations;
	if (maxIterations == 0)
	{
		maxIterations = A.Rows();
	}

	double fEpsilon = param.m_fEpsilon;

	const int divergenceCheck = 10;
	const double bNorm = b.Norm2();

	bool done = false;
	double lastDefects[3] = { 0 };

	// Conditioning
	CDenseVector conditionVector;
	CDenseVector* q = nullptr;
	if (opt)
	{
		conditionVector.AllocVector(A.Rows());
		for (size_t i = 0; i < conditionVector.Size(); i++)
		{
			conditionVector[i] = 1 / A.GetValueAt(i, i);
		}

		q = new CDenseVector(A.Rows());
	}

	// Initialization
	CDenseVector r = b - A * x;
	CGConditioning(q, r, opt, conditionVector);
	CDenseVector p = *q;

	for (size_t iterations = 0; iterations != maxIterations && !done; iterations++)
	{
		CDenseVector a = A * p;

		double pa = p * a;

		double alpha = (r * p) / (pa);
		x += p * alpha;
		r -= a * alpha;
		CGConditioning(q, r, opt, conditionVector);		// Thats a little tricky. If opt is true q is a Vector. If not, q points to r. Thsi is complicated but saves n copies per iteration
		p = (*q) - p * (((*q) * a) / pa);

		if (iterations % divergenceCheck == 0)
		{
			lastDefects[2] = lastDefects[1];
			lastDefects[1] = lastDefects[0];
			lastDefects[0] = r.Norm2() / bNorm;

			if (lastDefects[0] < fEpsilon)
			{
				done = true;
			}
			if (lastDefects[0] > lastDefects[1] && lastDefects[1] > lastDefects[2]) // Fehler wurde 3 mal schlechter
			{
				// Neu initialisieren
				r = b - A * x;
				CGConditioning(q, r, opt, conditionVector);
				p = *q;
			}
		}
	}

	if (opt)
	{
		delete q;
	}

	return true;
}

bool CSLESolver::SolveEquationCGALGLIB(const CSparseMatrix& A, const CDenseVector& b, CDenseVector& x, const CIterationParameters& param)
{
	assert(A.Rows() == A.Cols());
	assert(A.Rows() == b.Size());
	assert(A.Rows() == x.Size());
	assert(param.m_fEpsilon >= 0.0);

	size_t maxIterations = param.m_nMaxIterations;
	if (maxIterations == 0)
	{
		maxIterations = A.Rows();
	}

	double fEpsilon = param.m_fEpsilon;

	size_t n = A.Rows();

	alglib::sparsematrix aa;
	alglib::real_1d_array ab;
	alglib::real_1d_array ax;

	alglib::sparsecreate(n, n, aa);

	ab.setlength(n);
	ax.setlength(n);

	for (size_t i = 0; i < n; i++)
	{
		const CSparseVector& sv = A.GetVectorAt(i);
		for (size_t j = 0; j < sv.NonZeroSize(); j++)
		{
			alglib::sparseset(aa, i, sv.GetNonZeroColumnAt(j), sv.GetNonZeroValueAt(j));
		}
	}

	alglib::sparseconverttocrs(aa);

	for (size_t i = 0; i < n; i++)
	{
		ab[i] = b[i];
		ax[i] = x[i];
	}

	alglib::lincgstate astate;
	alglib::lincgreport areport;

	alglib::lincgcreate(n, astate);
	alglib::lincgsetcond(astate, fEpsilon, maxIterations);
	alglib::lincgsetstartingpoint(astate, ax);
	alglib::lincgsolvesparse(astate, aa, true, ab);
	alglib::lincgresults(astate, ax, areport);

	for (size_t i = 0; i < n; i++)
	{
		x[i] = ax[i];
	}

	if (!param.m_bQuiet && areport.terminationtype <= 0)
	{
		std::wstring formatString(L"Solver terminated with termination code %d\n\nNumber of iterations: %d\nNumber of multiplications: %d\n R2: %f");
		CLog::Log(CLog::eError, L"SolveEquationCGALGLIB", boost::wformat(formatString) % areport.terminationtype % areport.iterationscount % areport.nmv % areport.r2);
	}

	return areport.terminationtype > 0;
}

bool CSLESolver::SolveEquationGauss(const CAbstractMatrix& A, const CDenseVector& vecInput, CDenseVector& vecResult)
{
	CDenseMatrix in(vecInput, true);
	CDenseMatrix res(vecResult, true);

	if (!SolveEquationGauss(A, in, res))
	{
		return false;
	}

	vecResult = res.Col(0);

	return true;
}

bool CSLESolver::SolveEquationJacobi(CSparseMatrix& A, const CDenseVector& vecInput, CDenseVector& vecResult, const CIterationParameters& param)
{
	CDenseMatrix in(vecInput, true);
	CDenseMatrix res(vecResult, true);

	if (!SolveEquationJacobi(A, in, res, param))
	{
		return false;
	}
	vecResult = res.Col(0);

	return true;
}

void CSLESolver::CGConditioning(CDenseVector*& out, CDenseVector& r, bool opt, const CDenseVector& conditionVector)
{
	if (opt)
	{
		assert(conditionVector.Size() == r.Size());
		out->PointwiseMultiplication(r, conditionVector);
	}
	else
	{
		out = &r;
	}
}

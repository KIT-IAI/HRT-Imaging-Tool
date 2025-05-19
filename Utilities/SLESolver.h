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

#include <map>

#include "SparseMatrix.h"

class CSLESolver
{

public:

	CSLESolver();
	virtual ~CSLESolver();

public:

	enum class EAlgorithm : int		// Careful! No number can be left out here! If you remove one, make sure no index remains empty. 
	{
		eGauss,
		eInverse,
		eRigidStartIteration,
		eJacobi,
		eCG,
		eCGStartIteration,
		eCGALGLIB,
		eCGALGLIBStartIteration,
		eAlgorithmCount,
	};

	static std::map<EAlgorithm, std::wstring> EAlgorithmMap;

public:

	struct CIterationParameters
	{
		size_t	m_nMaxIterations = 0;
		double	m_fEpsilon = 0.0;
		bool	m_bQuiet = false;
	};

public:

	static std::wstring GetAlgorithmDescription(EAlgorithm eAlgorithm);
	static std::wstring GetAlgorithmName(EAlgorithm eAlgorithm);
	static EAlgorithm GetAlgorithmID(const std::wstring& sAlgorithm);
	static CIterationParameters CreateIterationParameters(CSLESolver::EAlgorithm eAlgorithm);

	static bool SolveEquationGauss(const	CAbstractMatrix& A, const CDenseMatrix& B, CDenseMatrix& x);
	static bool SolveEquationJacobi(CSparseMatrix& A, const CDenseMatrix& B, CDenseMatrix& x, const CIterationParameters& param);
	static bool SolveEquationCG(const	CSparseMatrix& A, const CDenseMatrix& B, CDenseMatrix& x, const CIterationParameters& param, bool opt);
	static bool SolveEquationCGALGLIB(const	CSparseMatrix& A, const CDenseMatrix& B, CDenseMatrix& x, const CIterationParameters& param);

	static bool SolveEquationGauss(const	CAbstractMatrix& A, const CDenseVector& B, CDenseVector& x);
	static bool SolveEquationJacobi(CSparseMatrix& A, const CDenseVector& B, CDenseVector& x, const CIterationParameters& param);
	static bool SolveEquationCG(const	CSparseMatrix& A, const CDenseVector& b, CDenseVector& x, const CIterationParameters& param, bool opt);
	static bool SolveEquationCGALGLIB(const	CSparseMatrix& A, const CDenseVector& b, CDenseVector& x, const CIterationParameters& param);

private:

	static void CGConditioning(CDenseVector*& out, CDenseVector& r, bool opt, const CDenseVector& conditionVector);

};

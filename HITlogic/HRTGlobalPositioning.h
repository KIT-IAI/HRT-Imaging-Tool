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

#include "GlobalPositioningParameters.h"
#include "RigidRegistrationResult.h"
#include "SLESolver.h"
#include "AbstractMatrix.h"
#include "ProcessType.h"
#include "ImageRegistrationResult.h"
#include "HrtImageParameters.h"
#include "Progressing.h"
#include "Cancelable.h"

class CHRTGlobalPositioning : public IProgressing, public ICancelable
{
private:
	typedef std::pair<size_t, size_t> index_pair_t;

public:
	explicit CHRTGlobalPositioning(const CGlobalPositioningParameters& parameters);

	CDenseMatrix SolvePositioning(const CImageRegistrationResult& RegistrationResult, const CHrtImageParameters& ImageParameters);

	bool SolvePositioning(const index_pair_t& minMaxImageIndexes, size_t nSubImagesPerImageWithGap, size_t nSubimageRowOffset, size_t ColumnCount, CDenseMatrix* pSolution, const std::vector<CRigidRegistrationResult>& LocalRegistrationSolutions);
	bool SolvePositioningWithConsistencyCheck(const index_pair_t& minMaxImageIndexes, size_t nSubImagesPerImageWithGap, size_t nSubimageRowOffset, CDenseMatrix* pSolution, std::vector<CRigidRegistrationResult>& LocalRegistrationSolutions, double fThreshold = 5.0);

	static void RemoveGap(const CDenseMatrix& source, CDenseMatrix& destination, size_t nSubImagesWithoutGap, size_t nGapBefore, size_t nGapAfter);
	static void RemoveGap(CDenseMatrix& inPlace, size_t nSubImagesWithoutGap, size_t nGapBefore, size_t nGapAfter);

	static void GenerateWtW(CSLESolver::EAlgorithm eAlgorithm, std::shared_ptr<CAbstractMatrix>& pWtW, size_t mSize);
	static void GenerateWtd(std::shared_ptr<CDenseMatrix>& pWtd, size_t mSize);
	static void GenerateRegularisation(CProcessType::EProcessType eProcessType, size_t nImages, size_t nSubImagesPerImageWithGap, size_t nColumnCount, CAbstractMatrix& WtW);
	static void AssignAnchorImage(CSLESolver::EAlgorithm eAlgorithm, CAbstractMatrix& WtW);
	static void AddSystemData(const index_pair_t& minMaxImageIndexes, const std::vector<CRigidRegistrationResult>& LocalRegistrationSolution, size_t SubImagesRowCount, size_t nSubimageRowOffset, size_t SubImagesColumnCount, CAbstractMatrix& wtw, CDenseMatrix& wtd, CProcessType::EProcessType eProcessType);

	static CDenseMatrix CreateStartVector(const index_pair_t& minMaxImageIndexes, size_t nSubImagesWithGaps, CSLESolver::EAlgorithm algo, const std::vector<CRigidRegistrationResult>& RigidRegistrationResultArray);

private:
	CGlobalPositioningParameters m_Parameters;

	static const double s_fLocalCurvatureCorrectionParameter;
	static const double s_fGlobalCurvatureCorrectionParameter;
	static const double s_fLocalGradientCorrectionParameter;
	static const double s_fGlobalGradientCorrectionParameter;
	static const double s_fEpsilon1;
	static const double s_fEpsilon2;

	static void GenerateConstraintCurvatureSystem(size_t nSubImages, size_t nImages, CAbstractMatrix& m, double fLambda);
	static void GenerateConstraintGradientSystem(size_t nSubImages, size_t nImages, CAbstractMatrix& m, double fLambda);
	static void GenerateConstraintCurvatureSystem2D(size_t nBlocksX, size_t nBlocksY, size_t nImages, CAbstractMatrix& m, double fLambda);

	static void AddEntrySLERigidRegistration(CAbstractMatrix& WtW, CDenseMatrix& Wtd, float ux, float uy, size_t image1, size_t nSubImage, size_t image2, size_t SubImagesHeight, size_t SubImagesCount, size_t ImagesCount);
	static void AddEntrySLE2D(CAbstractMatrix& WtW, CDenseMatrix& Wtd, float ux, float uy, size_t image1, size_t image2, size_t SubImageColumnCount, size_t SubImageRowCount, size_t nBlockNr);
	static void AddEntrySLE(CAbstractMatrix& WtW, CDenseMatrix& Wtd, float ux, float uy, size_t image1, size_t nSubImage, size_t image2, size_t SubImagesHeight, size_t SubImagesCount, size_t ImagesCount);
	static void AddEntrySLE(CAbstractMatrix& WtW, CDenseMatrix& Wtd, const CRigidRegistrationResult& regResult, size_t nSubimageRowOffset, size_t SubImagesCount, size_t ImagesCount);

	static void RemoveExcludedImages(const CDenseMatrix& source, CDenseMatrix& destination, size_t nSubImagesWithoutGap, const std::list<size_t>& group);
	static void RemoveExcludedImages(CDenseMatrix& inPlace, size_t nSubImagesWithoutGap, const std::list<size_t>& group);

	CDenseMatrix SolvePositioningBlockbased(const CImageRegistrationResult& RegistrationResult, const CHrtImageParameters& ImageParameters);
protected:
	virtual CProgress GetProgress() override;
	virtual void OnCancel() override;
	virtual void OnUncancel() override;

};

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
#include "HRTGlobalPositioning.h"



const double CHRTGlobalPositioning::s_fLocalCurvatureCorrectionParameter = 0.0;
const double CHRTGlobalPositioning::s_fGlobalCurvatureCorrectionParameter = 1.0;
const double CHRTGlobalPositioning::s_fLocalGradientCorrectionParameter = 0.1;
const double CHRTGlobalPositioning::s_fGlobalGradientCorrectionParameter = 0.0;

const double CHRTGlobalPositioning::s_fEpsilon1 = 0.001;
const double CHRTGlobalPositioning::s_fEpsilon2 = 0.0000001;

CHRTGlobalPositioning::CHRTGlobalPositioning(const CGlobalPositioningParameters& parameters) :
	m_Parameters(parameters) {
}

///	<summary> Calculate the image or sub-image positions for a single image group. </summary>
///	<returns> A 2-column matrix containing the x- and y-coordinates of the image or sub-image positions. </returns>
///	<param name="RegistrationResult"> The image indexes and registration results for a single image group. </param>
///	<param name="ImageParameters"> The parameters used for the calculation. </param>
CDenseMatrix CHRTGlobalPositioning::SolvePositioning(const CImageRegistrationResult& RegistrationResult, const CHrtImageParameters& ImageParameters)
{
	// there must be exactly one image group; it must be sorted and not empty
	assert(RegistrationResult.ImageGroups.size() == 1);
	assert(!RegistrationResult.ImageGroups[0].empty());
	assert(std::is_sorted(RegistrationResult.ImageGroups[0].begin(), RegistrationResult.ImageGroups[0].end()));

	auto group = RegistrationResult.ImageGroups[0];

	index_pair_t minMaxImageIndexes = { group.front(), group.back() };
	auto RigidRegistrationResults = RegistrationResult.GetRigidRegistrationResults();
	CDenseMatrix Solution = CreateStartVector(minMaxImageIndexes, ImageParameters.GetSubImageCountWithGap(), m_Parameters.eAlgorithm, RigidRegistrationResults);

	if (RigidRegistrationResults.size() == 0)
	{
		Solution = CDenseMatrix(ImageParameters.nSubImagesPerImageWithoutGap, 2);
		Solution.Fill(0.0);
		return Solution;
	}

	bool bPositioningSuccessful = false;
	switch (m_Parameters.eProcessType)
	{
	case CProcessType::eBlockBasedRegistration:
		m_Parameters.eProcessType = CProcessType::eRigidRegistration;
		bPositioningSuccessful = SolvePositioningWithConsistencyCheck(minMaxImageIndexes, ImageParameters.nSubImagesPerImageWithoutGap, ImageParameters.nGapsBeforeImage, &Solution, RigidRegistrationResults);
		break;
	case CProcessType::eRigidRegistration:
		bPositioningSuccessful = SolvePositioning(minMaxImageIndexes, 1, 0, 1, &Solution, RigidRegistrationResults);
		break;
	case CProcessType::eHRTImageRegistration:
	case CProcessType::eHRTStreamRegistration:
		bPositioningSuccessful = SolvePositioning(minMaxImageIndexes, ImageParameters.GetSubImageCountWithGap(), ImageParameters.nGapsBeforeImage, 1, &Solution, RegistrationResult.GetFlexibleRegistrationResults());
		break;
	default:
		assert(false);
	}

	RemoveGap(Solution, ImageParameters.nSubImagesPerImageWithoutGap, ImageParameters.nGapsBeforeImage, ImageParameters.nGapsAfterImage);
	RemoveExcludedImages(Solution, ImageParameters.nSubImagesPerImageWithoutGap, group);

	return Solution;
}

CDenseMatrix CHRTGlobalPositioning::SolvePositioningBlockbased(const CImageRegistrationResult& RegistrationResult, const CHrtImageParameters& ImageParameters)
{
	assert(!RegistrationResult.ImageGroups[0].empty());
	assert(std::is_sorted(RegistrationResult.ImageGroups[0].begin(), RegistrationResult.ImageGroups[0].end()));

	index_pair_t minMaxImageIndexes = { RegistrationResult.ImageGroups[0].front(), RegistrationResult.ImageGroups[0].back() };
	auto RigidRegistrationResults = RegistrationResult.GetRigidRegistrationResults();
	CDenseMatrix Solution = CreateStartVector(minMaxImageIndexes, ImageParameters.GetSubImageCountWithGap(), m_Parameters.eAlgorithm, RigidRegistrationResults);

	if (RigidRegistrationResults.size() == 0)
		return Solution;

	m_Parameters.eProcessType = CProcessType::eRigidRegistration;

	SolvePositioningWithConsistencyCheck(minMaxImageIndexes, ImageParameters.nSubImagesPerImageWithoutGap, ImageParameters.nGapsBeforeImage, &Solution, RigidRegistrationResults);
	return Solution;
}

///	<summary> Create an initial estimation of the solution matrix for the given group of images. </summary>
///	<returns> A 2-column matrix containing the x- and y-coordinates of the initial position estimations. </returns>
///	<param name="minMaxImageIndexes"> The minimum and maximum image indexes of the image group. </param>
///	<param name="nSubImagesWithGaps"> The number of sub-images per image, including the gap. </param>
///	<param name="algo"> The solver algorithm. </param>
///	<param name="RigidRegistrationResultArray"> The rigid registration results for the image group. </param>
CDenseMatrix CHRTGlobalPositioning::CreateStartVector(const index_pair_t& minMaxImageIndexes, size_t nSubImagesWithGaps, CSLESolver::EAlgorithm algo, const vector<CRigidRegistrationResult>& RigidRegistrationResultArray)
{
	size_t nImages = minMaxImageIndexes.second - minMaxImageIndexes.first + 1;

	CDenseMatrix Solution(nImages * nSubImagesWithGaps, 2);

	switch (algo)
	{
	case CSLESolver::EAlgorithm::eGauss:
	case CSLESolver::EAlgorithm::eInverse:
		break;
	case CSLESolver::EAlgorithm::eCG:
	case CSLESolver::EAlgorithm::eCGALGLIB:
	case CSLESolver::EAlgorithm::eJacobi:
		Solution.Fill(0.0);
		break;
	case CSLESolver::EAlgorithm::eRigidStartIteration:
	case CSLESolver::EAlgorithm::eCGStartIteration:
	case CSLESolver::EAlgorithm::eCGALGLIBStartIteration:
	{
		if (nSubImagesWithGaps == 1)	// If this is a rigid registration, creating a start vector does not make sense.
		{
			Solution.Fill(0.0);
			break;
		}

		// Create a solver to solve the SLE of the rigid registration
		CHRTGlobalPositioning rigidSolver(CGlobalPositioningParameters(CSLESolver::EAlgorithm::eGauss, CProcessType::eRigidRegistration));

		CDenseMatrix RigidSolution(nImages, 2);

		// Solve Rigid SLE
		rigidSolver.SolvePositioning(minMaxImageIndexes, 1, 0, 1, &RigidSolution, RigidRegistrationResultArray);

		// Prolong rigid Solution
		Solution.ProlongMatrix(RigidSolution, nSubImagesWithGaps, 1);
		break;
	}
	default:
		// The solution will still be created and initializied with 0.
		Solution.Fill(0.0);
		assert(false);
		break;
	}
	return Solution;
}

///	<summary> Calculate the image or sub-image positions for an image group. </summary>
///	<returns> The success of the operation. </returns>
///	<param name="minMaxImageIndexes"> The minimum and maximum image indexes of the image group. </param>
///	<param name="nSubImagesPerImageWithGap"> The number of sub-images per image (along the y-dimension), including the gap. </param>
///	<param name="nSubimageRowOffset"> An offset value to add to sub-image indexes to account for the gap part preceding each image. </param>
///	<param name="nColumnCount"> The number of sub-images per image (along the x-dimension). </param>
///	<param name="pSolution"> A 2-column matrix containing x- and y-coordinates of the calculated positions upon return; must be correctly allocated and may be initialized with position estimations before the call. </param>
///	<param name="LocalRegistrationSolutions"> The image or sub-image registration results for the image group. </param>
bool CHRTGlobalPositioning::SolvePositioning(
	const index_pair_t& minMaxImageIndexes,
	size_t nSubImagesPerImageWithGap,
	size_t nSubimageRowOffset,
	size_t nColumnCount,
	CDenseMatrix* pSolution,
	const vector<CRigidRegistrationResult>& LocalRegistrationSolutions)
{
	size_t nImages = minMaxImageIndexes.second - minMaxImageIndexes.first + 1;

	assert(pSolution->Rows() == nSubImagesPerImageWithGap * nImages * nColumnCount);
	assert(pSolution->Cols() == 2);

	// Initialize matrices
	std::shared_ptr<CAbstractMatrix> pWtW = nullptr;
	std::shared_ptr<CDenseMatrix> pWtd = nullptr;

	size_t mSize = nSubImagesPerImageWithGap * nImages * nColumnCount;
	GenerateWtW(m_Parameters.eAlgorithm, pWtW, mSize);
	GenerateWtd(pWtd, mSize);

	AddSystemData(
		minMaxImageIndexes,
		LocalRegistrationSolutions,
		nSubImagesPerImageWithGap,
		nSubimageRowOffset,
		nColumnCount,
		*pWtW,
		*pWtd,
		m_Parameters.eProcessType
	);

	GenerateRegularisation(m_Parameters.eProcessType, nImages, nSubImagesPerImageWithGap, nColumnCount, *pWtW);

	AssignAnchorImage(m_Parameters.eAlgorithm, *pWtW);

	// Lösen des Gleichungssystems
	bool bSuccess = true;
	switch (m_Parameters.eAlgorithm)
	{
	case CSLESolver::EAlgorithm::eInverse:
	{
		CDenseMatrix invA = static_cast<CDenseMatrix*>(pWtW.get())->Inverse();

		if (invA.Cols() == 0 || invA.Rows() == 0)
		{
			bSuccess = false;
		}
		else
		{
			*pSolution = invA * (*pWtd);
		}
	}
	break;
	case CSLESolver::EAlgorithm::eGauss:
	{
		bSuccess = CSLESolver::SolveEquationGauss(*pWtW, *pWtd, *pSolution);
	}
	break;
	case CSLESolver::EAlgorithm::eRigidStartIteration:
	case CSLESolver::EAlgorithm::eJacobi:
	{
		CSparseMatrix* pSWtW = static_cast<CSparseMatrix*>(pWtW.get());
		pSWtW->Flush();

		CSLESolver::CIterationParameters param = CSLESolver::CreateIterationParameters(m_Parameters.eAlgorithm);
		bSuccess = CSLESolver::SolveEquationJacobi(*pSWtW, *pWtd, *pSolution, param);
	}
	break;
	case CSLESolver::EAlgorithm::eCGStartIteration:
	case CSLESolver::EAlgorithm::eCG:
	{
		CSLESolver::CIterationParameters param = CSLESolver::CreateIterationParameters(m_Parameters.eAlgorithm);
		CSparseMatrix* pSWtW = static_cast<CSparseMatrix*>(pWtW.get());
		pSWtW->Flush();

		bSuccess = CSLESolver::SolveEquationCG(*pSWtW, *pWtd, *pSolution, param, true);
	}
	break;
	case CSLESolver::EAlgorithm::eCGALGLIBStartIteration:
	case CSLESolver::EAlgorithm::eCGALGLIB:
	{
		CSLESolver::CIterationParameters param = CSLESolver::CreateIterationParameters(m_Parameters.eAlgorithm);
		CSparseMatrix* pSWtW = static_cast<CSparseMatrix*>(pWtW.get());
		pSWtW->Flush();
		bSuccess = CSLESolver::SolveEquationCGALGLIB(*pSWtW, *pWtd, *pSolution, param);

	}
	break;

	default:
		bSuccess = false;
	}

	if (!bSuccess)
	{
		pSolution->Fill(0.0);
	}

	return bSuccess;
}

///	<summary> Calculate the image or sub-image positions for an image group with an included iterative process to guarantee consistency of the registration results. </summary>
///	<returns> The success of the operation. </returns>
///	<param name="minMaxImageIndexes"> The minimum and maximum image indexes of the image group. </param>
///	<param name="nSubImagesPerImageWithGap"> The number of sub-images per image (along the y-dimension), including the gap. </param>
///	<param name="nSubimageRowOffset"> An offset value to add to sub-image indexes to account for the gap part preceding each image. </param>
///	<param name="pSolution"> A 2-column matrix containing x- and y-coordinates of the calculated positions upon return; must be correctly allocated and may be initialized with position estimations before the call. </param>
///	<param name="LocalRegistrationSolutions"> The image or sub-image registration results for the image group. </param>
///	<param name="fThreshold"> A threshold value for the residuals of the registration results to evaluate their consistency. </param>
bool CHRTGlobalPositioning::SolvePositioningWithConsistencyCheck(
	const index_pair_t& minMaxImageIndexes,
	size_t nSubImagesPerImageWithGap,
	size_t nSubimageRowOffset,
	CDenseMatrix* pSolution,
	vector<CRigidRegistrationResult>& LocalRegistrationSolutions,
	double fThreshold /* = 5.0 */)
{
	std::list<CDenseMatrix> listSolutions;
	std::list<CDenseVector> listResiduals;

	bool bSuccess;

	while (true)
	{
		bSuccess = SolvePositioning(minMaxImageIndexes, nSubImagesPerImageWithGap, nSubimageRowOffset, 1, pSolution, LocalRegistrationSolutions);
		if (!bSuccess)
		{
			break;
		}

		listSolutions.push_back(*pSolution);

		CDenseVector residuals(LocalRegistrationSolutions.size());

		// calculate maximum residual
		double fMaxResidual = 0;
		size_t    nMaxResidualIndex = 0;
		for (size_t i = 0; i < LocalRegistrationSolutions.size(); i++)
		{
			const CRigidRegistrationResult& reg = LocalRegistrationSolutions[i];
			double fResidual = 0;

			// trust all registration results with scores greater than or equal to 40
			if ((reg.GetValidity() > 0) && (reg.GetScore() < 40))
			{
				double dx = abs(abs(reg.GetX()) - abs((*pSolution)[reg.GetReferenceImageIndex()][0] - (*pSolution)[reg.GetTemplateImageIndex()][0]));
				double dy = abs(abs(reg.GetY()) - abs((*pSolution)[reg.GetReferenceImageIndex()][1] - (*pSolution)[reg.GetTemplateImageIndex()][1]));
				fResidual = std::max(dx, dy);
				if (fResidual > fMaxResidual)
				{
					fMaxResidual = fResidual;
					nMaxResidualIndex = static_cast<int>(i);
				}
			}
			residuals[i] = fResidual;
		}
		listResiduals.push_back(residuals);

		if (fMaxResidual < fThreshold)
		{
			assert(bSuccess);
			break;
		}
		else
		{
			// throw worst result away and reiterate
			assert(nMaxResidualIndex >= 0);
			LocalRegistrationSolutions[nMaxResidualIndex].SetValidity(CHrtValidityCodes::eConsistencyCheckFailed);
			pSolution->Fill(0);
		}
	}

	// Assemble consistency check matrices
	size_t i = 0;
	CDenseMatrix matResiduals(LocalRegistrationSolutions.size(), listResiduals.size());
	while (!listResiduals.empty())
	{
		assert(listResiduals.front().Size() == LocalRegistrationSolutions.size());
		matResiduals.SetCol(listResiduals.front(), i);
		listResiduals.pop_front();
		i++;
	}

	i = 0;
	CDenseMatrix matSolutions(pSolution->Rows(), 2 * listSolutions.size());
	while (!listSolutions.empty())
	{
		assert(listSolutions.front().Rows() == matSolutions.Rows());
		assert(listSolutions.front().Cols() == 2);
		matSolutions.CopyRoi(0, 2 * i, listSolutions.front(), 0, 0, matSolutions.Rows(), 2);
		listSolutions.pop_front();
		i++;
	}

	return bSuccess;
}

///	<summary> Create the system matrix based on the registration results. </summary>
///	<param name="minMaxImageIndexes"> The minimum and maximum image indexes of the image group. </param>
///	<param name="LocalRegistrationSolution"> The image or sub-image registration results for the image group. </param>
///	<param name="SubImagesRowCount"> The number of sub-images per image (along the y-dimension), including the gap. </param>
///	<param name="nSubimageRowOffset"> An offset value to add to sub-image indexes to account for the gap part preceding each image. </param>
///	<param name="SubImagesColumnCount"> The number of sub-images per image (along the x-dimension). </param>
///	<param name="wtw"> A square matrix, containing the left-hand sides of the normal equations. </param>
///	<param name="wtd"> A 2-column matrix (for the x- and y-coordinates),  containing the right-hand sides of the normal equations. </param>
///	<param name="eProcessType"> The processing template for the solution process. </param>
void CHRTGlobalPositioning::AddSystemData(
	const index_pair_t& minMaxImageIndexes,
	const vector<CRigidRegistrationResult>& LocalRegistrationSolution,
	size_t SubImagesRowCount,
	size_t nSubimageRowOffset,
	size_t SubImagesColumnCount,
	CAbstractMatrix& wtw,
	CDenseMatrix& wtd,
	CProcessType::EProcessType eProcessType)
{
	size_t nMinImageIndex = minMaxImageIndexes.first;
	size_t nMaxImageIndex = minMaxImageIndexes.second;
	size_t nImages = nMaxImageIndex - nMinImageIndex + 1;

	//unkritisch, da alle Schleifendurchläufe voneinder unabhängig sind
	for (auto RegResult : LocalRegistrationSolution)
	{
		if (RegResult.GetValidity() <= 0)
			continue;

		if ((RegResult.GetReferenceImageIndex() < nMinImageIndex) ||
			(RegResult.GetReferenceImageIndex() > nMaxImageIndex) ||
			(RegResult.GetTemplateImageIndex() < nMinImageIndex) ||
			(RegResult.GetTemplateImageIndex() > nMaxImageIndex))
			throw std::out_of_range("CHRTGlobalPositioning: Registration result image index outside image group index range");

		// Adjust the image indexes so that they correctly refer to the matrix positions
		RegResult.SetReferenceImageIndex(RegResult.GetReferenceImageIndex() - nMinImageIndex);
		RegResult.SetTemplateImageIndex(RegResult.GetTemplateImageIndex() - nMinImageIndex);

		// Don't ask. It is being negated. That's how it works.
		RegResult.Negate();

		float x = RegResult.GetX();
		float y = RegResult.GetY();
		size_t nIndexRef = RegResult.GetReferenceImageIndex();
		size_t nIndexTempl = RegResult.GetTemplateImageIndex();
		size_t nSubImageRowIndex = RegResult.GetSubImageRowIndex() + static_cast<long>(nSubimageRowOffset);
		size_t nSubHeight = RegResult.GetSubImageHeight();

		switch (eProcessType)
		{
		case CProcessType::EProcessType::eHRTImageRegistration:
		case CProcessType::EProcessType::eHRTStreamRegistration:
			AddEntrySLE(wtw, wtd, RegResult, nSubimageRowOffset, SubImagesRowCount, nImages);
			break;
		case CProcessType::EProcessType::eBlockBasedRegistration:
			AddEntrySLE2D(wtw, wtd, x, y, nIndexRef, nIndexTempl, SubImagesColumnCount, SubImagesRowCount, RegResult.GetBlockIndex());
			break;
		case CProcessType::EProcessType::eRigidRegistration:
			AddEntrySLERigidRegistration(wtw, wtd, x, y, nIndexRef, nSubImageRowIndex, nIndexTempl, nSubHeight, SubImagesRowCount, nImages);
			break;
		default:
			assert(false);
			break;
		}
	}
}

void CHRTGlobalPositioning::GenerateRegularisation(CProcessType::EProcessType eProcessType, size_t nImages, size_t nSubImagesPerImageWithGap, size_t nColumnCount, CAbstractMatrix& WtW)
{
	if (eProcessType == CProcessType::EProcessType::eHRTImageRegistration)
	{
		// local (image-level) curvature and gradient constraints
		GenerateConstraintCurvatureSystem(nSubImagesPerImageWithGap, nImages, WtW, s_fLocalCurvatureCorrectionParameter);
		GenerateConstraintGradientSystem(nSubImagesPerImageWithGap, nImages, WtW, s_fLocalGradientCorrectionParameter);
	}
	else if (eProcessType == CProcessType::EProcessType::eHRTStreamRegistration)
	{
		// local (image-level) curvature and gradient constraints
		GenerateConstraintCurvatureSystem(nSubImagesPerImageWithGap, nImages, WtW, s_fLocalCurvatureCorrectionParameter);
		GenerateConstraintGradientSystem(nSubImagesPerImageWithGap, nImages, WtW, s_fLocalGradientCorrectionParameter);

		// global (sequence-level) curvature and gradient constraints
		GenerateConstraintCurvatureSystem(nSubImagesPerImageWithGap * nImages, 1, WtW, s_fGlobalCurvatureCorrectionParameter);
		GenerateConstraintGradientSystem(nSubImagesPerImageWithGap * nImages, 1, WtW, s_fGlobalGradientCorrectionParameter);
	}
	else if (eProcessType == CProcessType::EProcessType::eBlockBasedRegistration)
	{
		GenerateConstraintCurvatureSystem2D(nColumnCount, nSubImagesPerImageWithGap, nImages, WtW, s_fLocalCurvatureCorrectionParameter);
	}
}

void CHRTGlobalPositioning::GenerateWtW(CSLESolver::EAlgorithm eAlgorithm, std::shared_ptr<CAbstractMatrix>& pWtW, size_t mSize)
{
	switch (eAlgorithm)
	{
	case CSLESolver::EAlgorithm::eCG:
	case CSLESolver::EAlgorithm::eCGStartIteration:
	case CSLESolver::EAlgorithm::eCGALGLIB:
	case CSLESolver::EAlgorithm::eCGALGLIBStartIteration:
	case CSLESolver::EAlgorithm::eJacobi:
	case CSLESolver::EAlgorithm::eRigidStartIteration:
		pWtW = std::make_shared<CSparseMatrix>(mSize, mSize);
		break;

	case CSLESolver::EAlgorithm::eGauss:
	case CSLESolver::EAlgorithm::eInverse:
	default:
		pWtW = std::make_shared<CDenseMatrix>(mSize, mSize);
		break;
	}
	pWtW->Fill(0.0);
}

void CHRTGlobalPositioning::GenerateWtd(std::shared_ptr<CDenseMatrix>& pWtd, size_t mSize)
{
	pWtd = std::make_shared<CDenseMatrix>(mSize, 2);
	pWtd->Fill(0.0);
}

void CHRTGlobalPositioning::AssignAnchorImage(CSLESolver::EAlgorithm eAlgorithm, CAbstractMatrix& WtW)
{
	// Depending on the used algorithm the coordinate system gets centered using two differend methods
	// This is due to huge efficeny lacks when using the old methods with iterative approaches.

	// Either the first image is set as anchor image
	// Or the coordinate system is set to the middle.
	// Or nothing is done
	switch (eAlgorithm)
	{
	case CSLESolver::EAlgorithm::eJacobi:
	case CSLESolver::EAlgorithm::eRigidStartIteration:
	case CSLESolver::EAlgorithm::eCG:
	case CSLESolver::EAlgorithm::eCGStartIteration:
	case CSLESolver::EAlgorithm::eCGALGLIB:
	case CSLESolver::EAlgorithm::eCGALGLIBStartIteration:
		WtW.SetValueAt(0, 0, WtW.GetValueAt(0, 0) + 1);
		break;

	case CSLESolver::EAlgorithm::eGauss:
	case CSLESolver::EAlgorithm::eInverse:
	default:
		WtW += s_fEpsilon1;
		break;
	}

	for (size_t jd = 0; jd < WtW.Cols(); jd++)
	{
		WtW.SetValueAt(jd, jd, WtW.GetValueAt(jd, jd) + s_fEpsilon2);
	}
}


/**	\brief Generiert eine Matrix zur Regularisierung eines LGS.
 *
 *	Die Regularisierung ist notwendig, wenn an manchen Positionen keine
 *	Verschiebungsfunktionen vorliegen und folglich das System singulär würde.
 *	Durch die Regularisierungsmatrix werden bei der Lösung des LGS an den
 *	Fehlstellen die Ergebnisdaten mit einer Kurve interpoliert, deren
 *	Krümmung minimiert wird.
 *
 *	\return Die generierte Matrix.
 *
 *	\param[in] nSubImages Die Anzahl der Teilbilder.
 *	\param[in] nImages Die Anzahl der Bilder.
 */
void CHRTGlobalPositioning::GenerateConstraintCurvatureSystem(size_t nSubImages, size_t nImages, CAbstractMatrix& m, double fLambda)
{
	assert(nSubImages * nImages == m.Rows());
	assert(nSubImages * nImages == m.Cols());
	assert(fLambda >= 0.0);

	// A zero weight factor has no effect
	if (fLambda == 0.0)
	{
		return;
	}

	// Curvature calculation for less than three elements does not make sense
	// IMPORTANT NOTE: This limit is also critical for the unsigned arithmetics on nSubImages below to work correctly!
	if (nSubImages < 3)
	{
		return;
	}

	// Please don't parallelize this, it's not worth it
	for (size_t i = 0; i < nImages; i++)
	{
		size_t base = i * nSubImages;

		// Values along the main diagonal
		for (size_t si = 2; si <= nSubImages - 3; si++)
		{
			m.SetValueAt(base + si, base + si, m.GetValueAt(base + si, base + si) + 6 * fLambda);
		}

		// Values along the first diagonals
		for (size_t si = 1; si <= nSubImages - 3; si++)
		{
			m.SetValueAt(base + si, base + si + 1, m.GetValueAt(base + si, base + si + 1) - 4 * fLambda);
			m.SetValueAt(base + si + 1, base + si, m.GetValueAt(base + si + 1, base + si) - 4 * fLambda);
		}

		// Values along the second diagonals
		for (size_t si = 1; si <= nSubImages - 2; si++)
		{
			m.SetValueAt(base + si - 1, base + si + 1, m.GetValueAt(base + si - 1, base + si + 1) + 1 * fLambda);
			m.SetValueAt(base + si + 1, base + si - 1, m.GetValueAt(base + si + 1, base + si - 1) + 1 * fLambda);
		}

		// Border handling: first and last values on main and first diagonals
		m.SetValueAt(base, base, m.GetValueAt(base, base) + 1 * fLambda);
		m.SetValueAt(base, base + 1, m.GetValueAt(base, base + 1) - 2 * fLambda);
		m.SetValueAt(base + 1, base, m.GetValueAt(base + 1, base) - 2 * fLambda);
		m.SetValueAt(base + nSubImages - 1, base + nSubImages - 1, m.GetValueAt(base + nSubImages - 1, base + nSubImages - 1) + 1 * fLambda);
		m.SetValueAt(base + nSubImages - 2, base + nSubImages - 1, m.GetValueAt(base + nSubImages - 2, base + nSubImages - 1) - 2 * fLambda);
		m.SetValueAt(base + nSubImages - 1, base + nSubImages - 2, m.GetValueAt(base + nSubImages - 1, base + nSubImages - 2) - 2 * fLambda);

		// Border handling: second and second-to-last values on main diagonal
		if (nSubImages > 3)
		{
			m.SetValueAt(base + 1, base + 1, m.GetValueAt(base + 1, base + 1) + 5 * fLambda);
			m.SetValueAt(base + nSubImages - 2, base + nSubImages - 2, m.GetValueAt(base + nSubImages - 2, base + nSubImages - 2) + 5 * fLambda);
		}
		else
		{
			// The boundary case of three subimages needs to be handled separately
			// assert(nSubImages == 3);
			m.SetValueAt(base + 1, base + 1, m.GetValueAt(base + 1, base + 1) + 4 * fLambda);
		}
	}
}

void CHRTGlobalPositioning::GenerateConstraintGradientSystem(size_t nSubImages, size_t nImages, CAbstractMatrix& m, double fLambda)
{
	assert(nSubImages * nImages == m.Rows());
	assert(nSubImages * nImages == m.Cols());
	assert(fLambda >= 0.0);

	// A zero weight factor has no effect
	if (fLambda == 0.0)
	{
		return;
	}

	// Gradient calculation for less than two elements does not make sense
	// IMPORTANT NOTE: This limit is also critical for the unsigned arithmetics on nSubImages below to work correctly!
	if (nSubImages < 2)
	{
		return;
	}

	// Please don't parallelize this, it's not worth it
	for (size_t i = 0; i < nImages; i++)
	{
		size_t base = i * nSubImages;

		// Values along the main diagonal
		for (size_t si = 1; si <= nSubImages - 2; si++)
		{
			m.SetValueAt(base + si, base + si, m.GetValueAt(base + si, base + si) + 2 * fLambda);
		}

		// Values along the first diagonals
		for (size_t si = 0; si <= nSubImages - 2; si++)
		{
			m.SetValueAt(base + si, base + si + 1, m.GetValueAt(base + si, base + si + 1) - 1 * fLambda);
			m.SetValueAt(base + si + 1, base + si, m.GetValueAt(base + si + 1, base + si) - 1 * fLambda);
		}

		// Border handling: first and last value on main diagonal
		m.SetValueAt(base, base, m.GetValueAt(base, base) + 1 * fLambda);
		m.SetValueAt(base + nSubImages - 1, base + nSubImages - 1, m.GetValueAt(base + nSubImages - 1, base + nSubImages - 1) + 1 * fLambda);
	}
}

void CHRTGlobalPositioning::GenerateConstraintCurvatureSystem2D(size_t nBlocksX, size_t nBlocksY, size_t nImages, CAbstractMatrix& m, double fLambda)
{
	assert(nBlocksX * nBlocksY * nImages == m.Rows());
	assert(nBlocksX * nBlocksY * nImages == m.Cols());
	assert(fLambda >= 0.0);

	// A zero weight factor has no effect
	if (fLambda == 0.0)
	{
		return;
	}

	// Curvature calculation for less than three elements does not make sense
	if ((nBlocksX < 3) && (nBlocksY < 3))
	{
		return;
	}

	// Please don't parallelize this, it's not worth it
	for (size_t i = 0; i < nImages; i++)
	{
		size_t base = i * nBlocksX * nBlocksY;

		/////////////////////////////////////////////////////////////////////
		// Constraint system for horizontally neighboring blocks           //
		// -----------------------------------------------------           //
		// This applies the second derivative (1 -4 6 -4 1) to all         //
		// (nx*nx)-blocks along the main diagonal of matrix m. Because of  //
		// the length of 5 of the second deriavtive operator, matrix       //
		// values along the main diagonal and the first and second         //
		// diagonals inside each of the blocks are affected.               //
		/////////////////////////////////////////////////////////////////////

		// Curvature calculation for less than three elements does not make sense
		// IMPORTANT NOTE: This limit is also critical for the unsigned arithmetics on nBlocksX below to work correctly!
		if (nBlocksX >= 3)
		{
			// All blocks along main diagonal
			for (size_t y = 0; y < nBlocksY; y++)
			{
				// Values along the main diagonal
				for (size_t x = 2; x <= nBlocksX - 3; x++)
				{
					m.SetValueAt(base + y * nBlocksX + x, base + y * nBlocksX + x, m.GetValueAt(base + y * nBlocksX + x, base + y * nBlocksX + x) + 6 * fLambda);
				}

				// Values along the first diagonals
				for (size_t x = 1; x <= nBlocksX - 3; x++)
				{
					m.SetValueAt(base + y * nBlocksX + x, base + y * nBlocksX + (x + 1), m.GetValueAt(base + y * nBlocksX + x, base + y * nBlocksX + (x + 1)) - 4 * fLambda);
					m.SetValueAt(base + y * nBlocksX + (x + 1), base + y * nBlocksX + x, m.GetValueAt(base + y * nBlocksX + (x + 1), base + y * nBlocksX + x) - 4 * fLambda);
				}

				// Values along the second diagonals
				for (size_t x = 1; x <= nBlocksX - 2; x++)
				{
					m.SetValueAt(base + y * nBlocksX + (x - 1), base + y * nBlocksX + (x + 1), m.GetValueAt(base + y * nBlocksX + (x - 1), base + y * nBlocksX + (x + 1)) + 1 * fLambda);
					m.SetValueAt(base + y * nBlocksX + (x + 1), base + y * nBlocksX + (x - 1), m.GetValueAt(base + y * nBlocksX + (x + 1), base + y * nBlocksX + (x - 1)) + 1 * fLambda);
				}

				// Border handling: first and last values on main and first diagonals
				m.SetValueAt(base + y * nBlocksX, base + y * nBlocksX, m.GetValueAt(base + y * nBlocksX, base + y * nBlocksX) + 1 * fLambda);
				m.SetValueAt(base + y * nBlocksX, base + y * nBlocksX + 1, m.GetValueAt(base + y * nBlocksX, base + y * nBlocksX + 1) - 2 * fLambda);
				m.SetValueAt(base + y * nBlocksX + 1, base + y * nBlocksX, m.GetValueAt(base + y * nBlocksX + 1, base + y * nBlocksX) - 2 * fLambda);
				m.SetValueAt(base + y * nBlocksX + (nBlocksX - 1), base + y * nBlocksX + (nBlocksX - 1), m.GetValueAt(base + y * nBlocksX + (nBlocksX - 1), base + y * nBlocksX + (nBlocksX - 1)) + 1 * fLambda);
				m.SetValueAt(base + y * nBlocksX + (nBlocksX - 2), base + y * nBlocksX + (nBlocksX - 1), m.GetValueAt(base + y * nBlocksX + (nBlocksX - 2), base + y * nBlocksX + (nBlocksX - 1)) - 2 * fLambda);
				m.SetValueAt(base + y * nBlocksX + (nBlocksX - 1), base + y * nBlocksX + (nBlocksX - 2), m.GetValueAt(base + y * nBlocksX + (nBlocksX - 1), base + y * nBlocksX + (nBlocksX - 2)) - 2 * fLambda);

				// Border handling: second and second-to-last values on main diagonal
				if (nBlocksX > 3)
				{
					m.SetValueAt(base + y * nBlocksX + 1, base + y * nBlocksX + 1, m.GetValueAt(base + y * nBlocksX + 1, base + y * nBlocksX + 1) + 5 * fLambda);
					m.SetValueAt(base + y * nBlocksX + (nBlocksX - 2), base + y * nBlocksX + (nBlocksX - 2), m.GetValueAt(base + y * nBlocksX + (nBlocksX - 2), base + y * nBlocksX + (nBlocksX - 2)) + 5 * fLambda);
				}
				else
				{
					// The boundary case of three blocks needs to be handled separately
					// assert(nBlocksX == 3);
					m.SetValueAt(base + y * nBlocksX + 1, base + y * nBlocksX + 1, m.GetValueAt(base + y * nBlocksX + 1, base + y * nBlocksX + 1) + 4 * fLambda);
				}
			}
		}

		/////////////////////////////////////////////////////////////////////
		// Constraint system for vertically neighboring blocks             //
		// ---------------------------------------------------             //
		// This applies the second derivative (1 -4 6 -4 1) to             //
		// (nx*nx)-blocks along the main diagonal and the first and second //
		// block diagonals of matrix m. In effect the value 6 affects      //
		// blocks along the main diagonal, the value -4 affects blocks     //
		// along the first block diagonal and the value 1 affects blocks   //
		// along the second block diagonal of m. The values always affect  //
		// the entire main diagonal of their respective blocks.            //
		/////////////////////////////////////////////////////////////////////

		// Curvature calculation for less than three elements does not make sense
		// IMPORTANT NOTE: This limit is also critical for the unsigned arithmetics on nBlocksY below to work correctly!
		if (nBlocksY >= 3)
		{
			// All blocks along main diagonal
			for (size_t x = 0; x < nBlocksX; x++)
			{
				// Values along the main diagonal
				for (size_t y = 2; y <= nBlocksY - 3; y++)
				{
					m.SetValueAt(base + y * nBlocksX + x, base + y * nBlocksX + x, m.GetValueAt(base + y * nBlocksX + x, base + y * nBlocksX + x) + 6 * fLambda);
				}

				// Values along the first diagonals
				for (size_t y = 1; y <= nBlocksY - 3; y++)
				{
					m.SetValueAt(base + y * nBlocksX + x, base + (y + 1) * nBlocksX + x, m.GetValueAt(base + y * nBlocksX + x, base + (y + 1) * nBlocksX + x) - 4 * fLambda);
					m.SetValueAt(base + (y + 1) * nBlocksX + x, base + y * nBlocksX + x, m.GetValueAt(base + (y + 1) * nBlocksX + x, base + y * nBlocksX + x) - 4 * fLambda);
				}

				// Values along the second diagonals
				for (size_t y = 1; y <= nBlocksY - 2; y++)
				{
					m.SetValueAt(base + (y - 1) * nBlocksX + x, base + (y + 1) * nBlocksX + x, m.GetValueAt(base + (y - 1) * nBlocksX + x, base + (y + 1) * nBlocksX + x) + 1 * fLambda);
					m.SetValueAt(base + (y + 1) * nBlocksX + x, base + (y - 1) * nBlocksX + x, m.GetValueAt(base + (y + 1) * nBlocksX + x, base + (y - 1) * nBlocksX + x) + 1 * fLambda);
				}

				// Border handling: first and last values on main and first diagonals
				m.SetValueAt(base + x, base + x, m.GetValueAt(base + x, base + x) + 1 * fLambda);
				m.SetValueAt(base + x, base + nBlocksX + x, m.GetValueAt(base + x, base + nBlocksX + x) - 2 * fLambda);
				m.SetValueAt(base + nBlocksX + x, base + x, m.GetValueAt(base + nBlocksX + x, base + x) - 2 * fLambda);
				m.SetValueAt(base + (nBlocksY - 1) * nBlocksX + x, base + (nBlocksY - 1) * nBlocksX + x, m.GetValueAt(base + (nBlocksY - 1) * nBlocksX + x, base + (nBlocksY - 1) * nBlocksX + x) + 1 * fLambda);
				m.SetValueAt(base + (nBlocksY - 2) * nBlocksX + x, base + (nBlocksY - 1) * nBlocksX + x, m.GetValueAt(base + (nBlocksY - 2) * nBlocksX + x, base + (nBlocksY - 1) * nBlocksX + x) - 2 * fLambda);
				m.SetValueAt(base + (nBlocksY - 1) * nBlocksX + x, base + (nBlocksY - 2) * nBlocksX + x, m.GetValueAt(base + (nBlocksY - 1) * nBlocksX + x, base + (nBlocksY - 2) * nBlocksX + x) - 2 * fLambda);

				// Border handling: second and second-to-last values on main diagonal
				if (nBlocksY > 3)
				{
					m.SetValueAt(base + nBlocksX + x, base + nBlocksX + x, m.GetValueAt(base + nBlocksX + x, base + nBlocksX + x) + 5 * fLambda);
					m.SetValueAt(base + (nBlocksY - 2) * nBlocksX + x, base + (nBlocksY - 2) * nBlocksX + x, m.GetValueAt(base + (nBlocksY - 2) * nBlocksX + x, base + (nBlocksY - 2) * nBlocksX + x) + 5 * fLambda);
				}
				else
				{
					// The boundary case of three blocks needs to be handled separately
					// assert(nBlocksY == 3);
					m.SetValueAt(base + nBlocksX + x, base + nBlocksX + x, m.GetValueAt(base + nBlocksX + x, base + nBlocksX + x) + 4 * fLambda);
				}
			}
		}
	}
}

/**	\brief generiert aus U(y) die _T("Verknüpfungsmatrix") W und den Abweichungsvektor d für m Bilder mit n Unterteilungen im Abstand Sy.
 *
 *	\param[in] nSubImageHeight	Höhe eines Teilbildes (in Pixel)
 *	\param[in] nSubImagesCount	Anzahl der Teilbilder
 *	\param[in] nImageCount		Anzahl der Bilder
 *	\param[in] fMinimalScore		minimaler Score-Wert
 *	\param[in] differenceVector	Vektor der die Differenz enthält
 *	\param[in] LocalRegistrationSolutions	Ergebnisse der lokalen Registrierung
 *
 *	\return die generierte Matrix
 */
void CHRTGlobalPositioning::AddEntrySLE(CAbstractMatrix& WtW, CDenseMatrix& Wtd, float xOffset, float yOffset, size_t nReferenceImage, size_t nSubImageIndex, size_t nTemplateImage, size_t nSubImageHeight, size_t nSubImagesCount, size_t nImageCount)
{
	long long ny = static_cast<long long>(floor(yOffset / nSubImageHeight));
	double q = (yOffset / nSubImageHeight) - ny;
	long long nk = nSubImageIndex + ny;

	if (nk < 0)
		return;
	if (static_cast<size_t>(nk + 1) >= nSubImagesCount)
		return;


	double p = 1 - q;


	size_t ndiag = nReferenceImage * nSubImagesCount + nSubImageIndex;
	size_t np = nk + nSubImagesCount * nTemplateImage;
	size_t nq = np + 1;

	assert(nTemplateImage > 0);
	assert(np > 0);
	assert(nSubImagesCount > 0);

	if (nq >= nSubImagesCount * nImageCount)
		return;

	WtW.SetValueAt(ndiag, ndiag, WtW.GetValueAt(ndiag, ndiag) + 1);
	Wtd.SetValueAt(ndiag, 0, Wtd.GetValueAt(ndiag, 0) + xOffset);
	Wtd.SetValueAt(ndiag, 1, Wtd.GetValueAt(ndiag, 1) + yOffset);

	WtW.SetValueAt(ndiag, np, WtW.GetValueAt(ndiag, np) - p);
	WtW.SetValueAt(np, ndiag, WtW.GetValueAt(np, ndiag) - p);
	WtW.SetValueAt(ndiag, nq, WtW.GetValueAt(ndiag, nq) - q);
	WtW.SetValueAt(nq, ndiag, WtW.GetValueAt(nq, ndiag) - q);

	WtW.SetValueAt(np, np, WtW.GetValueAt(np, np) + pow(p, 2));
	WtW.SetValueAt(nq, nq, WtW.GetValueAt(nq, nq) + pow(q, 2));

	WtW.SetValueAt(nq, np, WtW.GetValueAt(nq, np) + p * q);
	WtW.SetValueAt(np, nq, WtW.GetValueAt(np, nq) + p * q);

	Wtd.SetValueAt(np, 0, Wtd.GetValueAt(np, 0) - xOffset * p);
	Wtd.SetValueAt(nq, 0, Wtd.GetValueAt(nq, 0) - xOffset * q);
	Wtd.SetValueAt(np, 1, Wtd.GetValueAt(np, 1) - yOffset * p);
	Wtd.SetValueAt(nq, 1, Wtd.GetValueAt(nq, 1) - yOffset * q);
}

void CHRTGlobalPositioning::AddEntrySLE(CAbstractMatrix& WtW, CDenseMatrix& Wtd, const CRigidRegistrationResult& regResult, size_t nSubimageRowOffset, size_t SubImagesCount, size_t nImageCount)
{
	auto nReferenceImage = regResult.GetReferenceImageIndex();
	auto nTemplateImage = regResult.GetTemplateImageIndex();
	auto nSubImageIndex = regResult.GetSubImageRowIndex() + nSubimageRowOffset;
	auto nSubImageHeight = regResult.GetSubImageHeight();
	auto x = regResult.GetX();
	auto y = regResult.GetY();
	return AddEntrySLE(WtW, Wtd, x, y, nReferenceImage, nSubImageIndex, nTemplateImage, nSubImageHeight, SubImagesCount, nImageCount);
}

void CHRTGlobalPositioning::AddEntrySLERigidRegistration(CAbstractMatrix& WtW, CDenseMatrix& Wtd, float ux, float uy, size_t image1, size_t /*nSubImageIndex*/, size_t image2, size_t /*SubImagesAmount*/, size_t SubImagesCount, size_t /*nImageCount*/)
{
	size_t n1 = image1 * SubImagesCount;
	size_t n2 = image2 * SubImagesCount;

	WtW.SetValueAt(n1, n1, WtW.GetValueAt(n1, n1) + 1);
	Wtd.SetValueAt(n1, 0, Wtd.GetValueAt(n1, 0) + ux);
	Wtd.SetValueAt(n1, 1, Wtd.GetValueAt(n1, 1) + uy);

	WtW.SetValueAt(n1, n2, WtW.GetValueAt(n1, n2) - 1);
	WtW.SetValueAt(n2, n1, WtW.GetValueAt(n2, n1) - 1);

	WtW.SetValueAt(n2, n2, WtW.GetValueAt(n2, n2) + 1);

	Wtd.SetValueAt(n2, 0, Wtd.GetValueAt(n2, 0) - ux);
	Wtd.SetValueAt(n2, 1, Wtd.GetValueAt(n2, 1) - uy);
}

void CHRTGlobalPositioning::AddEntrySLE2D(CAbstractMatrix& WtW, CDenseMatrix& Wtd, float ux, float uy, size_t image1, size_t image2, size_t SubImageColumnCount, size_t SubImageRowCount, size_t nBlockNr)
{
	size_t n1 = image1 * SubImageRowCount * SubImageColumnCount + nBlockNr;
	size_t n2 = image2 * SubImageRowCount * SubImageColumnCount + nBlockNr;

	WtW.SetValueAt(n1, n1, WtW.GetValueAt(n1, n1) + 1);
	Wtd.SetValueAt(n1, 0, Wtd.GetValueAt(n1, 0) + ux);
	Wtd.SetValueAt(n1, 1, Wtd.GetValueAt(n1, 1) + uy);

	WtW.SetValueAt(n1, n2, WtW.GetValueAt(n1, n2) - 1);
	WtW.SetValueAt(n2, n1, WtW.GetValueAt(n2, n1) - 1);

	WtW.SetValueAt(n2, n2, WtW.GetValueAt(n2, n2) + 1);
	Wtd.SetValueAt(n2, 0, Wtd.GetValueAt(n2, 0) - ux);
	Wtd.SetValueAt(n2, 1, Wtd.GetValueAt(n2, 1) - uy);
}


/*
* Requires the destination Matrix to be preallocated with nImages*nSubImagesWithoutGap times 2
* It is not tested whether they can overlap. (I personally think they can, though...(L.T.)).
*/
void CHRTGlobalPositioning::RemoveGap(const CDenseMatrix& source, CDenseMatrix& destination, size_t nSubImagesWithoutGap, size_t nGapBefore, size_t nGapAfter)
{
	size_t nTotalSubImages = nSubImagesWithoutGap + nGapBefore + nGapAfter;
	assert(source.Rows() % nTotalSubImages == 0);
	size_t nImageCount = source.Rows() / nTotalSubImages;
	assert(destination.Rows() == nImageCount * nSubImagesWithoutGap);
	assert(destination.Cols() == source.Cols());

	size_t nGapPos = 0;
	for (size_t nImage = 0; nImage < nImageCount; nImage++)
	{
		for (size_t nSubImage = nGapBefore; nSubImage < nSubImagesWithoutGap + nGapBefore; nSubImage++)
		{
			for (size_t nCol = 0; nCol < source.Cols(); nCol++)
			{
				destination[nGapPos][nCol] = source[nImage * nTotalSubImages + nSubImage][nCol];
			}
			nGapPos++;
		}
	}
}

void CHRTGlobalPositioning::RemoveGap(CDenseMatrix& inPlace, size_t nSubImagesWithoutGap, size_t nGapBefore, size_t nGapAfter)
{
	CDenseMatrix temp;
	temp.Copy(inPlace);

	size_t nTotalSubImages = nSubImagesWithoutGap + nGapBefore + nGapAfter;
	size_t nImageCount = inPlace.Rows() / nTotalSubImages;
	inPlace.AllocMatrix(nImageCount * nSubImagesWithoutGap, temp.Cols());

	return RemoveGap(temp, inPlace, nSubImagesWithoutGap, nGapBefore, nGapAfter);
}

///	<summary> Remove image position from solution for images that are not part of the image group. </summary>
///	<param name="source"> A 2-column matrix containing x- and y-coordinates of the calculated positions. </param>
///	<param name="destination"> A 2-column matrix containing x- and y-coordinates of the image or sub-image positions of the image group upon return; must be correctly allocated before the call. </param>
///	<param name="nSubImagesWithoutGap"> The number of sub-images per image (along the y-dimension), not including the gap. </param>
///	<param name="group"> The image indexes of the image group. </param>
void CHRTGlobalPositioning::RemoveExcludedImages(const CDenseMatrix& source, CDenseMatrix& destination, size_t nSubImagesWithoutGap, const std::list<size_t>& group)
{
	assert(source.Rows() >= group.size() * nSubImagesWithoutGap);
	assert(source.Cols() == 2);
	assert(destination.Rows() == group.size() * nSubImagesWithoutGap);
	assert(destination.Cols() == 2);

	size_t min = group.front();
	size_t row = 0;
	for (auto it = group.begin(); it != group.end(); it++)
	{
		size_t nImage = *it - min;
		for (size_t nSubImage = 0; nSubImage < nSubImagesWithoutGap; nSubImage++)
		{
			for (size_t col = 0; col < source.Cols(); col++)
			{
				destination[row][col] = source[nImage * nSubImagesWithoutGap + nSubImage][col];
			}
			row++;
		}
	}
	assert(row == destination.Rows());
}

///	<summary> Remove image position from solution for images that are not part of the image group. </summary>
///	<param name="inPlace"> A 2-column matrix containing x- and y-coordinates of the calculated positions before the call and the positions coordinates limited to the image group upon return. </param>
///	<param name="nSubImagesWithoutGap"> The number of sub-images per image (along the y-dimension), not including the gap. </param>
///	<param name="group"> The image indexes of the image group. </param>
void CHRTGlobalPositioning::RemoveExcludedImages(CDenseMatrix& inPlace, size_t nSubImagesWithoutGap, const std::list<size_t>& group)
{
	assert(inPlace.Rows() >= group.size() * nSubImagesWithoutGap);
	assert(inPlace.Cols() == 2);

	CDenseMatrix temp;
	temp.Copy(inPlace);

	inPlace.AllocMatrix(group.size() * nSubImagesWithoutGap, temp.Cols());

	return RemoveExcludedImages(temp, inPlace, nSubImagesWithoutGap, group);
}

CProgress CHRTGlobalPositioning::GetProgress()
{
	return CProgress();
}

void CHRTGlobalPositioning::OnCancel()
{
	// Intentionally empty.
}

void CHRTGlobalPositioning::OnUncancel()
{
	// Intentionally empty.
}

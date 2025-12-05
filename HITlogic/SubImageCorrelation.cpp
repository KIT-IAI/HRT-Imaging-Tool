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
#include "SubImageCorrelation.h"



const int CSubImageCorrelation::s_nNonBestSubImages = 2;

const int CSubImageCorrelation::s_nMaxIterations = 4;

const double CSubImageCorrelation::s_fHardConvergenceCriteria = 0.25;
const double CSubImageCorrelation::s_fWeakConvergenceCriteria = 1;

const double CSubImageCorrelation::s_fDivergenceCriteriaX = 1.25;
const double CSubImageCorrelation::s_fDivergenceCriteriaY = 0.375;

CSubImageCorrelation::CSubImageCorrelation(CCorrelationParameters& procedureParams, CImagePair& imageParams, CScoreContainer& scoreContainer, CRegistrationResult& correlationResult)
	:m_Images(imageParams),
	m_ProcedureParameter(procedureParams),
	m_ScoreContainer(scoreContainer),
	m_CorrelationResult(correlationResult),
	m_nPaddingSize(CalculatePaddingSize(procedureParams))
{

	m_ProcedureParameter.bSmallestPossibleFFT = true;
	if (m_ProcedureParameter.bConsistencyCheck)
		m_ProcedureParameter.eMaskRegArea = CCorrelationParameters::EMaskRegArea::eOn;
}

CSubImageCorrelation::~CSubImageCorrelation()
{
}

size_t CSubImageCorrelation::GetBestCorrelatedSubImage() const
{
	return m_nBestCorrelatedSubImage;
}

size_t CSubImageCorrelation::GetSubImageCount() const
{
	return m_Images.GetReferenceImage()->GetSize().y / GetSubImageHeight();
}

size_t CSubImageCorrelation::CalculatePaddingSize(const CCorrelationParameters& params)
{
	size_t FFT_Height;

	if (CMathTools::IsPowerOf2(params.nSubImageHeight))
		FFT_Height = params.nSubImageHeight * 2;
	else
		FFT_Height = CMathTools::NextPowOf2(params.nSubImageHeight);

	assert(FFT_Height >= params.nSubImageHeight);

	return (FFT_Height - params.nSubImageHeight) / 2;
}

void CSubImageCorrelation::FindBestCorrelatedRow()
{
	m_nBestCorrelatedRow = SearchBestCorrelatedRowInOverlappingArea(StlImagePoint(CMathTools::RoundValue(m_CorrelationResult.RigidRegistrationResult.GetX()), CMathTools::RoundValue(m_CorrelationResult.RigidRegistrationResult.GetY())));

	m_CorrelationResult.RigidRegistrationResult.SetSpecialImageRowIndex(m_nBestCorrelatedRow);

	// Because nBestCorrelatedRow is the the index of the best-correlated row
	// of the OVERLAPPING image region, the offset of this region must be
	// added if it doesn't coincide with the upper image boundary.
	if (m_CorrelationResult.RigidRegistrationResult.GetY() > 0)
	{
		m_nBestCorrelatedRow += static_cast<size_t>(m_CorrelationResult.RigidRegistrationResult.GetY());
	}
}

/*
 * Returns the index of the subimnage that contains the best correlated row
 */
void CSubImageCorrelation::FindBestCorrelatedSubImage()
{
	FindBestCorrelatedRow();

	m_nBestCorrelatedSubImage = m_nBestCorrelatedRow / GetSubImageHeight();

	ExcludeBorderSubImages();
}

/**	\brief Bestimmt aus zwei Bildern \a pReference und \a pTemplate mit
 *	bekanntem Versatz die am besten korrelierte Zeile.
 */
size_t CSubImageCorrelation::SearchBestCorrelatedRowInOverlappingArea(StlImagePoint Offset) const
{
	//auto&& SmoothedReferenceImage = m_Images.GetReferenceImageSmooth()==nullptr?SmoothImage(*m_Images.GetReferenceImage()): *m_Images.GetReferenceImageSmooth();
	//auto&& SmoothedTemplateImage = m_Images.GetTemplateImageSmooth()==nullptr?SmoothImage(*m_Images.GetTemplateImage()): *m_Images.GetTemplateImageSmooth();

	auto OverlappingRectangles = CImagePair::GetOverlappingRectangles(m_Images.GetReferenceImage()->GetSize(), m_Images.GetTemplateImage()->GetSize(), Offset);

	assert(!OverlappingRectangles.first.IsRectEmpty() && !OverlappingRectangles.second.IsRectEmpty());
	assert(OverlappingRectangles.first.Size() == OverlappingRectangles.second.Size());

	StlImage<float> OverlappingReferenceChild;
	if (m_Images.GetReferenceImageSmooth() == nullptr)	OverlappingReferenceChild.Child2dIndep(SmoothImage(*m_Images.GetReferenceImage()), OverlappingRectangles.first);
	else 	OverlappingReferenceChild.Child2dIndep(*m_Images.GetReferenceImageSmooth(), OverlappingRectangles.first);
	//OverlappingReferenceChild.Child2dIndep(SmoothedReferenceImage, OverlappingRectangles.first);
	StlImage<float> OverlappingTemplateChild;
	if (m_Images.GetTemplateImageSmooth() == nullptr)	OverlappingTemplateChild.Child2dIndep(SmoothImage(*m_Images.GetTemplateImage()), OverlappingRectangles.first);
	else 	OverlappingTemplateChild.Child2dIndep(*m_Images.GetTemplateImageSmooth(), OverlappingRectangles.second);
	//OverlappingTemplateChild.Child2dIndep(m_Images.GetTemplateImageSmooth() == nullptr ? SmoothImage(*m_Images.GetTemplateImage()) : *m_Images.GetTemplateImageSmooth(), OverlappingRectangles.second);

	assert(OverlappingReferenceChild.GetSize() == OverlappingTemplateChild.GetSize());

	// Calculate sum and difference image of overlapping area
	auto sumImage = OverlappingReferenceChild + OverlappingTemplateChild;
	auto differenceImage = OverlappingReferenceChild - OverlappingTemplateChild;

	auto deviations = CalculateDeviation(sumImage, differenceImage);

	// Smooth Deviations
	const int nMovingAverage = 8;
	MovingAverage(deviations, nMovingAverage);
	MovingAverage(deviations, nMovingAverage);

	// Get Minimum Deviation Row
	return static_cast<int>(std::min_element(deviations.begin(), deviations.end()) - deviations.begin());
}

StlImage<float> CSubImageCorrelation::SmoothImage(const StlImage<float>& image) const
{
	StlImage<float> SmoothImage;
	SmoothImage.Smooth(image);
	return SmoothImage;
}



/**	\brief Berechnet ein Korrelationsma� zwischen den Bildzeilen von
 *	\a sumImage und \a differenceImage.
 *
 *	Als Bewertung der Korrelation zwischen zwei korrespondierenden Bildzeilen
 *	\c s_i und \c d_i werden deren Standardabweichungen zueinander ins
 *	Verh�ltnis gesetzt, <tt>stddev(d_i)/stddev(s_i)</tt>.
 *
 *	\return Ein Vektor der berechneten Zeilen-Korrelationsma�e.
 *
 *	\param[in] sumImage,differenceImage Die beiden Eingangsbilder.
 *
 *	\note Die beiden Eingangsbilder m�ssen diesselbe Gr��e und einen Pixeltyp
 *	von <tt>M_FLOAT | 32</tt> aufweisen.
 */
std::vector<float> CSubImageCorrelation::CalculateDeviation(const StlImage<float>& sumImage, const StlImage<float>& differenceImage) const
{
	assert(sumImage.GetSize() == differenceImage.GetSize());

	auto OverlappingSize = sumImage.GetSize();

	vector<float> afStdDev(OverlappingSize.y);

	vector<float> bufferS = sumImage.AsArray();
	vector<float> bufferD = differenceImage.AsArray();

	auto itS = bufferS.cbegin();
	auto itD = bufferD.cbegin();

	vector<double> lineS(OverlappingSize.x);
	vector<double> lineD(OverlappingSize.x);

	for (size_t y = 0; y < OverlappingSize.y; y++)
	{
		for (size_t x = 0; x < OverlappingSize.x; x++)
		{
			lineS[x] = *itS; itS++;
			lineD[x] = *itD; itD++;
		}
		afStdDev[y] = static_cast<float>(CMathTools::StdDev(&lineD) / CMathTools::StdDev(&lineS));
	}

	assert(itS == bufferS.cend());
	assert(itD == bufferD.cend());

	return afStdDev;
}

/**	\brief Gl�ttet das Array \a vector mit dem gleitenden Durchschnitt.
 *
 *	\param[in,out] vector Das zu gl�ttende Array.
 *	\param[in] nNeighbours Anzahl der Nachbarwerte, die links- und
 *		rechtsseitig in den gleitenden Durchschnitt einbezogen werden sollen.
 */
void CSubImageCorrelation::MovingAverage(vector<float>& afStdDev, int nNeighbours) const
{
	size_t nSize = afStdDev.size();
	vector<float> solutionVector(nSize);

	size_t nMinIndex;
	size_t nMaxIndex;

	for (size_t i = 0; i < nSize; i++)
	{
		nMinIndex = (i > nNeighbours) ? (i - nNeighbours) : 0;
		nMaxIndex = std::min(i + nNeighbours, nSize - 1);

		float fSum = 0.0;
		for (size_t j = nMinIndex; j <= nMaxIndex; j++)
		{
			fSum += afStdDev[j];
		}

		solutionVector[i] = fSum / (nMaxIndex - nMinIndex + 1);
	}

	afStdDev.swap(solutionVector);
}

void CSubImageCorrelation::PrepareFlexibleResultArray()
{
	CRigidRegistrationResult localRegistrationStructure;
	localRegistrationStructure.SetX(m_CorrelationResult.RigidRegistrationResult.GetX());
	localRegistrationStructure.SetY(m_CorrelationResult.RigidRegistrationResult.GetY());
	localRegistrationStructure.SetReferenceImageIndex(m_Images.GetReferenceImageIndex());
	localRegistrationStructure.SetTemplateImageIndex(m_Images.GetTemplateImageIndex());
	localRegistrationStructure.SetSubImageHeight(GetSubImageHeight());
	localRegistrationStructure.SetSpecialSubImageRowIndex(m_nBestCorrelatedSubImage);
	localRegistrationStructure.SetValidity(CHrtValidityCodes::eValidityInitialization);

	assert(m_CorrelationResult.FlexibleRegistrationResults.empty());

	m_CorrelationResult.FlexibleRegistrationResults.reserve(GetSubImageCount());
	for (size_t i = 0; i < GetSubImageCount(); i++)
	{
		localRegistrationStructure.SetSubImageRowIndex(i);
		m_CorrelationResult.FlexibleRegistrationResults.push_back(localRegistrationStructure);
	}
}

void CSubImageCorrelation::PrepareOffsetArray()
{
	CVector2d<float> rigidOffset(m_CorrelationResult.RigidRegistrationResult.GetX(), m_CorrelationResult.RigidRegistrationResult.GetY());
	m_CorrelationResult.RowOffset.assign(m_Images.GetReferenceImage()->GetSize().y, rigidOffset);
}

void CSubImageCorrelation::AdjustTrajectory(CCorrelationParameters::EDirection direction)
{
	assert((direction == CCorrelationParameters::EDirection::eDown) || (direction == CCorrelationParameters::EDirection::eUp));
	assert((m_nBestCorrelatedSubImage >= 2) && (m_nBestCorrelatedSubImage < m_CorrelationResult.FlexibleRegistrationResults.size() - 2));

	if (direction == CCorrelationParameters::EDirection::eDown)
	{
		if (m_CorrelationResult.FlexibleRegistrationResults[m_nBestCorrelatedSubImage + 2].GetValidity() > 0)
		{
			m_nBestCorrelatedSubImage += 2;
		}
		else if (m_CorrelationResult.FlexibleRegistrationResults[m_nBestCorrelatedSubImage + 1].GetValidity() > 0)
		{
			m_nBestCorrelatedSubImage++;
		}
	}
	// Okay we have this block of code duplication here, but it makes it much more clear what is actually being done. If you have a better idea, change it.
	else if (direction == CCorrelationParameters::EDirection::eUp)
	{
		if (m_CorrelationResult.FlexibleRegistrationResults[m_nBestCorrelatedSubImage - 2].GetValidity() > 0)
		{
			m_nBestCorrelatedSubImage -= 2;
		}
		else if (m_CorrelationResult.FlexibleRegistrationResults[m_nBestCorrelatedSubImage - 1].GetValidity() > 0)
		{
			m_nBestCorrelatedSubImage--;
		}
	}
}

void CSubImageCorrelation::InitCorrelation()
{
	FindBestCorrelatedSubImage();
	PrepareFlexibleResultArray();
	PrepareOffsetArray();
}

bool CSubImageCorrelation::IsReferenceImageMaskingActive() const
{
	switch (m_ProcedureParameter.eMaskRegArea)
	{
	case CCorrelationParameters::EMaskRegArea::eOn:
		return true;
	case CCorrelationParameters::EMaskRegArea::eOff:
		return false;
	case CCorrelationParameters::EMaskRegArea::ePart:
		return !m_bIsFirstSubImage;
	default:
		assert(false);
		return false;
	}
}

void CSubImageCorrelation::SetScoreValues(size_t nImageIndex, CCorrelationOffset correlationOffset) const
{
	m_CorrelationResult.FlexibleRegistrationResults[nImageIndex].SetScore(correlationOffset.m_scoreNorm1);
	m_CorrelationResult.FlexibleRegistrationResults[nImageIndex].SetScoreBeforeNorm(correlationOffset.m_scoreBeforeNorm);
}

CImagePair CSubImageCorrelation::ExtractPartialImages(const StlImageRect rDefinedWindow) const
{
	bool bMask = IsReferenceImageMaskingActive();

	auto undistortedTemplateImage = InterpolateImage(*m_Images.GetTemplateImage(), m_CorrelationResult.RowOffset, rDefinedWindow);

	auto MaskedReferenceImage = ApplyReferenceImageMask(*m_Images.GetReferenceImage(), rDefinedWindow, bMask);
	auto MaskedTemplateImage = ApplyTemplateImageMask(undistortedTemplateImage, m_Images.GetReferenceImage()->GetSize(), rDefinedWindow, bMask);

	return CImagePair(std::move(MaskedReferenceImage), std::move(MaskedTemplateImage));
}

/*
 * Correlates the subiamge with the given index
 * returns false if the subimage does not overlap sufficiently
 */
bool CSubImageCorrelation::CorrelateSubImage(size_t nSubImageIndex, CCorrelationParameters::EDirection windowDirection)
{
	auto& LocalResult = m_CorrelationResult.FlexibleRegistrationResults[nSubImageIndex];

	std::list<CCorrelationOffset> OffsetsByIteration;

	for (int nIteration = 0; nIteration < s_nMaxIterations; nIteration++)
	{
		LocalResult.SetPhi(static_cast<float>(nIteration + 1));				// For testing purposes

		InterpolateSubImageRowPositions(nSubImageIndex, windowDirection);

		CCorrelationOffset correlationOffset = PerformSubImageCorrelation(nSubImageIndex);

		/****************************   NOTE   ****************************/
		/*                                                                */
		/* Until some point in the past, the iteration had been stopped   */
		/* (with a negative validity value, of course) if the calculated  */
		/* offset surmounted a threshold value (unconditionally fixed at  */
		/* 10 and 40 pixels for the y- and x-offset, respectively; these  */
		/* should at least depend on the sub-image height). This check    */
		/* was done for all but the first sub-image. Without this check,  */
		/* it may happen that larger offsets are calculated which lead to */
		/* extremely distorted sub-images in the registration process,    */
		/* that can in turn result in a large enough correlation score to */
		/* be accepted if they happen to converge.                        */
		/*                                                                */
		/******************************************************************/

		if (correlationOffset.GetValidity() < 0)		// Subimage has no sufficient overlap
		{
			LocalResult.SetValidity(correlationOffset.GetValidity());
			m_bIsFirstSubImage = false;
			return false;
		}

		ValidateSubImageOffset(nSubImageIndex, correlationOffset);
		AdjustOffsetValues(nSubImageIndex, correlationOffset);

		if (ResultHasDiverged(correlationOffset))
		{
			LocalResult.SetValidity(CHrtValidityCodes::eInvalidSubImageOffset);
			m_bIsFirstSubImage = false;
			return false;
		}

		OffsetsByIteration.push_back(correlationOffset);
		if (IterationProcessCanBeEnded(nSubImageIndex, OffsetsByIteration))
		{
			break;
		}
	}

	if (LocalResult.IsValid())
	{
		ExtrapolateOffset(nSubImageIndex, windowDirection);
	}

	m_bIsFirstSubImage = false;
	return true;
}

void CSubImageCorrelation::AdjustOffsetValues(size_t nSubImageIndex, const CCorrelationOffset& correlationOffset) const
{
	auto& LocalResult = m_CorrelationResult.FlexibleRegistrationResults[nSubImageIndex];
	LocalResult.SetDx(correlationOffset.xOffset);
	LocalResult.SetDy(correlationOffset.yOffset);

	LocalResult.SetX(LocalResult.GetX() + (LocalResult.GetDx()));
	LocalResult.SetY(LocalResult.GetY() + (LocalResult.GetDy()));
}

/*
 * Sets ScoreValues to the LocalRestult
 * and EClassification for the Offset-object
 */
void CSubImageCorrelation::ValidateSubImageOffset(size_t nSubImageIndex, CCorrelationOffset& correlationOffset) const
{
	auto& LocalResult = m_CorrelationResult.FlexibleRegistrationResults[nSubImageIndex];
	LocalResult.SetScore(correlationOffset.m_scoreNorm1);
	LocalResult.SetScoreBeforeNorm(correlationOffset.m_scoreBeforeNorm);
	m_ScoreContainer.GetOffsetClassifier().ClassifyFlexible(correlationOffset);
	//, m_Images.GetFFTSize(m_ProcedureParameter.fScaleReduction), m_CurrentSubImagePair.GetFFTSize()
}

/*
 * Returns false if further iterations are necessary
 */
bool CSubImageCorrelation::IterationProcessCanBeEnded(size_t nSubImageIndex, std::list<CCorrelationOffset> OffsetsByIteration)
{
	auto& LocalResult = m_CorrelationResult.FlexibleRegistrationResults[nSubImageIndex];
	const CCorrelationOffset& latestOffset = OffsetsByIteration.back();

	bool bIsLastIteration = (OffsetsByIteration.size() == s_nMaxIterations);
	bool bHasConverged = ResultHasConverged(latestOffset);
	bool bHasAlmostConverged = ResultHasAlmostConverged(latestOffset);
	bool bIsRegistrationValid = (latestOffset.m_EClassification >= EClassification::eCorrect);

	if (bHasConverged)
	{
		if (bIsRegistrationValid)
		{
			LocalResult.SetValidity(CHrtValidityCodes::eValidSubImageOffset);
		}
		else
		{
			LocalResult.SetValidity(CHrtValidityCodes::eInvalidMinScoreAfterAllIterations);
		}
		return true;
	}
	else
	{
		if (bIsLastIteration)
		{
			if (bIsRegistrationValid && bHasAlmostConverged)
			{
				LocalResult.SetValidity(CHrtValidityCodes::eValidSubImageOffset);
			}
			else
			{
				LocalResult.SetValidity(CHrtValidityCodes::eInvalidMaxNumberOfIterations);
			}
		}
		return bIsLastIteration;
	}

	assert(false);
	return false; // Should never reach this
}

// For whatever reason, we don't want to start with the first or with the
// last subimage. If the best correlated row points to one of those
// subimages we take the neighboring one instead.
void CSubImageCorrelation::ExcludeBorderSubImages()
{
	if (m_nBestCorrelatedSubImage < s_nNonBestSubImages)
		m_nBestCorrelatedSubImage = s_nNonBestSubImages;

	if (m_nBestCorrelatedSubImage > GetSubImageCount() - (s_nNonBestSubImages + 1))
		m_nBestCorrelatedSubImage = GetSubImageCount() - (s_nNonBestSubImages + 1);
}

/*
 * Performs the subimage correlation of the given subimage-index.
 * Herefore it searches the defined window, extracts the subimages and then correlates them.
 *
 * Returns a CCorrelationOffset with no set validity unless the defined rectangle is empty. In this case the Offset has an invalid-flag.
 */
CCorrelationOffset CSubImageCorrelation::PerformSubImageCorrelation(size_t nSubImageIndex)
{
	assert(m_Images.GetReferenceImage()->GetSize() == m_Images.GetTemplateImage()->GetSize());

	const auto rDefinedWindow = GetDefinedWindow(nSubImageIndex, m_CorrelationResult.RowOffset, m_Images.GetTemplateImage()->GetSize());

	if (!IsDefinedWindowSufficient(rDefinedWindow))
	{
		CCorrelationOffset result;
		result.SetValidity(CHrtValidityCodes::eInvalidSubImageSize);
		return result;
	}

	m_CurrentSubImagePair = ExtractPartialImages(rDefinedWindow);
	//m_CurrentSubImagePair.SetUseMinimalFFTSize(true);

	return PartialCorrelation(m_CurrentSubImagePair);
}

StlImageRect CSubImageCorrelation::GetDefinedWindow(size_t nSubImageIndex, const std::vector<CVector2d<float>>& RowOffsets, StlImageSize imageSize) const
{
	StlImageRect DefinedWindow = GetSubImageRect(nSubImageIndex);

	// Iterate through the output image rows
	for (auto row = DefinedWindow.y; row < (DefinedWindow.y + DefinedWindow.sy); row++)
	{
		int nCorrespondingReferenceRow = static_cast<int>(floor(row - RowOffsets[row].y)); // = Corresponding row in ReferenceImage

		if (nCorrespondingReferenceRow < 0) // The current row lies above the Reference Image
		{
			auto old_bottom = DefinedWindow.sy + DefinedWindow.y;
			DefinedWindow.y = row + 1;
			DefinedWindow.sy = old_bottom - DefinedWindow.y;
			continue;
		}
		if (nCorrespondingReferenceRow >= imageSize.y - 1) // The current row lies below the Reference Image
		{
			DefinedWindow.sy = row - DefinedWindow.y;
			break;
		}

		for (auto col = DefinedWindow.x; col < (DefinedWindow.x + DefinedWindow.sx); col++)
		{
			int nCorrespondingReferenceColumn = static_cast<int>(floor(col - RowOffsets[row].x)); // Corresponding column in RefImg

			if (nCorrespondingReferenceColumn < 0) // The current column lies left of the the Reference Image
			{
				auto old_right = DefinedWindow.sx + DefinedWindow.x;
				DefinedWindow.x = col + 1;
				DefinedWindow.sx = old_right - DefinedWindow.x;
				continue;
			}

			if (nCorrespondingReferenceColumn >= imageSize.x - 1) // The current column lies right of the the Reference Image
			{
				DefinedWindow.sx = col - DefinedWindow.x;
				break;
			}
		}
	}
	return DefinedWindow;
}

bool CSubImageCorrelation::IsDefinedWindowSufficient(const StlImageRect& DefinedWindow) const
{
	return (!DefinedWindow.IsRectEmpty() && static_cast<size_t>(DefinedWindow.sy) > GetSubImageHeight() / 2);
}

StlImage<float> CSubImageCorrelation::ApplyReferenceImageMask(const StlImage<float>& ReferenceImage, StlImageRect rDefinedWindow, bool bMaskImage) const
{
	StlImage<float> out;
	if (!bMaskImage)				// Return the whole ReferenceImage
	{
		out = ReferenceImage;
	}
	else
	{
		out.Alloc(StlImageSize(ReferenceImage.GetSize().x, GetSubImageHeight() + 2 * m_nPaddingSize));
		out.Clear(0.0);

		auto rChildWindow = CalculateCorrectChildWindow(rDefinedWindow, out.GetSize(), ReferenceImage.GetSize());

		out.Copy2d(rChildWindow, ReferenceImage, rDefinedWindow);

		//CMeasureImage child;
		//child.Child2d(out, rChildWindow);
		//child.CopyROI(ReferenceImage, rDefinedWindow);
		//child.Free();
	}

	return out;
}

StlImageRect CSubImageCorrelation::CalculateCorrectChildWindow(StlImageRect& rDefinedWindow, const StlImageSize stripeSize, const StlImageSize imageSize) const
{
	StlImagePoint offset{ (stripeSize.x - rDefinedWindow.sx) / 2, (stripeSize.y - rDefinedWindow.sy) / 2 };

	if (rDefinedWindow.x >= m_nPaddingSize)
	{
		long long temp = std::min(offset.x, static_cast<long long>(m_nPaddingSize));
		offset.x -= temp;
		rDefinedWindow.x -= temp;
	}
	if ((rDefinedWindow.x + rDefinedWindow.sx) + offset.x + m_nPaddingSize < imageSize.x)
	{
		rDefinedWindow.sx += m_nPaddingSize;
	}
	if (rDefinedWindow.y >= m_nPaddingSize)
	{
		long long temp = std::min(offset.y, static_cast<long long>(m_nPaddingSize));
		offset.y -= temp;
		rDefinedWindow.y -= temp;
	}
	if ((rDefinedWindow.y + rDefinedWindow.sy) + offset.y + m_nPaddingSize < imageSize.y)
	{
		rDefinedWindow.sy += m_nPaddingSize;
	}

	return StlImageRect(offset, rDefinedWindow.Size());
}

StlImage<float> CSubImageCorrelation::ApplyTemplateImageMask(const StlImage<float>& TemplateImage, StlImageSize imagesize, StlImageRect rDefinedWindow, bool bMaskImage) const
{
	assert(TemplateImage.GetSize() == rDefinedWindow.Size());

	StlImage<float> out;
	if (!bMaskImage)				// Return the Image in a black context
	{
		out.Alloc(imagesize);
	}
	else							// Embed the template into a 384x48px border
	{
		out.Alloc(StlImageSize(imagesize.x, GetSubImageHeight() + 2 * m_nPaddingSize));
		rDefinedWindow.MoveToXY((out.GetSize().x - rDefinedWindow.Size().x) / 2, (out.GetSize().y - rDefinedWindow.Size().y) / 2);
	}
	out.Clear(0.0);

	//CMeasureImage child;
	//child.Child2d(out, rDefinedWindow);
	//child.Copy(TemplateImage);
	//child.Free();
	out.Copy2d(rDefinedWindow, TemplateImage);

	return out;
}

size_t CSubImageCorrelation::GetSubImageHeight() const
{
	return m_ProcedureParameter.nSubImageHeight;
}

/**	\brief Interpoliert Ausschnitt aus Templatebild, sodass es perfekt auf
 *	die Pixel des Referenzbildes passt.
 */
StlImage<float> CSubImageCorrelation::InterpolateImage(const StlImage<float>& TemplateImage,
	const vector<CVector2d<float>>& offsetArray, StlImageRect DefinedWindow) const
{
	assert(offsetArray.size() == static_cast<size_t>(TemplateImage.GetSize().y));
	assert(DefinedWindow.x >= 0);
	assert(DefinedWindow.x + DefinedWindow.sx <= TemplateImage.GetSize().x);
	assert(DefinedWindow.y >= 0);
	assert(DefinedWindow.y + DefinedWindow.sy <= TemplateImage.GetSize().y);
	assert(DefinedWindow.sx <= TemplateImage.GetSize().x);
	assert(DefinedWindow.sy <= TemplateImage.GetSize().y);

	auto imageSize = TemplateImage.GetSize();

	auto StripeSize = DefinedWindow.Size();

	std::vector<float> pInputImageArray = TemplateImage.AsArray();
	StlImage<float> imOutputImage;
	imOutputImage.Alloc({ StripeSize.x, StripeSize.y });
	imOutputImage.Clear(0.0f);
	// For each pixel in Defined Window:
	//		Find correspongingPixel in ReferenceImage
	//		Interpolate the 4 surroudning pixels (Pixel -> BL) bilinearly 
	//		Copy Results into the area of the template image, where the corresponding referenceImage area is

	for (long long row = DefinedWindow.y; row < DefinedWindow.y + DefinedWindow.sy; row++)
	{
		for (long long col = DefinedWindow.x; col < DefinedWindow.x + DefinedWindow.sx; col++)
		{
			CVector2d<float> fCorrespondingReferenceImagePixel = GetCorrespondingPixelInReferenceImage(CVector2d<long long>(col, row), offsetArray[row]);
			CVector2d<long long> nCorrespondingReferenceImagePixel(fCorrespondingReferenceImagePixel);

			auto InterpolatedPixel = InterpolatePixelValue(pInputImageArray, fCorrespondingReferenceImagePixel, nCorrespondingReferenceImagePixel, imageSize.x);

			imOutputImage[{col - DefinedWindow.x, row - DefinedWindow.y}] = InterpolatedPixel;
		}
	}
	return imOutputImage;
}

CVector2d<float> CSubImageCorrelation::GetCorrespondingPixelInReferenceImage(CVector2d<long long> TemplatePixel, const CVector2d<float> RowOffset) const
{
	CVector2d<float> fCorrespondingReferenceImagePixel;
	fCorrespondingReferenceImagePixel.y = TemplatePixel.y - RowOffset.y;	// = Corresponding row in ReferenceImage
	fCorrespondingReferenceImagePixel.x = TemplatePixel.x - RowOffset.x;	// = Corresponding column in RefImg
	return fCorrespondingReferenceImagePixel;
}

float CSubImageCorrelation::InterpolatePixelValue(const vector<float>& InputBuffer, const CVector2d<float>& fCorrespondingReferencePixel, const CVector2d<long long>& nCorrespondingReferencePixel, long long ImageWidth) const
{
	CVector2d<float> fDist = fCorrespondingReferencePixel - CVector2d<float>(nCorrespondingReferencePixel);

	assert(fDist.x >= 0 && fDist.y >= 0);
	// Interpolate the pixel value aus 4 nachbarpixeln (bilinear)
	auto fPixelTL = InputBuffer[nCorrespondingReferencePixel.y * ImageWidth + nCorrespondingReferencePixel.x];
	auto fPixelTR = InputBuffer[nCorrespondingReferencePixel.y * ImageWidth + (nCorrespondingReferencePixel.x + 1)];
	auto fPixelBL = InputBuffer[(nCorrespondingReferencePixel.y + 1) * ImageWidth + nCorrespondingReferencePixel.x];
	auto fPixelBR = InputBuffer[(nCorrespondingReferencePixel.y + 1) * ImageWidth + (nCorrespondingReferencePixel.x + 1)];

	auto fPixelT = ((1 - fDist.x) * fPixelTL) + (fDist.x * fPixelTR);
	auto fPixelB = ((1 - fDist.x) * fPixelBL) + (fDist.x * fPixelBR);

	return (1 - fDist.y) * fPixelT + fDist.y * fPixelB;
}

/**	\brief Berechnet aus dem Referenz- und dem Templatebild die horizontalen
 *	und vertikalen Verschiebungen.
 *
 *	\param[in] imReference Das Referenzbild.
 *	\param[in] imTemplate Das Templatebild.
 *	\param[in] window Der zu suchende Bereich des Templatebilds.
 *	\param[out] vecOffset Enth�lt nach dem Aufruf der Methode den berechneten
 *		Versatzvektor.
 *	\param[out] fScore Enth�lt nach dem Aufruf der Methode den Score der
 *		Versatzberechnung.
 */
CCorrelationOffset CSubImageCorrelation::PartialCorrelation(const CImagePair& imageParams) const
{
	assert(imageParams.GetReferenceImage()->GetSize() == imageParams.GetTemplateImage()->GetSize());

	CImageCorrelation imageCorrelation(m_ProcedureParameter, imageParams, m_ScoreContainer);

	if (!imageCorrelation.CalculateBestOffsets(1))
	{
		return CCorrelationOffset();
	}

	assert(imageCorrelation.GetDetectedOffsetCount() == 1); // Should always be the case, since CalculateBestOffsets ist called with 1 
	// and the case 0 is impossible, since CalculateBestOffsets would have returned false.
	auto correlationOffset = imageCorrelation.GetBestOffset();

	return correlationOffset;
}

/**	\brief Addiert einen Betrag auf den Verschiebungswert des Teilbilds \a k
 *	und extrapoliert die Verschiebungswerte der benachbarten Teilbilder.
 *
 *	\param[in] k Index des aktuellen Teilbilds.
 *	\param[in] windowDirection Richtung der Extrapolation (<tt>up</tt> oder
 *		<tt>down</tt>).
 *	\param[in] nWindowRowAmount Die Anzahl der Bildzeilen des Teilbilds (in
 *		Pixel).
 *	\param[in,out] RowOffset Die Verschiebungswerte f�r alle Bildzeilen.
 *	\param[in,out] solutionArray Das Array f�r die Registriererbgebnisse
 *		aller Teilbilder.
 */
void CSubImageCorrelation::ExtrapolateOffset(size_t nCalculatedSubImageIndex, CCorrelationParameters::EDirection windowDirection)
{
	// don't extrapolate on invalid results
	if (m_CorrelationResult.FlexibleRegistrationResults[nCalculatedSubImageIndex].GetValidity() < 0)
		return;

	size_t nLastCalculatedIndex;

	if (windowDirection == CCorrelationParameters::EDirection::eDown)
		nLastCalculatedIndex = nCalculatedSubImageIndex - 1;
	else if (windowDirection == CCorrelationParameters::EDirection::eUp)
		nLastCalculatedIndex = nCalculatedSubImageIndex + 1;
	else
		// perform constant extrapolation after first subimage to continue correctly after faulty full-image registration
		nLastCalculatedIndex = nCalculatedSubImageIndex;

	// don't extrapolate if we have no previous subimage
	if ((nLastCalculatedIndex == -1) || (nLastCalculatedIndex == m_CorrelationResult.FlexibleRegistrationResults.size()))
		return;

	if (windowDirection == CCorrelationParameters::EDirection::eFirst)
		ExtrapolateOffsetConstant(nCalculatedSubImageIndex, windowDirection);
	else if (m_CorrelationResult.FlexibleRegistrationResults[nLastCalculatedIndex].GetValidity() < 0)
		ExtrapolateOffsetConstant(nCalculatedSubImageIndex, windowDirection);
	else
		ExtrapolateOffsetLinear(nCalculatedSubImageIndex, windowDirection);
}

void CSubImageCorrelation::ExtrapolateOffsetConstant(size_t nCalculatedSubImageIndex, CCorrelationParameters::EDirection windowDirection)
{
	if (windowDirection == CCorrelationParameters::EDirection::eDown)
	{
		for (size_t nObservedSubImage = nCalculatedSubImageIndex + 1; nObservedSubImage < m_CorrelationResult.FlexibleRegistrationResults.size(); nObservedSubImage++)
		{
			m_CorrelationResult.FlexibleRegistrationResults[nObservedSubImage].SetOffset(m_CorrelationResult.FlexibleRegistrationResults[nCalculatedSubImageIndex].GetOffset());
		}
	}
	else if (windowDirection == CCorrelationParameters::EDirection::eUp)
	{
		for (ptrdiff_t nObservedSubImage = nCalculatedSubImageIndex - 1; nObservedSubImage >= 0; nObservedSubImage--)
		{
			m_CorrelationResult.FlexibleRegistrationResults[nObservedSubImage].SetOffset(m_CorrelationResult.FlexibleRegistrationResults[nCalculatedSubImageIndex].GetOffset());
		}
	}
	else
	{
		for (size_t nObservedSubImage = 0; nObservedSubImage < m_CorrelationResult.FlexibleRegistrationResults.size(); nObservedSubImage++)
		{
			m_CorrelationResult.FlexibleRegistrationResults[nObservedSubImage].SetOffset(m_CorrelationResult.FlexibleRegistrationResults[nCalculatedSubImageIndex].GetOffset());
		}
	}
}

void CSubImageCorrelation::ExtrapolateOffsetLinear(size_t nCalculatedSubImageIndex, CCorrelationParameters::EDirection windowDirection)
{
	assert((windowDirection == CCorrelationParameters::EDirection::eDown) || (windowDirection == CCorrelationParameters::EDirection::eUp));

	CRigidRegistrationResult& localResult = m_CorrelationResult.FlexibleRegistrationResults[nCalculatedSubImageIndex];

	if ((windowDirection == CCorrelationParameters::EDirection::eDown) && (nCalculatedSubImageIndex > 0))
	{
		// Estimate sub-image offsets by linear extrapolation
		auto fDelta = localResult.GetOffset() - m_CorrelationResult.FlexibleRegistrationResults[nCalculatedSubImageIndex - 1].GetOffset();

		for (size_t nObservedSubImage = nCalculatedSubImageIndex + 1; nObservedSubImage < m_CorrelationResult.FlexibleRegistrationResults.size(); nObservedSubImage++)
		{
			m_CorrelationResult.FlexibleRegistrationResults[nObservedSubImage].SetOffset(localResult.GetOffset() + fDelta * static_cast<float>(nObservedSubImage - nCalculatedSubImageIndex));
		}
	}
	else if ((windowDirection == CCorrelationParameters::EDirection::eUp) && (nCalculatedSubImageIndex < m_CorrelationResult.FlexibleRegistrationResults.size() - 1))
	{
		// Estimate sub-image offsets by linear extrapolation
		auto fDelta = m_CorrelationResult.FlexibleRegistrationResults[nCalculatedSubImageIndex + 1].GetOffset() - localResult.GetOffset();

		for (ptrdiff_t nObservedSubImage = static_cast<ptrdiff_t>(nCalculatedSubImageIndex) - 1; nObservedSubImage >= 0; nObservedSubImage--)
		{
			m_CorrelationResult.FlexibleRegistrationResults[nObservedSubImage].SetOffset(
				localResult.GetOffset() - fDelta * static_cast<float>(static_cast<ptrdiff_t>(nCalculatedSubImageIndex) - nObservedSubImage));
		}
	}
}

void CSubImageCorrelation::InterpolateSubImageRowPositions(size_t nSubImageIndex, CCorrelationParameters::EDirection windowDirection)
{
	CRigidRegistrationResult& localResult = m_CorrelationResult.FlexibleRegistrationResults[nSubImageIndex];
	if (windowDirection == CCorrelationParameters::EDirection::eDown)
	{
		auto fDelta = (localResult.GetOffset() - m_CorrelationResult.FlexibleRegistrationResults[nSubImageIndex - 1].GetOffset()) / static_cast<float>(GetSubImageHeight());
		auto fRefRow = m_CorrelationResult.FlexibleRegistrationResults[nSubImageIndex - 1].GetOffset() + (fDelta * (0.5f + static_cast<float>(GetSubImageHeight()) / 2.0f));

		size_t nRefRow = nSubImageIndex * GetSubImageHeight();

		for (size_t nRow = nRefRow; nRow < m_CorrelationResult.RowOffset.size(); nRow++)
		{
			m_CorrelationResult.RowOffset[nRow] = fRefRow + fDelta * static_cast<float>(nRow - nRefRow);
		}
	}
	else if (windowDirection == CCorrelationParameters::EDirection::eUp)
	{
		auto fDelta = (m_CorrelationResult.FlexibleRegistrationResults[nSubImageIndex + 1].GetOffset() - localResult.GetOffset()) / static_cast<float>(GetSubImageHeight());
		auto fRefRow = m_CorrelationResult.FlexibleRegistrationResults[nSubImageIndex + 1].GetOffset() - (fDelta * (0.5f + static_cast<float>(GetSubImageHeight()) / 2.0f));

		ptrdiff_t nRefRow = (nSubImageIndex + 1) * GetSubImageHeight() - 1;

		for (ptrdiff_t nRow = nRefRow; nRow >= 0; nRow--)
		{
			m_CorrelationResult.RowOffset[nRow] = fRefRow - fDelta * static_cast<float>(nRefRow - nRow);
		}
	}
	else
	{
		for (size_t nRow = 0; nRow < GetSubImageHeight(); nRow++)
		{
			m_CorrelationResult.RowOffset[nSubImageIndex * GetSubImageHeight() + nRow] = m_CorrelationResult.FlexibleRegistrationResults[nSubImageIndex].GetOffset();
		}
	}
}

StlImageRect CSubImageCorrelation::GetSubImageRect(size_t nSubImageIndex) const
{
	auto nImageWidth = m_Images.GetReferenceImage()->GetSize().x;
	return StlImageRect(0, static_cast<int>(nSubImageIndex * GetSubImageHeight()), static_cast<int>(nImageWidth), static_cast<int>((nSubImageIndex + 1) * GetSubImageHeight()));
}

/**	\brief Überprüft, ob sich der Versatz um Vergleich zur letzten Iteration
 *	stark verändert hat.
 */
bool CSubImageCorrelation::ResultHasConverged(const CCorrelationOffset& correlationOffset) const
{
	return abs(correlationOffset.yOffset) < s_fHardConvergenceCriteria && abs(correlationOffset.xOffset) < s_fHardConvergenceCriteria;
}

/**	\brief Überprüft, ob sich der Versatz um Vergleich zur letzten Iteration
 *	stark verändert hat.
 */
bool CSubImageCorrelation::ResultHasAlmostConverged(const CCorrelationOffset& correlationOffset) const
{
	return abs(correlationOffset.yOffset) < s_fWeakConvergenceCriteria && abs(correlationOffset.xOffset) < s_fWeakConvergenceCriteria;
}

/**	\brief Überprüft, ob sich der Versatz im Vergleich zur letzten Iteration
 *	stark verändert hat.
 */
bool CSubImageCorrelation::ResultHasDiverged(const CCorrelationOffset& correlationOffset) const
{
	double thresholdX = GetSubImageHeight() * s_fDivergenceCriteriaX;
	double thresholdY = GetSubImageHeight() * s_fDivergenceCriteriaY;
	return !m_bIsFirstSubImage && ((abs(correlationOffset.yOffset) > thresholdY) || (abs(correlationOffset.xOffset) > thresholdX));
}

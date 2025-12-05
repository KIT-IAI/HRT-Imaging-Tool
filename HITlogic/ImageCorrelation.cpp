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
#include "ImageCorrelation.h"

#include "ScoreNormalizer.h"



CImageCorrelation::CImageCorrelation(CCorrelationParameters procParams, const CImagePair imgParams, CScoreContainer& scoreContainer)
	: m_CorrelationParameters(procParams),
	m_ScoreContainer(scoreContainer),
	m_Images(imgParams)
{
}


CImageCorrelation::~CImageCorrelation()
{
}

bool CImageCorrelation::IsPeriodicOverlapProblemPossible(const CCorrelationOffset& offset) const
{
	auto refSize = m_Images.GetReferenceImage()->GetSize();
	auto tempSize = m_Images.GetTemplateImage()->GetSize();

	StlImageSize maxSize{ std::max(refSize.x, tempSize.x), std::max(refSize.y, tempSize.y) };

	auto FFTSize = m_Images.GetFFTSize();

	return ((abs(offset.xOffset) + maxSize.x) > FFTSize.x) || ((abs(offset.yOffset) + maxSize.y) > FFTSize.y);
}

CCorrelationOffset CImageCorrelation::CalculatePeriodicOffsetX(const CCorrelationOffset& offset) const
{
	auto FFTSize = m_Images.GetFFTSize();
	auto fCorrectedX = (abs(offset.xOffset) - FFTSize.x) * static_cast<float>(CMathTools::Signum(offset.xOffset));

	return CalculateOffsetOfOverlappingArea(DPoint(fCorrectedX, offset.yOffset));
}

CCorrelationOffset CImageCorrelation::CalculatePeriodicOffsetY(const CCorrelationOffset& offset) const
{
	auto FFTSize = m_Images.GetFFTSize();
	auto fCorrectedY = (abs(offset.yOffset) - FFTSize.y) * static_cast<float>(CMathTools::Signum(offset.yOffset));

	return CalculateOffsetOfOverlappingArea(DPoint(offset.xOffset, fCorrectedY));
}

CCorrelationOffset CImageCorrelation::CalculatePeriodicOffsetXY(const CCorrelationOffset& offset) const
{
	auto FFTSize = m_Images.GetFFTSize();
	auto fCorrectedX = (abs(offset.xOffset) - FFTSize.x) * static_cast<float>(CMathTools::Signum(offset.xOffset));
	auto fCorrectedY = (abs(offset.yOffset) - FFTSize.y) * static_cast<float>(CMathTools::Signum(offset.yOffset));

	return CalculateOffsetOfOverlappingArea(DPoint(fCorrectedX, fCorrectedY));
}

CCorrelationOffset CImageCorrelation::GetBestOffset(bool bCheckValidity)
{
	if (GetDetectedOffsetCount() == 1)
		return m_correlationOffsets[0];

	int ZeroOffsetIndex, NonZeroOffsetIndex;
	FindBestScoringOffsets(ZeroOffsetIndex, NonZeroOffsetIndex, bCheckValidity);

	if (NonZeroOffsetIndex >= 0)
		return m_correlationOffsets[NonZeroOffsetIndex];

	if (ZeroOffsetIndex >= 0)
		return m_correlationOffsets[ZeroOffsetIndex];

	if (m_correlationOffsets.size() > 0)
		return m_correlationOffsets[0];

	return m_ScoreContainer.GetOffsetClassifier().CreateInvalidOffset();
}

void CImageCorrelation::ValidateOffsets()
{
	for (auto& offset : m_correlationOffsets)
	{
		if (IsPeriodicOverlapProblemPossible(offset))
		{
			CorrectPeriodicOverlapProblem(offset);
		}

		if (offset.m_EClassification >= EClassification::eCorrect)
		{
			auto OverlappingRectangles = CImagePair::GetOverlappingRectangles(m_Images.GetReferenceImage()->GetSize(), m_Images.GetTemplateImage()->GetSize(), StlImagePoint(CMathTools::RoundValue(offset.xOffset), CMathTools::RoundValue(offset.yOffset)));
			assert(OverlappingRectangles.first.IsRectEmpty() == OverlappingRectangles.second.IsRectEmpty());
			if (OverlappingRectangles.first.IsRectEmpty())
			{
				offset.SetValidity(CHrtValidityCodes::eInvalidEmptyOverlappingArea);
			}
			else
			{
				assert(OverlappingRectangles.first.Size() == OverlappingRectangles.second.Size());
				offset.SetValidity(CHrtValidityCodes::eValidRoughCorrelationMinScore);
			}
			continue;
		}

		if (offset.m_EClassification <= EClassification::eInvalid)
		{
			offset.SetValidity(CHrtValidityCodes::eInvalidRoughCorrelationMinScore);
			continue;
		}

		// At this point one could add a functionallity which does not check further offsets if one is classified as correct.

		HandleUnsureRegistration(offset);

	}
}

void CImageCorrelation::HandleUnsureRegistration(CCorrelationOffset& offset) const
{
	CCorrelationOffset overlappingOffset = CalculateOffsetOfOverlappingArea(offset.ToPoint());

	if (overlappingOffset.m_EClassification <= EClassification::eInvalid)
	{
		//offset.m_scoreBeforeNorm = overlappingOffset.m_scoreNorm1;
		offset.SetValidity(CHrtValidityCodes::eInvalidRoughCorrelationFlexibleChecked);
	}
	else
		offset.SetValidity(CHrtValidityCodes::eValidRoughCorrelationFlexibleChecked);
}

void CImageCorrelation::CorrectPeriodicOverlapProblem(CCorrelationOffset& OffsetToBeCorrected)
{
	auto FFTSize = m_Images.GetFFTSize();

	CCorrelationOffset nonPeriodicOffset = CalculateOffsetOfOverlappingArea(OffsetToBeCorrected.ToPoint());
	CCorrelationOffset periodicOffsetX;
	CCorrelationOffset periodicOffsetY;
	CCorrelationOffset periodicOffsetXY;

	bool bIsPeriodicOverlapProblemPossibleX = abs(OffsetToBeCorrected.xOffset) > FFTSize.x / 4;
	bool bIsPeriodicOverlapProblemPossibleY = abs(OffsetToBeCorrected.yOffset) > FFTSize.y / 4;

	if (bIsPeriodicOverlapProblemPossibleX)
	{
		periodicOffsetX = CalculatePeriodicOffsetX(OffsetToBeCorrected);
	}
	if (bIsPeriodicOverlapProblemPossibleY)
	{
		periodicOffsetY = CalculatePeriodicOffsetY(OffsetToBeCorrected);
	}
	if (bIsPeriodicOverlapProblemPossibleX && bIsPeriodicOverlapProblemPossibleY)
	{
		periodicOffsetXY = CalculatePeriodicOffsetXY(OffsetToBeCorrected);
	}
#define verybad_max(a,b) (((a)>(b)) ? (a) : (b))
	auto& maxScore = verybad_max(nonPeriodicOffset, verybad_max(periodicOffsetX, verybad_max(periodicOffsetY, periodicOffsetXY)));

	if (nonPeriodicOffset != maxScore)
	{
		m_ScoreContainer.GetOffsetClassifier().Classify(maxScore);
		if (maxScore.m_EClassification > EClassification::eInvalid)
		{
			OffsetToBeCorrected.xOffset = maxScore.xOffset;
			OffsetToBeCorrected.yOffset = maxScore.yOffset;
			//OffsetToBeCorrected.SetValidity(CHrtValidityCodes::eValidRoughCorrelationFlexibleCheckChangedDirection);
		}
	}
	//OffsetToBeCorrected.SetValidity(CHrtValidityCodes::eValidRoughCorrelationFlexibleChecked);
}

void CImageCorrelation::FindBestScoringOffsets(int& outBestScoringZeroOffsetIndex, int& outBestScoringNonZeroOffsetIndex, bool bCheckValidity)
{
	outBestScoringZeroOffsetIndex = -1;
	outBestScoringNonZeroOffsetIndex = -1;

	for (int index = 0; index < static_cast<int>(m_correlationOffsets.size()); index++)
	{
		CCorrelationOffset& offset = m_correlationOffsets[index];
		if (bCheckValidity && !offset.IsValid())
			continue;


		if (offset.IsZeroOffset())
		{
			if (outBestScoringZeroOffsetIndex == -1 || offset > m_correlationOffsets[outBestScoringZeroOffsetIndex])
			{
				outBestScoringZeroOffsetIndex = index;
			}
		}
		else
		{
			if (outBestScoringNonZeroOffsetIndex == -1 || offset > m_correlationOffsets[outBestScoringNonZeroOffsetIndex])
			{
				outBestScoringNonZeroOffsetIndex = index;
			}
		}
	}
}


/*
* Calculates the correlation score of only the two overlapping image areas with the given Image-Offset.
*/
CCorrelationOffset CImageCorrelation::CalculateOffsetOfOverlappingArea(DPoint offsetPosition) const
{
	// Area is calculated twice. This CAN be optimized. But does it need to be?
	if (GetOverlappingAreaNonZeroPixels(offsetPosition) == 0)
		return m_ScoreContainer.GetOffsetClassifier().CreateInvalidOffset();

	auto OverlappingRectangles = CImagePair::GetOverlappingRectangles(m_Images.GetReferenceImage()->GetSize(), m_Images.GetTemplateImage()->GetSize(), StlImagePoint(CMathTools::RoundValue(offsetPosition.m_x), CMathTools::RoundValue(offsetPosition.m_y)));

	assert(!OverlappingRectangles.first.IsRectEmpty() && !OverlappingRectangles.second.IsRectEmpty());
	assert(OverlappingRectangles.first.Size() == OverlappingRectangles.second.Size());

	StlImage<float> childRef, childTemp;
	childRef.Child2dIndep(*m_Images.GetReferenceImage(), OverlappingRectangles.first);
	childTemp.Child2dIndep(*m_Images.GetTemplateImage(), OverlappingRectangles.second);

	// Generate new parameterset
	CImagePair imageParams(&childRef, &childTemp);
	imageParams.SetUseFixedFFTSize(m_Images.GetFFTSize());

	//Calculate offsets
	CImageCorrelation imageCorrelation(m_CorrelationParameters, imageParams, m_ScoreContainer);
	imageCorrelation.CalculateBestOffsets(1);

	auto correctedOffset = imageCorrelation.GetBestOffset();

	correctedOffset.xOffset += static_cast<float>(offsetPosition.m_x);
	correctedOffset.yOffset += static_cast<float>(offsetPosition.m_y);

	m_ScoreContainer.GetOffsetClassifier().Classify(correctedOffset);

	return correctedOffset;
}

/*
	Returns true of one or more maxima were found
*/
bool CImageCorrelation::CalculateBestOffsets(int nMaxCount)
{
	return CalculateBestOffsets_cpu(nMaxCount);
}
/*
	Returns true of one or more maxima were found
*/
bool CImageCorrelation::CalculateBestOffsets_cpu(int nMaxCount)
{
	auto Phase_Image = GetPhaseCorrelationImage();

	if (!IsStandardDeviationSufficient(Phase_Image))
		return false;

	StlImage<float> ScoreNorm1;

	NormalizePhaseImage(Phase_Image, ScoreNorm1);

	for (int maxcount = 0; maxcount < nMaxCount; maxcount++)
	{
		if (maxcount != 0 && !IsStandardDeviationSufficient(Phase_Image))
			return maxcount > 0;

		CCorrelationOffset offset;
		offset = FindOffset(Phase_Image, ScoreNorm1);
		m_correlationOffsets.push_back(offset);

		ScoreNorm1.ClearPoint(offset.m_MaximumPosition, s_nPaddingSize);
	}
	return true;
}

void CImageCorrelation::NormalizePhaseImage(StlImage<float>& phase_image, StlImage<float>& ScoreNorm1)
{
	m_ScoreContainer.GetScoreNormalizer().CalculateScoreNorm1(phase_image, ScoreNorm1);
}

StlImage<float> CImageCorrelation::GetPhaseCorrelationImage() const
{
	StlImage<float> Phase_Image;
	Phase_Image.Correlation_phase(*m_Images.GetReferenceImage(), *m_Images.GetTemplateImage(), s_nLowFreqSize, s_nHighFreqSize, m_Images.GetFFTSize());
	return Phase_Image;
}

CCorrelationOffset CImageCorrelation::FindOffset(const StlImage<float>& Phase_Image, const StlImage<float>& ScoreNorm1) const
{
	CCorrelationOffset offset;

	auto [max, maximumPixelPosition] = ScoreNorm1.FindMax();
	DPoint maximumSubpixelPosition = ScoreNorm1.FindMaxSubPixelPosition(maximumPixelPosition);
	DPoint offsetPosition = ConvertPhaseCorrelationMaximumPositionToOffset(maximumSubpixelPosition, ScoreNorm1.GetSize());

	offset.m_MaximumPosition = maximumSubpixelPosition;
	offset.xOffset = static_cast<float>(offsetPosition.m_x);
	offset.yOffset = static_cast<float>(offsetPosition.m_y);

	offset.m_scoreBeforeNorm = Phase_Image.at({ static_cast<long long>(maximumPixelPosition.m_x), static_cast<long long>(maximumPixelPosition.m_y) });
	offset.m_scoreNorm1 = ScoreNorm1.at({ static_cast<long long>(maximumPixelPosition.m_x), static_cast<long long>(maximumPixelPosition.m_y) });

	//NormalizeCorrelationValue1(offset.m_scoreNorm1, offset.m_scoreNorm1);
	//NormalizeCorrelationValue2(offset.m_scoreNorm2, offset.m_scoreNorm2);

	m_ScoreContainer.GetOffsetClassifier().Classify(offset);
	return offset;
}
bool CImageCorrelation::IsStandardDeviationSufficient(StlImage<float>& Phase_Image) const
{
	auto stddev = Phase_Image.StandardDeviation();
	return !CCompare::CompareDoubles(stddev, 0.0, 6);
}

DPoint CImageCorrelation::ConvertPhaseCorrelationMaximumPositionToOffset(DPoint maximum, StlImageSize FFTSize) const
{
	double fModX = static_cast<float>(fmod((maximum.m_x + FFTSize.x / 2.0), FFTSize.x));
	maximum.m_x = fModX - FFTSize.x / 2.0F;

	double fModY = static_cast<float>(fmod((maximum.m_y + FFTSize.y / 2.0), FFTSize.y));
	maximum.m_y = fModY - FFTSize.y / 2.0F;

	return maximum;
}

size_t CImageCorrelation::GetOverlappingAreaNonZeroPixels(const DPoint& subpixelPosition) const
{
	auto [refChildRect, tempChildRect] = CImagePair::GetOverlappingRectangles(m_Images.GetReferenceImage()->GetSize(), m_Images.GetTemplateImage()->GetSize(), StlImagePoint(CMathTools::RoundValue(subpixelPosition.m_x), CMathTools::RoundValue(subpixelPosition.m_y)));

	assert(refChildRect.Size() == tempChildRect.Size());

	if (refChildRect.IsRectEmpty())
		return 0;

	//	StlImage<float> child1, child2;
	//	child1.Child2dIndep(*m_Images.GetReferenceImage(), refChildRect);
	//	child2.Child2dIndep(*m_Images.GetTemplateImage(), tempChildRect);

	//	StlImage<float> temp = child1 * child2;

	//	auto nNonZeroPixelCount = temp.NumberOfNonzeroPixels();

	size_t res = 0;
	auto& ref = *m_Images.GetReferenceImage();
	auto& tpl = *m_Images.GetTemplateImage();
	for (int y = 0;y < refChildRect.Size().y;y++)
	{
		for (int x = 0;x < refChildRect.Size().x;x++)
		{
			if (ref[{x + refChildRect.x, y + refChildRect.y}] != 0 && tpl[{x + tempChildRect.x, y + tempChildRect.y}] != 0) res++;
		}
	}

	//child1.Free();
	//child2.Free();

	//return nNonZeroPixelCount;
	return res;
}

int CImageCorrelation::GetDetectedOffsetCount()
{
	return static_cast<int>(m_correlationOffsets.size());
}

void CImageCorrelation::NormalizeCorrelationValue1(float fScore, float& fScoreNorm) const
{
	const auto& img1 = *m_Images.GetReferenceImage();
	const auto& img2 = *m_Images.GetTemplateImage();

	const auto& FFTSize = m_Images.GetFFTSize();

	assert(std::get<0>(img1.FindMin()) >= 0);
	assert(std::get<0>(img2.FindMin()) >= 0);

	// Anzahl der Pixel, die im Quellbild nicht 0 sind
	ptrdiff_t nNumberOfNonzeroPixelsImg1 = img1.NumberOfNonzeroPixels();
	ptrdiff_t nNumberOfNonzeroPixelsImg2 = img2.NumberOfNonzeroPixels();

	ptrdiff_t nNumberOfAllPixels = FFTSize.x * FFTSize.y * 2;
	ptrdiff_t nNumberOfDefPixels = nNumberOfNonzeroPixelsImg1 + nNumberOfNonzeroPixelsImg2;

	fScoreNorm = static_cast<float>(fScore * sqrt(static_cast<double>(nNumberOfDefPixels) / static_cast<double>(nNumberOfAllPixels)));
}


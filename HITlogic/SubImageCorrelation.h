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
#include "CorrelationParameters.h"
#include "ImagePair.h"
#include "RegistrationResult.h"
#include "CorrelationOffset.h"
#include "ImageCorrelation.h"
#include "StlImage.h"

class CSubImageCorrelation
{
public:
	CSubImageCorrelation(CCorrelationParameters& m_CorrelationProcedureParameter,
		CImagePair& imageParams,
		CScoreContainer& scoreContainer,
		CRegistrationResult& correlationResult);
	~CSubImageCorrelation();

private:
	static const int s_nMaxIterations;
	static const int s_nNonBestSubImages;
	static const double s_fHardConvergenceCriteria;
	static const double s_fWeakConvergenceCriteria;
	static const double s_fDivergenceCriteriaX;
	static const double s_fDivergenceCriteriaY;

	CImagePair m_Images;
	CImagePair m_CurrentSubImagePair;

	CCorrelationParameters m_ProcedureParameter;
	CScoreContainer& m_ScoreContainer;

	CRegistrationResult& m_CorrelationResult;

	size_t m_nBestCorrelatedRow = 0;
	size_t m_nBestCorrelatedSubImage = 0;

	const size_t m_nPaddingSize;

	bool m_bIsFirstSubImage = true;

public:
	size_t GetBestCorrelatedSubImage() const;
	size_t GetSubImageCount() const;

	void AdjustTrajectory(CCorrelationParameters::EDirection direction);
	void InitCorrelation();

	bool CorrelateSubImage(size_t nImageIndex, CCorrelationParameters::EDirection windowDirection);

private:
	static size_t CalculatePaddingSize(const CCorrelationParameters& params);

	void ExcludeBorderSubImages();
	void FindBestCorrelatedSubImage();
	void PrepareFlexibleResultArray();
	void PrepareOffsetArray();
	bool IterationProcessCanBeEnded(size_t nSubImageIndex, std::list<CCorrelationOffset> OffsetsByIteration);

	void ValidateSubImageOffset(size_t nSubImageOffset, CCorrelationOffset& correlationOffset) const;

	StlImage<float> ApplyReferenceImageMask(const StlImage<float>& ReferenceImage, StlImageRect rDefinedWindow, bool bMaskImage) const;
	StlImage<float> ApplyTemplateImageMask(const StlImage<float>& TemplateImage, StlImageSize imagesize, StlImageRect rDefinedWindow, bool bMaskImage) const;

	bool IsReferenceImageMaskingActive() const;
	void SetScoreValues(size_t nImageIndex, CCorrelationOffset correlationOffset) const;

	StlImageRect GetDefinedWindow(size_t nSubImageIndex, const std::vector<CVector2d<float>>& rowOffsets, StlImageSize imageSize) const;
	bool IsDefinedWindowSufficient(const StlImageRect& DefinedWindow) const;

	CImagePair ExtractPartialImages(const StlImageRect rDefinedWindow) const;
	void FindBestCorrelatedRow();
	size_t GetSubImageHeight() const;

	CCorrelationOffset PerformSubImageCorrelation(size_t nSubImageIndex);

	size_t SearchBestCorrelatedRowInOverlappingArea(StlImagePoint Offset) const;
	StlImage<float> SmoothImage(const StlImage<float>& image) const;
	std::vector<float> CalculateDeviation(const StlImage<float>& sumImage, const StlImage<float>& difference_image) const;
	void MovingAverage(std::vector<float>& afStdDev, int nNeighbours) const;

	CCorrelationOffset PartialCorrelation(const CImagePair& imageParams) const;
	void ExtrapolateOffset(size_t nResultIndex, CCorrelationParameters::EDirection windowDirection);
	void ExtrapolateOffsetConstant(size_t nResultIndex, CCorrelationParameters::EDirection windowDirection);
	void ExtrapolateOffsetLinear(size_t nResultIndex, CCorrelationParameters::EDirection windowDirection);
	void AdjustOffsetValues(size_t nSubImageIndex, const CCorrelationOffset& offset) const;
	void InterpolateSubImageRowPositions(size_t nSubImageIndex, CCorrelationParameters::EDirection windowDirection);

	StlImageRect CalculateCorrectChildWindow(StlImageRect& rDefinedWindow, const StlImageSize stripeSize, const StlImageSize imageSize) const;

	CVector2d<float> GetCorrespondingPixelInReferenceImage(CVector2d<long long> TemplatePixel, const CVector2d<float> rowOffset) const;
	float InterpolatePixelValue(const std::vector<float>& InputBuffer, const CVector2d<float>& fCorrespondingReferencePixel, const CVector2d<long long>& nCorrespondingReferencePixel, long long ImageWidth) const;
	StlImage<float> InterpolateImage(const StlImage<float>& imInputImage,
		const std::vector<CVector2d<float>>& offsetArray,
		StlImageRect DefinedWindow) const;

	StlImageRect GetSubImageRect(size_t nSubImageIndex) const;
	bool ResultHasConverged(const CCorrelationOffset& correlationOffset) const;
	bool ResultHasAlmostConverged(const CCorrelationOffset& correlationOffset) const;
	bool ResultHasDiverged(const CCorrelationOffset& correlationOffset) const;
};

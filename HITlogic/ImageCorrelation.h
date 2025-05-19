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
#include "CorrelationOffset.h"
#include "ScoreContainer.h"

class CImageCorrelation
{
private:
	static const int s_nLowFreqSize = 12;
	static const int s_nHighFreqSize = 2;
	static const int s_nPaddingSize = 3;

public:
	CImageCorrelation(CCorrelationParameters procParams, const CImagePair imgParams, CScoreContainer& scoreContainer);
	~CImageCorrelation();

	bool CalculateBestOffsets(int nMaxCount);
	int GetDetectedOffsetCount();

	void ValidateOffsets();
	CCorrelationOffset GetBestOffset(bool bCheckValidity = false);
	void FindBestScoringOffsets(int& outBestScoringZeroOffsetIndex, int& outBestScoringNonZeroOffsetIndex, bool bCheckValidity);

private:
	bool CalculateBestOffsets_cpu(int nMaxCount);

	CCorrelationParameters m_CorrelationParameters;
	CScoreContainer& m_ScoreContainer;

	const CImagePair m_Images;

	std::vector<CCorrelationOffset> m_correlationOffsets;


	void NormalizePhaseImage(StlImage<float>& phase_image, StlImage<float>& score_norm1);
	StlImage<float> GetPhaseCorrelationImage() const;


	bool IsStandardDeviationSufficient(StlImage<float>& Phase_Image) const;

	size_t GetOverlappingAreaNonZeroPixels(const DPoint& subpixelPosition) const;

	CCorrelationOffset FindOffset(const StlImage<float>& Phase_Image, const StlImage<float>& ScoreNorm1) const;

	DPoint ConvertPhaseCorrelationMaximumPositionToOffset(DPoint maximum, StlImageSize FFTSize) const;

	CCorrelationOffset CalculateOffsetOfOverlappingArea(DPoint offsetPosition) const;
	CCorrelationOffset CalculatePeriodicOffsetX(const CCorrelationOffset& offset) const;
	CCorrelationOffset CalculatePeriodicOffsetY(const CCorrelationOffset& offset) const;
	CCorrelationOffset CalculatePeriodicOffsetXY(const CCorrelationOffset& offset) const;
	void CorrectPeriodicOverlapProblem(CCorrelationOffset& offset);

	void HandleUnsureRegistration(CCorrelationOffset& offset) const;

	bool IsPeriodicOverlapProblemPossible(const CCorrelationOffset& offset) const;

	void NormalizeCorrelationValue1(float fScore, float& fScoreNorm) const;
};

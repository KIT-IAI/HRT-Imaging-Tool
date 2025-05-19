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
#include "SNPDatasetParameters.h"
#include <filesystem>

const wstring CSNPDatasetParameters::TableName = L"Parameters";

CRegistrationStrategyParameters CSNPDatasetParameters::GenerateRegistrationStrategyParameters(StlImageSize ImageSize) const
{
	CRegistrationStrategyParameters param;

	param.eProcessType = eProcessType;
	param.nMaxDistance = nMaxImageDistance;
	param.nMaxRegistrationDistance = nMaxRegDistance;
	param.nSectorSize = nSectorSize;
	param.nStepSize = nStepSize;
	param.nWindowSize = nWindowSize;
	param.ScoreParameters = GenerateScoreParameters();
	param.ImageSize = ImageSize;/*
	param.nPlaneCount = nPlaneCount;
	param.nPlaneIncrement = nPlaneIncrement;*/
	param.fSelectionHeight = fSelectionHeight;

	return param;
}

CHrtPreProcessingParameters CSNPDatasetParameters::GeneratePreProcessingParameters() const
{
	CHrtPreProcessingParameters param;

	param.MaskSize = StlImageSize(1, static_cast<int>(nSubImageHeight));
	param.eMaskImages = EMaskOptions::eMaskToBiggestButMultipleOfN;
	param.bCrobBlackBorder = true;
	param.bShiftCorrection = bShiftCorrection;
	param.bBrightnessCorrection = bBrightnessCorrectionBeforeRegistration;
	param.bVigAdjustment = bVignettingCorrectionBeforeRegistration;
	param.sVigAdjustmentProfilePath = sVignettingFileBeforeRegistration;
	param.nVigCreationMaxImages = nVignettingCreationMaxImages;
	param.nVigSmoothingIterations = nVignettingSmoothingBeforeRegistration;

	return param;
}

CGlobalPositioningParameters CSNPDatasetParameters::GenerateGlobalPositioningParameters() const
{
	CGlobalPositioningParameters param;

	param.eProcessType = eProcessType;
	param.eAlgorithm = eSolver;

	return param;
}

CRegistrationProcedureParameters CSNPDatasetParameters::GenerateRegistrationProcedureParameters(StlImageSize ImageSize) const
{
	CRegistrationProcedureParameters param;

	param.CorrelationParameters = GenerateCorrelationParameters();
	param.StrategyParameters = GenerateRegistrationStrategyParameters(ImageSize);
	param.VerificationParameters = GenerateRegistrationVerificationParameters(ImageSize);
	param.ScoreParameters = GenerateScoreParameters();

	param.SetProcessType(eProcessType);
	param.ePairVerificator = eImagePairVerificator;
	param.eRegistrationStrategy = eStrategie;
	param.SetSolverAlgorithm(eSolver);
	param.fScaleReduction = fScalation;
	param.fSubImageScaleReduction = fScalationSubImages;
	param.bAutomaticThresholdDetection = bAutomaticThresholdDetection;
	param.bConsistencyCheck = bConsistencyCheck;

	return param;
}

CCorrelationParameters CSNPDatasetParameters::GenerateCorrelationParameters() const
{
	CCorrelationParameters param;

	param.nSubImageHeight = nSubImageHeight;
	param.bConsistencyCheck = bConsistencyCheck;
	param.eProcessType = eProcessType;

	return param;
}

CRegistrationScoreParameters CSNPDatasetParameters::GenerateScoreParameters() const
{
	CRegistrationScoreParameters param;

	param.fMinScore = fMinScore;
	param.fMinScoreFlexible = fMinScoreFlexible;
	param.fCertainScore = fCertainScore;
	param.bAutomaticThresholdDetection = bAutomaticThresholdDetection;
	param.SetScoreThresholdPrt(fScoreThreshold);

	*fScoreThreshold = fMinScore;

	return param;
}

CHrtPreProcessingParameters CSNPDatasetParameters::GenerateCompositingPreProcessingParameters() const
{
	CHrtPreProcessingParameters param;

	param.MaskSize = StlImageSize(1, static_cast<int>(nSubImageHeight));
	param.eMaskImages = EMaskOptions::eMaskToBiggestButMultipleOfN;
	param.bCrobBlackBorder = true;
	param.bShiftCorrection = bShiftCorrection;
	param.bBrightnessCorrection = bBrightnessCorrectionBeforeCompositing;
	param.bVigAdjustment = bVignettingCorrectionBeforeCompositing;
	param.sVigAdjustmentProfilePath = sVignettingFileBeforeCompositing;
	param.nVigCreationMaxImages = nVignettingCreationMaxImages;
	param.nVigSmoothingIterations = nVignettingSmoothingBeforeCompositing;

	return param;
}

CHrtImageParameters CSNPDatasetParameters::GenerateImageParameters(StlImageSize ImageSize) const
{
	if (eProcessType == CProcessType::eRigidRegistration)
		return CHrtImageParameters(ImageSize, ImageSize.y);
	else
		return CHrtImageParameters(ImageSize, nSubImageHeight);
}

CCompositingParameters CSNPDatasetParameters::GenerateCompositingParameters() const
{
	CCompositingParameters param;

	param.eRegistrationProcessType = eProcessType;
	param.eWeightMode = eWeightMode;
	param.nBorder = nBorder;
	param.cBackgroundColor = cBackgroundColor;
	param.nSubImageHeight = nSubImageHeight;
	param.eCompositingMode = eCompositingMode;
	param.fIntensityLimit = fIntensityLimit;
	param.fZMultiplier = fZMultiplier;
	param.bExcludeNonSNPImages = bExcludeNonSNPImages;

	return param;
}

CRegistrationVerificationParameters CSNPDatasetParameters::GenerateRegistrationVerificationParameters(StlImageSize ImageSize) const
{
	CRegistrationVerificationParameters param;

	param.eSolverAlgorithm = eSolver;
	param.ImageSize = ImageSize;

	return param;
}

bool CSNPDatasetParameters::IsValidParameterset() const
{
	bool ok = true;
	{
		int tmp = static_cast<int>(eProcessType);
		ok &= tmp >= 0 && tmp < static_cast<int>(CProcessType::EProcessType::eInvalid);
	}
	{
		int tmp = static_cast<int>(eSolver);
		ok &= tmp >= 0 && tmp < static_cast<int>(CSLESolver::EAlgorithm::eAlgorithmCount);
	}
	ok &= nMaxImageDistance >= 0;
	ok &= nMaxRegDistance >= 1;
	ok &= nWindowSize >= 1;
	ok &= nStepSize >= 1;
	ok &= nSectorSize >= 1;
	ok &= fSelectionHeight > 0;
	ok &= VignettingProfileBeforeRegistrationIsValid();
	{
		int tmp = static_cast<int>(eStrategie);
		ok &= tmp >= 0 && tmp < static_cast<int>(ERegistrationStrategy::eInvalid);
	}
	ok &= nSubImageHeight == 64 || nSubImageHeight == 32 || nSubImageHeight == 16;
	ok &= fScalation >= 1;
	ok &= fScalationSubImages >= 1;
	ok &= fMinScore > 0;
	ok &= fMinScoreFlexible > 0;
	ok &= fCertainScore >= fMinScore;
	ok &= VignettingProfileBeforeCompositingIsValid();
	{
		int tmp = static_cast<int>(eWeightMode);
		ok &= tmp >= 0 && tmp < static_cast<int>(CCompositingParameters::EWeightMode::eInvalid);
	}
	ok &= nBorder < 1000;
	ok &= cBackgroundColor >= 0 && cBackgroundColor <= 255;

	ok &= nVignettingCreationMaxImages >= 0;
	ok &= nVignettingSmoothingBeforeRegistration >= 0;
	ok &= nVignettingSmoothingBeforeCompositing >= 0;

	return ok;
}

std::filesystem::path CSNPDatasetParameters::GetExePath() {
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(nullptr, path, MAX_PATH);  // Ermittelt den Pfad der aktuellen EXE
	return std::filesystem::path(path).parent_path();  // Gibt nur den Ordnerpfad zurück
}
 
bool CSNPDatasetParameters::VignettingProfileBeforeRegistrationIsValid() const
{
	if (!bVignettingCorrectionBeforeRegistration)
		return true;
	if (sVignettingFileBeforeRegistration == L"")	// Autoprofile
		return true;

	if (CFileUtilities::IsRelative(sVignettingFileBeforeRegistration))
	{
		auto sAbsolutePath = CFileUtilities::GetAbsolutePath(GetExePath().wstring(), sVignettingFileBeforeRegistration);
		return CFileUtilities::FileExists(sAbsolutePath);
	}
	return CFileUtilities::FileExists(sVignettingFileBeforeRegistration);
}

bool CSNPDatasetParameters::VignettingProfileBeforeCompositingIsValid() const
{
	if (!bVignettingCorrectionBeforeCompositing)
		return true;
	if (sVignettingFileBeforeCompositing == L"")	// Autoprofile
		return true;
	if (CFileUtilities::IsRelative(sVignettingFileBeforeCompositing))
	{
		auto sAbsolutePath = CFileUtilities::GetAbsolutePath(GetExePath().wstring(), sVignettingFileBeforeCompositing);
		return CFileUtilities::FileExists(sAbsolutePath);
	}
	return CFileUtilities::FileExists(sVignettingFileBeforeCompositing);
}

bool operator==(const CSNPDatasetParameters& lhs, const CSNPDatasetParameters& rhs)
{
	return lhs.eProcessType == rhs.eProcessType
		&& lhs.eSolver == rhs.eSolver
		&& lhs.nMaxImageDistance == rhs.nMaxImageDistance
		&& lhs.nMaxRegDistance == rhs.nMaxRegDistance
		&& lhs.nWindowSize == rhs.nWindowSize
		&& lhs.nStepSize == rhs.nStepSize
		&& lhs.nSectorSize == rhs.nSectorSize
		&& lhs.fSelectionHeight == rhs.fSelectionHeight
		&& lhs.bBrightnessCorrectionBeforeRegistration == rhs.bBrightnessCorrectionBeforeRegistration
		&& lhs.bVignettingCorrectionBeforeRegistration == rhs.bVignettingCorrectionBeforeRegistration
		&& lhs.bShiftCorrection == rhs.bShiftCorrection
		&& lhs.sVignettingFileBeforeRegistration == rhs.sVignettingFileBeforeRegistration
		&& lhs.eImagePairVerificator == rhs.eImagePairVerificator
		&& lhs.eStrategie == rhs.eStrategie
		&& lhs.nSubImageHeight == rhs.nSubImageHeight
		&& lhs.bConsistencyCheck == rhs.bConsistencyCheck
		&& lhs.fScalation == rhs.fScalation
		&& lhs.fScalationSubImages == rhs.fScalationSubImages
		&& lhs.fMinScore == rhs.fMinScore
		&& lhs.fCertainScore == rhs.fCertainScore
		&& lhs.fMinScoreFlexible == rhs.fMinScoreFlexible
		&& lhs.bAutomaticThresholdDetection == rhs.bAutomaticThresholdDetection
		&& lhs.bVignettingCorrectionBeforeCompositing == rhs.bVignettingCorrectionBeforeCompositing
		&& lhs.bBrightnessCorrectionBeforeCompositing == rhs.bBrightnessCorrectionBeforeCompositing
		&& lhs.sVignettingFileBeforeCompositing == rhs.sVignettingFileBeforeCompositing
		&& lhs.eWeightMode == rhs.eWeightMode
		&& lhs.nBorder == rhs.nBorder
		&& lhs.cBackgroundColor == rhs.cBackgroundColor
		&& lhs.eCompositingMode == rhs.eCompositingMode
		&& lhs.nVignettingCreationMaxImages == rhs.nVignettingCreationMaxImages
		&& lhs.nVignettingSmoothingBeforeRegistration == rhs.nVignettingSmoothingBeforeRegistration
		&& lhs.nVignettingSmoothingBeforeCompositing == rhs.nVignettingSmoothingBeforeCompositing
		&& lhs.nMaxSequenceLength == rhs.nMaxSequenceLength
		&& lhs.fIntensityLimit == rhs.fIntensityLimit
		&& lhs.fZMultiplier == rhs.fZMultiplier
		&& lhs.bExcludeNonSNPImages == rhs.bExcludeNonSNPImages
		&& lhs.bForceExportMotionCorrectedImages == rhs.bForceExportMotionCorrectedImages
		&& lhs.bDetailedLogging == rhs.bDetailedLogging;
}

bool operator!=(const CSNPDatasetParameters& lhs, const CSNPDatasetParameters& rhs)
{
	return !(lhs == rhs);
}

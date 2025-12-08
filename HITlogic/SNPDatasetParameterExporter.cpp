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
#include "SNPDatasetParameterExporter.h"



void CSNPDatasetParameterExporter::ExportChangesToRegistry(const std::wstring& regPath, const CSNPDatasetParameters& parameters, const CSNPDatasetParameters& reference)
{
#define PARAM(NAME, VALUE, REF) if (VALUE != REF) {GetGSS()->WriteValueCurrentUser(regPath, NAME, VALUE);}
#define PARAM_ENUM(NAME, VALUE, REF) if (VALUE != REF) {typename std::underlying_type<decltype(VALUE)>::type tmp = static_cast<typename std::underlying_type<decltype(VALUE)>::type>(VALUE); GetGSS()->WriteValueCurrentUser(regPath, NAME, tmp);}

	// Image Import
	PARAM(L"MaxSequenceLength", parameters.nMaxSequenceLength, reference.nMaxSequenceLength);

	// Registration Preprocessing
	PARAM(L"BrightnessCorrectionBeforeRegistration", parameters.bBrightnessCorrectionBeforeRegistration, reference.bBrightnessCorrectionBeforeRegistration);
	PARAM(L"VignettingCorrectionBeforeRegistration", parameters.bVignettingCorrectionBeforeRegistration, reference.bVignettingCorrectionBeforeRegistration);
	PARAM(L"ShiftCorrection", parameters.bShiftCorrection, reference.bShiftCorrection);
	PARAM(L"VignettingFileBeforeRegistration", parameters.sVignettingFileBeforeRegistration, reference.sVignettingFileBeforeRegistration);
	PARAM(L"VignettingSmoothingBeforeRegistration", parameters.nVignettingSmoothingBeforeRegistration, reference.nVignettingSmoothingBeforeRegistration);
	PARAM(L"VignettingCreationMaxImages", parameters.nVignettingCreationMaxImages, reference.nVignettingCreationMaxImages);

	// Registration Strategy
	PARAM_ENUM(L"ProcessType", parameters.eProcessType, reference.eProcessType);
	PARAM_ENUM(L"Solver", parameters.eSolver, reference.eSolver);
	PARAM(L"MaxImageDistance", parameters.nMaxImageDistance, reference.nMaxImageDistance);
	PARAM(L"MaxRegDistance", parameters.nMaxRegDistance, reference.nMaxRegDistance);
	PARAM(L"WindowSize", parameters.nWindowSize, reference.nWindowSize);
	PARAM(L"StepSize", parameters.nStepSize, reference.nStepSize);
	PARAM(L"SectorSize", parameters.nSectorSize, reference.nSectorSize);
	PARAM(L"SelectionHeight", parameters.fSelectionHeight, reference.fSelectionHeight);

	// Registration Procedure
	PARAM_ENUM(L"ImagePairVerificator", parameters.eImagePairVerificator, reference.eImagePairVerificator);
	PARAM_ENUM(L"Strategy", parameters.eStrategie, reference.eStrategie);

	// Correlation Procedure
	PARAM(L"SubImageHeight", parameters.nSubImageHeight, reference.nSubImageHeight);
	PARAM(L"ConsistencyCheck", parameters.bConsistencyCheck, reference.bConsistencyCheck);
	PARAM(L"Scalation", parameters.fScalation, reference.fScalation);
	PARAM(L"ScalationSubImages", parameters.fScalationSubImages, reference.fScalationSubImages);

	// Score Parameters
	PARAM(L"MinScore", parameters.fMinScore, reference.fMinScore);
	PARAM(L"CertainScore", parameters.fCertainScore, reference.fCertainScore);
	PARAM(L"MinScoreFlexible", parameters.fMinScoreFlexible, reference.fMinScoreFlexible);
	PARAM(L"AutomaticThresholdDetection", parameters.bAutomaticThresholdDetection, reference.bAutomaticThresholdDetection);
	PARAM(L"ResidualThreshold", parameters.fResidualThreshold, reference.fResidualThreshold);

	// Compositing Preprocessing
	PARAM(L"VignettingCorrectionBeforeCompositing", parameters.bVignettingCorrectionBeforeCompositing, reference.bVignettingCorrectionBeforeCompositing);
	PARAM(L"BrightnessCorrectionBeforeCompositing", parameters.bBrightnessCorrectionBeforeCompositing, reference.bBrightnessCorrectionBeforeCompositing);
	PARAM(L"VignettingFileBeforeCompositing", parameters.sVignettingFileBeforeCompositing, reference.sVignettingFileBeforeCompositing);
	PARAM(L"VignettingSmoothingBeforeCompositing", parameters.nVignettingSmoothingBeforeCompositing, reference.nVignettingSmoothingBeforeCompositing);

	// Compositing
	PARAM_ENUM(L"WeightMode", parameters.eWeightMode, reference.eWeightMode);
	PARAM(L"Border", parameters.nBorder, reference.nBorder);
	PARAM(L"BackgroundColor", parameters.cBackgroundColor, reference.cBackgroundColor);
	PARAM_ENUM(L"CompositingMode", parameters.eCompositingMode, reference.eCompositingMode);
	PARAM(L"f3dIntensityLimit", parameters.fIntensityLimit, reference.fIntensityLimit);
	PARAM(L"f3dZMultiplier", parameters.fZMultiplier, reference.fZMultiplier);
	PARAM(L"ExcludeNonSNPImages", parameters.bExcludeNonSNPImages, reference.bExcludeNonSNPImages);
	//PARAM(L"MakeMosaik", parameters.makeMosaik);

	// Image Export
	PARAM(L"ForceExportMotionCorrectedImages", parameters.bForceExportMotionCorrectedImages, reference.bForceExportMotionCorrectedImages);

	// Logging
	PARAM(L"DetailedLogging", parameters.bDetailedLogging, reference.bDetailedLogging);

#undef PARAM
#undef PARAM_ENUM
}

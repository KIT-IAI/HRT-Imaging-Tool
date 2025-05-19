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
#include "SNPDatasetParameterLoader.h"
#include "SettingsStore.h"

CSNPDatasetParameters CSNPDatasetParameterLoader::FromSQLite(const wstring& sPath)
{
	CSQLiteDatabase Database(sPath);
	return FromSQLite(Database);
}

CSNPDatasetParameters CSNPDatasetParameterLoader::FromSQLite(CSQLiteDatabase& Database)
{
	return Database.SelectAll(L"Parameters").Convert<CSNPDatasetParameters>();
}

CSNPDatasetParameters CSNPDatasetParameterLoader::FromRegistry(const wstring& regPath)
{
	// We use a signed type variable to read the registry entry, otherwise
	// negative values would be silently misinterpreted. The parsing of the
	// string-typed registry entry into the signed type variable seems to be
	// robust insofar as non-numeric and non-integer entries as well as over-
	// and underflowing integer values (>=2^63 and <-2^63) leave the initial
	// variable value unchanged.
	ptrdiff_t signedMaxSequenceLength = 0;

	CSNPDatasetParameters parameters;

#define PARAM(NAME, VALUE) GetGSS()->ReadValue(regPath,NAME,VALUE,VALUE);
#define PARAM_ENUM(NAME, VALUE) {typename std::underlying_type<decltype(VALUE)>::type tmp = static_cast<typename std::underlying_type<decltype(VALUE)>::type>(VALUE); GetGSS()->ReadValue(regPath, NAME,tmp,tmp); VALUE=static_cast<decltype(VALUE)>(tmp);}

	// Image Import
	PARAM(L"MaxSequenceLength", signedMaxSequenceLength);

	// Registration Preprocessing
	PARAM(L"BrightnessCorrectionBeforeRegistration", parameters.bBrightnessCorrectionBeforeRegistration);
	PARAM(L"VignettingCorrectionBeforeRegistration", parameters.bVignettingCorrectionBeforeRegistration);
	PARAM(L"ShiftCorrection", parameters.bShiftCorrection);
	PARAM(L"VignettingFileBeforeRegistration", parameters.sVignettingFileBeforeRegistration);
	PARAM(L"VignettingSmoothingBeforeRegistration", parameters.nVignettingSmoothingBeforeRegistration);
	PARAM(L"VignettingCreationMaxImages", parameters.nVignettingCreationMaxImages);

	// Registration Strategy
	PARAM_ENUM(L"ProcessType", parameters.eProcessType);
	PARAM_ENUM(L"Solver", parameters.eSolver);
	PARAM(L"MaxImageDistance", parameters.nMaxImageDistance);
	PARAM(L"MaxRegDistance", parameters.nMaxRegDistance);
	PARAM(L"WindowSize", parameters.nWindowSize);
	PARAM(L"StepSize", parameters.nStepSize);
	PARAM(L"SectorSize", parameters.nSectorSize);
	PARAM(L"SelectionHeight", parameters.fSelectionHeight);

	// Registration Procedure
	PARAM_ENUM(L"ImagePairVerificator", parameters.eImagePairVerificator);
	PARAM_ENUM(L"Strategy", parameters.eStrategie);

	// Correlation Procedure
	PARAM(L"SubImageHeight", parameters.nSubImageHeight);
	PARAM(L"ConsistencyCheck", parameters.bConsistencyCheck);
	PARAM(L"Scalation", parameters.fScalation);
	PARAM(L"ScalationSubImages", parameters.fScalationSubImages);

	// Score Parameters
	PARAM(L"MinScore", parameters.fMinScore);
	PARAM(L"CertainScore", parameters.fCertainScore);
	PARAM(L"MinScoreFlexible", parameters.fMinScoreFlexible);
	PARAM(L"AutomaticThresholdDetection", parameters.bAutomaticThresholdDetection);

	// Compositing Preprocessing
	PARAM(L"VignettingCorrectionBeforeCompositing", parameters.bVignettingCorrectionBeforeCompositing);
	PARAM(L"BrightnessCorrectionBeforeCompositing", parameters.bBrightnessCorrectionBeforeCompositing);
	PARAM(L"VignettingFileBeforeCompositing", parameters.sVignettingFileBeforeCompositing);
	PARAM(L"VignettingSmoothingBeforeCompositing", parameters.nVignettingSmoothingBeforeCompositing);

	// Compositing
	PARAM_ENUM(L"WeightMode", parameters.eWeightMode);
	PARAM(L"Border", parameters.nBorder);
	PARAM(L"BackgroundColor", parameters.cBackgroundColor);
	PARAM_ENUM(L"CompositingMode", parameters.eCompositingMode);
	PARAM(L"f3dIntensityLimit", parameters.fIntensityLimit);
	PARAM(L"f3dZMultiplier", parameters.fZMultiplier);
	PARAM(L"ExcludeNonSNPImages", parameters.bExcludeNonSNPImages);
	//PARAM(L"MakeMosaik", parameters.makeMosaik);

	// Image Export
	PARAM(L"ForceExportMotionCorrectedImages", parameters.bForceExportMotionCorrectedImages);

	// Logging
	PARAM(L"DetailedLogging", parameters.bDetailedLogging);

#undef PARAM
#undef PARAM_ENUM

	// Only assign positive registry entries to the (unsigned) size_t parameter.
	if (signedMaxSequenceLength > 0)
		parameters.nMaxSequenceLength = signedMaxSequenceLength;

	return parameters;
}

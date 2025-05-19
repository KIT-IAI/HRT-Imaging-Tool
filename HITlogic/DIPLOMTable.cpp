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
#include "DIPLOMTable.h"
#include "DIPLOMTableRow.h"

CSQLiteTable CDIPLOMTable::From(const wstring& Name, const CSNPDatasetOutputParameters& Parameters)
{
	CSQLiteTable Table(Name);

	Table.AddColumn(L"Name", L"TEXT PRIMARY KEY");
	Table.AddColumn(L"Value", L"TEXT");

#define ADD_PARAM(NAME, VALUE)	Table.AddRow(CSQLiteTableRow::From(vector<wstring>{ NAME, CStringUtilities::ToString(VALUE) }));

	ADD_PARAM(L"ImageCount", Parameters.nImageCount);
	ADD_PARAM(L"ProcessedImageSizeX", Parameters.ImageParameters.ImageSize.x);
	ADD_PARAM(L"ProcessedImageSizeY", Parameters.ImageParameters.ImageSize.y);
	ADD_PARAM(L"SubImageHeight", Parameters.ImageParameters.nSubImageHeight);
	ADD_PARAM(L"SubImagesPerImageWithoutGap", Parameters.ImageParameters.nSubImagesPerImageWithoutGap);
	ADD_PARAM(L"GapsBeforeImage", Parameters.ImageParameters.nGapsBeforeImage);
	ADD_PARAM(L"GapsAfterImage", Parameters.ImageParameters.nGapsAfterImage);
	ADD_PARAM(L"ColumnCount", Parameters.ImageParameters.nColumnCount);
	ADD_PARAM(L"Date", Parameters.sDate);
	ADD_PARAM(L"Computer", Parameters.sComputer);
	ADD_PARAM(L"User", Parameters.sUser);
	ADD_PARAM(L"ScoreThreshold", Parameters.fDetectedThreshold);
	ADD_PARAM(L"DetectedImageGroups", Parameters.nDetectedImageGroups);

#undef ADD_PARAM

	return Table;
}

template <> CSNPDatasetOutputParameters CSQLiteTable::Convert() const
{
	if (size() == 0)
		return{};

	CSNPDatasetOutputParameters Parameters;

#define APPLY_PARAM(NAME,VALUE) if (TableRow.Get<wstring>(0) == NAME) {VALUE = TableRow.Get<decltype(VALUE)>(1);continue;}

	for (const auto& TableRow : *this)
	{
		APPLY_PARAM(L"ImageCount", Parameters.nImageCount);
		APPLY_PARAM(L"ProcessedImageSizeX", Parameters.ImageParameters.ImageSize.x);
		APPLY_PARAM(L"ProcessedImageSizeY", Parameters.ImageParameters.ImageSize.y);
		APPLY_PARAM(L"SubImageHeight", Parameters.ImageParameters.nSubImageHeight);
		APPLY_PARAM(L"SubImagesPerImageWithoutGap", Parameters.ImageParameters.nSubImagesPerImageWithoutGap);
		APPLY_PARAM(L"GapsBeforeImage", Parameters.ImageParameters.nGapsBeforeImage);
		APPLY_PARAM(L"GapsAfterImage", Parameters.ImageParameters.nGapsAfterImage);
		APPLY_PARAM(L"ColumnCount", Parameters.ImageParameters.nColumnCount);
		APPLY_PARAM(L"Date", Parameters.sDate);
		APPLY_PARAM(L"Computer", Parameters.sComputer);
		APPLY_PARAM(L"User", Parameters.sUser);
		APPLY_PARAM(L"ScoreThreshold", Parameters.fDetectedThreshold);
		APPLY_PARAM(L"DetectedImageGroups", Parameters.nDetectedImageGroups);

		/*
		* If you hit this assertion it means that the SQLite database contains a parameter that is unknown.
		* Don't worry, this is not too much of a problem. The parameter will be ignored.
		* But the question remains. Where did the parameter come from? What does it want from us?
		* Did an evil government send it to intrude our system? Was is an alien trying to warn us?
		* We don't know. And we never will. Or at least, that's what the government wants you to think.
		*/
		ASSERT(false);
	}

#undef APPLY_PARAM

	return Parameters;
}

CSQLiteTable CDIPLOMTable::From(const wstring& Name, const vector<CRigidRegistrationResult>& RegResults)
{
	CSQLiteTable Table(Name);

	Table.AddColumn(L"X", L"REAL");
	Table.AddColumn(L"Y", L"REAL");
	Table.AddColumn(L"Phi", L"REAL");
	Table.AddColumn(L"Dx", L"REAL");
	Table.AddColumn(L"Dy", L"REAL");
	Table.AddColumn(L"Score", L"REAL");
	Table.AddColumn(L"BlockIndex", L"INTEGER");
	Table.AddColumn(L"SubImageRowIndex", L"INTEGER");
	Table.AddColumn(L"SubImageColumnIndex", L"INTEGER");
	Table.AddColumn(L"SubImageHeight", L"INTEGER");
	Table.AddColumn(L"SpecialImageRowIndex", L"INTEGER");
	Table.AddColumn(L"SpecialImageColumnIndex", L"INTEGER");
	Table.AddColumn(L"ReferenceImageIndex", L"INTEGER");
	Table.AddColumn(L"TemplateImageIndex", L"INTEGER");
	Table.AddColumn(L"ScoreNorm2", L"REAL");
	Table.AddColumn(L"ScoreBeforeNorm", L"REAL");
	Table.AddColumn(L"Validity", L"INTEGER");
	Table.AddColumn(L"SpecialSubImageRowIndex", L"INTEGER");
	Table.AddColumn(L"OffsetStartingPointX", L"REAL");
	Table.AddColumn(L"OffsetStartingPointY", L"REAL");

	for (const auto& RegResult : RegResults)
		Table.AddRow(CDIPLOMTableRow::From(RegResult));

	return Table;
}

template <> vector<CRigidRegistrationResult> CSQLiteTable::Convert() const
{
	if (size() == 0)
		return{};

	if (!std::all_of(begin(), end(), [this](const CSQLiteTableRow& Row) {return Row.size() == (*begin()).size(); }))
		throw std::exception("Non-uniform table cannot be converted into a registration result.");

	vector<CRigidRegistrationResult> Result;

	for (const auto& TableRow : *this)
		Result.push_back(TableRow.Convert<CRigidRegistrationResult>());

	return Result;
}

CSQLiteTable CDIPLOMTable::From(const wstring& Name, const CSNPDatasetParameters& Parameters)
{
	CSQLiteTable Table(Name);
	Table.AddColumn(L"Name", L"TEXT PRIMARY KEY");
	Table.AddColumn(L"Value", L"TEXT");

#define ADD_PARAM(name,value) Table.AddRow(CSQLiteTableRow::From({name,CStringUtilities::ToString(value)}));
#define ADD_ENUM_PARAM(name,value) Table.AddRow(CSQLiteTableRow::From({name,CStringUtilities::ToString(static_cast<int>(value))}));

	vector<std::pair<wstring, wstring>> TableRows;

	ADD_PARAM(L"nMaxSequenceLength", Parameters.nMaxSequenceLength)
		ADD_ENUM_PARAM(L"eProcessType", Parameters.eProcessType)
		ADD_ENUM_PARAM(L"eSolver", Parameters.eSolver)
		ADD_PARAM(L"nMaxImageDistance", Parameters.nMaxImageDistance)
		ADD_PARAM(L"nMaxRegDistance", Parameters.nMaxRegDistance)
		ADD_PARAM(L"nWindowSize", Parameters.nWindowSize)
		ADD_PARAM(L"nStepSize", Parameters.nStepSize)
		ADD_PARAM(L"nSectorSize", Parameters.nSectorSize)
		ADD_PARAM(L"fSelectionHeight", Parameters.fSelectionHeight)
		ADD_PARAM(L"bBrightnessCorrectionBeforeRegistration", Parameters.bBrightnessCorrectionBeforeRegistration)
		ADD_PARAM(L"bVignettingCorrectionBeforeRegistration", Parameters.bVignettingCorrectionBeforeRegistration)
		ADD_PARAM(L"bShiftCorrection", Parameters.bShiftCorrection)
		ADD_PARAM(L"sVignettingFileBeforeRegistration", Parameters.sVignettingFileBeforeRegistration)
		ADD_ENUM_PARAM(L"eImagePairVerificator", Parameters.eImagePairVerificator)
		ADD_ENUM_PARAM(L"eStrategie", Parameters.eStrategie)
		ADD_PARAM(L"nSubImageHeight", Parameters.nSubImageHeight)
		ADD_PARAM(L"bConsistencyCheck", Parameters.bConsistencyCheck)
		ADD_PARAM(L"fScalation", Parameters.fScalation)
		ADD_PARAM(L"fScalationSubImages", Parameters.fScalationSubImages)
		ADD_PARAM(L"fMinScore", Parameters.fMinScore)
		ADD_PARAM(L"fCertainScore", Parameters.fCertainScore)
		ADD_PARAM(L"fMinScoreFlexible", Parameters.fMinScoreFlexible)
		ADD_PARAM(L"bAutomaticThresholdDetection", Parameters.bAutomaticThresholdDetection)
		ADD_PARAM(L"bVignettingCorrectionBeforeCompositing", Parameters.bVignettingCorrectionBeforeCompositing)
		ADD_PARAM(L"bBrightnessCorrectionBeforeCompositing", Parameters.bBrightnessCorrectionBeforeCompositing)
		ADD_PARAM(L"sVignettingFileBeforeCompositing", Parameters.sVignettingFileBeforeCompositing)
		ADD_ENUM_PARAM(L"eWeightMode", Parameters.eWeightMode)
		ADD_PARAM(L"nBorder", Parameters.nBorder)
		ADD_PARAM(L"cBackgroundColor", Parameters.cBackgroundColor)
		ADD_ENUM_PARAM(L"eCompositingMode", Parameters.eCompositingMode)
		ADD_PARAM(L"f3dIntensityLimit", Parameters.fIntensityLimit)
		ADD_PARAM(L"f3dZMultiplier", Parameters.fZMultiplier)
		ADD_PARAM(L"nVignettingCreationMaxImages", Parameters.nVignettingCreationMaxImages)
		ADD_PARAM(L"nVignettingSmoothingBeforeRegistration", Parameters.nVignettingSmoothingBeforeRegistration)
		ADD_PARAM(L"nVignettingSmoothingBeforeCompositing", Parameters.nVignettingSmoothingBeforeCompositing)
		ADD_PARAM(L"bExcludeNonSNPImages", Parameters.bExcludeNonSNPImages)
		ADD_PARAM(L"bForceExportMotionCorrectedImages", Parameters.bForceExportMotionCorrectedImages)
		ADD_PARAM(L"bDetailedLogging", Parameters.bDetailedLogging)

#undef ADD_PARAM
#undef ADD_ENUM_PARAM

		return Table;
}

template <> CSNPDatasetParameters CSQLiteTable::Convert() const
{
	CSNPDatasetParameters Param;

#define ADD_PARAM(name,value) if(ParameterName == name){CStringUtilities::ParseString(ParameterValue,value); continue;}
#define ADD_ENUM_PARAM(name,value) if(ParameterName == name){int temp; CStringUtilities::ParseString(ParameterValue,temp); value = static_cast<decltype(value)>(temp); continue;}

	for (const auto& Row : *this)
	{
		auto ParameterName = Row.Get<wstring>(0);
		auto ParameterValue = Row.Get<wstring>(1);

		ADD_PARAM(L"nMaxSequenceLength", Param.nMaxSequenceLength);
		ADD_ENUM_PARAM(L"eProcessType", Param.eProcessType);
		ADD_ENUM_PARAM(L"eSolver", Param.eSolver);
		ADD_PARAM(L"nMaxImageDistance", Param.nMaxImageDistance);
		ADD_PARAM(L"nMaxRegDistance", Param.nMaxRegDistance);
		ADD_PARAM(L"nWindowSize", Param.nWindowSize);
		ADD_PARAM(L"nStepSize", Param.nStepSize);
		ADD_PARAM(L"nSectorSize", Param.nSectorSize);
		ADD_PARAM(L"fSelectionHeight", Param.fSelectionHeight);
		ADD_PARAM(L"bBrightnessCorrectionBeforeRegistration", Param.bBrightnessCorrectionBeforeRegistration);
		ADD_PARAM(L"bVignettingCorrectionBeforeRegistration", Param.bVignettingCorrectionBeforeRegistration);
		ADD_PARAM(L"bShiftCorrection", Param.bShiftCorrection);
		ADD_PARAM(L"sVignettingFileBeforeRegistration", Param.sVignettingFileBeforeRegistration);
		ADD_ENUM_PARAM(L"eImagePairVerificator", Param.eImagePairVerificator);
		ADD_ENUM_PARAM(L"eStrategie", Param.eStrategie);
		ADD_PARAM(L"nSubImageHeight", Param.nSubImageHeight);
		ADD_PARAM(L"bConsistencyCheck", Param.bConsistencyCheck);
		ADD_PARAM(L"fScalation", Param.fScalation);
		ADD_PARAM(L"fScalationSubImages", Param.fScalationSubImages);
		ADD_PARAM(L"fMinScore", Param.fMinScore);
		ADD_PARAM(L"fCertainScore", Param.fCertainScore);
		ADD_PARAM(L"fMinScoreFlexible", Param.fMinScoreFlexible);
		ADD_PARAM(L"bAutomaticThresholdDetection", Param.bAutomaticThresholdDetection);
		ADD_PARAM(L"bVignettingCorrectionBeforeCompositing", Param.bVignettingCorrectionBeforeCompositing);
		ADD_PARAM(L"bBrightnessCorrectionBeforeCompositing", Param.bBrightnessCorrectionBeforeCompositing);
		ADD_PARAM(L"sVignettingFileBeforeCompositing", Param.sVignettingFileBeforeCompositing);
		ADD_ENUM_PARAM(L"eWeightMode", Param.eWeightMode);
		ADD_PARAM(L"nBorder", Param.nBorder);
		ADD_PARAM(L"cBackgroundColor", Param.cBackgroundColor);
		ADD_ENUM_PARAM(L"eCompositingMode", Param.eCompositingMode);
		ADD_PARAM(L"f3dIntensityLimit", Param.fIntensityLimit)
			ADD_PARAM(L"f3dZMultiplier", Param.fZMultiplier)
			ADD_PARAM(L"nVignettingCreationMaxImages", Param.nVignettingCreationMaxImages);
		ADD_PARAM(L"nVignettingSmoothingBeforeRegistration", Param.nVignettingSmoothingBeforeRegistration);
		ADD_PARAM(L"nVignettingSmoothingBeforeCompositing", Param.nVignettingSmoothingBeforeCompositing);
		ADD_PARAM(L"bExcludeNonSNPImages", Param.bExcludeNonSNPImages);
		ADD_PARAM(L"bForceExportMotionCorrectedImages", Param.bForceExportMotionCorrectedImages);
		ADD_PARAM(L"bDetailedLogging", Param.bDetailedLogging);

		/*
		* If you hit this assertion it means that the SQLite database contains a parameter that is unknown.
		* Don't worry, this is not too much of a problem. The parameter will be ignored.
		* But the question remains. Where did the parameter come from? What does it want from us?
		* Did an evil government send it to intrude our system? Was is an alien trying to warn us?
		* We don't know. And we never will. Or at least, that's what the government wants you to think.
		*/
		ASSERT(false);
	}

#undef ADD_PARAM
#undef ADD_ENUM_PARAM

	return Param;
}

CSQLiteTable CDIPLOMTable::From(const wstring& Name, const vector<CMosaicImageProperties>& Properties)
{
	CSQLiteTable Table(Name);
	Table.AddColumn(L"GroupID", L"INTEGER PRIMARY KEY");
	Table.AddColumn(L"MosaicImageSizeX", L"INTEGER");
	Table.AddColumn(L"MosaicImageSizeY", L"INTEGER");
	Table.AddColumn(L"DefinedMosaicArea", L"INTEGER");
	Table.AddColumn(L"FilePath", L"TEXT");

	for (const auto& Property : Properties)
		Table.AddRow(CDIPLOMTableRow::From(Property));
	return Table;
}

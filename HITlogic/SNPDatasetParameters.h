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

#include "ProcessType.h"
#include "RegistrationStrategy.h"
#include "HrtPreProcessingParameters.h"
#include "GlobalPositioningParameters.h"
#include "RegistrationProcedureParameters.h"
#include "HrtImageParameters.h"
#include "CompositingParameters.h"
#include "RegistrationScoreParameters.h"

class CSNPDatasetParameters
{
public:
	static const std::wstring TableName;
	CSNPDatasetParameters() = default;
	~CSNPDatasetParameters() = default;

	/************************************************************
	******************* VERY IMPORTANT NOTICE *******************
	*************************************************************
	***                                                       ***
	*** If you add a parameter to this file make sure:        ***
	*** 1. It is an INPUT parameter                           ***
	*** 2. It is not an OUTPUT parameter                      ***
	*** 3. You added a line in:                               ***
	***                                                       ***
	***        CSNPDatasetParameters::operator==              ***
	***                 CDIPLOMTable::Convert                 ***
	***                 CDIPLOMTable::From                    ***
	***   CSNPDatasetParameterLoader::FromRegistry            ***
	*** CSNPDatasetParameterExporter::ExportChangesToRegistry ***
	***                                                       ***
	*************************************************************
	************************************************************/

	CHrtPreProcessingParameters			GeneratePreProcessingParameters() const;
	CHrtPreProcessingParameters			GenerateCompositingPreProcessingParameters() const;

	CRegistrationStrategyParameters		GenerateRegistrationStrategyParameters(StlImageSize ImageSize) const;
	CRegistrationVerificationParameters GenerateRegistrationVerificationParameters(StlImageSize ImageSize) const;
	CRegistrationProcedureParameters	GenerateRegistrationProcedureParameters(StlImageSize ImageSize) const;
	CCorrelationParameters				GenerateCorrelationParameters() const;

	CGlobalPositioningParameters		GenerateGlobalPositioningParameters() const;

	CHrtImageParameters					GenerateImageParameters(StlImageSize ImageSize) const;

	CRegistrationScoreParameters		GenerateScoreParameters() const;
	CCompositingParameters				GenerateCompositingParameters() const;

	bool								IsValidParameterset() const;
	bool								VignettingProfileBeforeRegistrationIsValid() const;
	bool								VignettingProfileBeforeCompositingIsValid() const;

	// Image Import
	size_t										nMaxSequenceLength = 0;									// Index = 0

	// Registration Preprocessing
	bool										bBrightnessCorrectionBeforeRegistration = false;		// Index = 1
	bool										bVignettingCorrectionBeforeRegistration = true;			// Index = 2
	bool										bShiftCorrection = false;								// Index = 3
	std::wstring								sVignettingFileBeforeRegistration;						// Index = 4
	size_t										nVignettingSmoothingBeforeRegistration = 0;				// Index = 5
	size_t										nVignettingCreationMaxImages = 0;						// Index = 6

	// Registration Strategy
	CProcessType::EProcessType					eProcessType = CProcessType::eHRTImageRegistration;		// Index = 7
	CSLESolver::EAlgorithm						eSolver = CSLESolver::EAlgorithm::eCGALGLIB;			// Index = 8
	long										nMaxImageDistance = 400;								// Index = 9
	long										nMaxRegDistance = 8;									// Index = 10
	long										nWindowSize = 3;										// Index = 11
	long										nStepSize = 3;											// Index = 12
	long										nSectorSize = 30;										// Index = 13
	//new ones at index 30

	// Registration Procedure
	EImagePairVerificator						eImagePairVerificator = EImagePairVerificator::eNone;	// Index = 14
	ERegistrationStrategy						eStrategie = ERegistrationStrategy::eComplete;			// Index = 15

	// Correlation Procedure
	size_t										nSubImageHeight = 32;									// Index = 16
	bool										bConsistencyCheck = false;								// Index = 17
	float										fScalation = 3.0;										// Index = 18
	float										fScalationSubImages = 2.0;								// Index = 19

	// Score Parameters
	double										fMinScore = 8.0;										// Index = 20
	double										fCertainScore = 24.0;									// Index = 21
	double										fMinScoreFlexible = 16.0;								// Index = 22
	std::shared_ptr<double>						fScoreThreshold = std::make_shared<double>(fMinScore);	// Index = 23
	bool										bAutomaticThresholdDetection = false;					// Index = 24
	double										fResidualThreshold = 10.0;

	// Compositing Preprocessing
	bool										bVignettingCorrectionBeforeCompositing = true;			// Index = 25
	bool										bBrightnessCorrectionBeforeCompositing = false;			// Index = 26
	std::wstring								sVignettingFileBeforeCompositing;						// Index = 27
	size_t										nVignettingSmoothingBeforeCompositing = 0;				// Index = 28

	// New Registration Strategy
	//size_t										nPlaneCount = 1;									// Index = 29
	//size_t										nPlaneIncrement = 1;								// Index = 30
	double										fSelectionHeight = 1.0;									// Index = 31

	// Compositing
	CCompositingParameters::EWeightMode			eWeightMode = CCompositingParameters::EWeightMode::eCos2;				/**	\brief Gewichtung der Quellbilder beim Erzeugen des Mosaikbilds.*/
	size_t										nBorder = 0;															/**	\brief Größe der Umrandung des Mosaikbilds.*/
	double										cBackgroundColor = 0.0;													/**	\brief Hintergrundfarbe des Mosaikbilds.*/
	CCompositingParameters::ECompositingType	eCompositingMode = CCompositingParameters::ECompositingType::eFusion;	/**	\brief Algorithmus zur Bildmontage.*/
	float										fIntensityLimit = 0.0;													/**	Für 3D-Compositer: Wenn Punkte herausgefiltert werden sollen.*/
	float										fZMultiplier = 2.0;														/**	Für 3D-Compositer: Legt fest ob Punktwolke in x_achse gestreckt werden soll.*/
	bool										bExcludeNonSNPImages = false;

	// Export
	bool										bForceExportMotionCorrectedImages = false;								/**	\brief Force file export of motion-corrected images.*/

	// Logging
	bool										bDetailedLogging = false;												/**	\brief Enable detailed process log entries.*/

	friend bool operator==(const CSNPDatasetParameters& lhs, const CSNPDatasetParameters& rhs);
	friend bool operator!=(const CSNPDatasetParameters& lhs, const CSNPDatasetParameters& rhs);
	
private:
	static std::filesystem::path GetExePath();
};

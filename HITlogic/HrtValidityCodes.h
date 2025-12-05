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

class CHrtValidityCodes
{
public:

	enum EValidityCode
	{
		// all the following values are currently unused:
		//	  4 (eValidRoughCorrelationFlexibleCheckChangedDirection)
		//	 14 (eValidSubImageScoreLocalDistortion)
		//	 -3 (eInvalidSubImageOffset)
		//	 -6 (eInvalidMaxScorePosition)
		//	 -7 (eInvalidGradient)
		//	-14 (eInvalidSubImageScoreLocalDistortion)
		//	-15 (eInvalidRoughCorrelationBothChangedDirection)   wird nur in Tests verwendet
		//	-44 (eInvalidMaximumDueToStd)
		//	-47 (eLonelyRegistration)

		///	<summary> Initialisierung des Gültigkeitsflags </summary>
		eValidityInitialization = 0,

		/// <summary> Grobregistrierung MinScore erfüllt - Nebenbedingung: Offset in X-Richtung &lt; max. Offset </summary>
		eValidRoughCorrelationMinScore = 1,

		/// <summary> The overlapping image region is empty after rounding the rigid registration result </summary>
		eInvalidEmptyOverlappingArea = -1,

		/// <summary> Grobregistrierung mit Hilfe der Teilbildregistrierung überprüft: Offset in X-Richtung &gt; max. Offset oder Score &lt; min. Score (jedoch mind. (min. Score * 0.5) </summary>
		eValidRoughCorrelationFlexibleChecked = 2,

		/// <summary> Grobregistrierung lieferte keine Ergebnis </summary>
		eInvalidRoughCorrelationNoResult = -2,

		/// <summary> Offset der Teilbildregistrierung ist in y &lt;= 10 und in x &lt;= 40 </summary>
		eValidSubImageOffset = 3,

		/// <summary> Offset der Teilbildregistrierung ist in y &gt; 10 und in x &gt; 40 </summary>
		eInvalidSubImageOffset = -3,

		/// <summary> Grobregistrierung mit Hilfe der Teilbildregistrierung überprüft nach dem die Richtung gewechselt wurde: Offset in X-Richtung &gt; max. Offset </summary>
		eValidRoughCorrelationFlexibleCheckChangedDirection = 4,

		/// <summary> Mindestanzahl erfolgreich registrierter Teilbilder nicht erreicht </summary>
		eInvalidSubImagesCriterion = -5,

		/// <summary> Suche nach der Bildzeile, die die beste Korrelation aufweist, nicht erfolgreich </summary>
		eInvalidMaxScorePosition = -6,

		/// <summary> Gradient fabs(dux/dy) oder fabs(duy/dy) ist größer als 1 </summary>
		eInvalidGradient = -7,

		/// <summary> Das aktuelle Fenster (Referenzbild) liegt außerhalb des Templatebilds </summary>
		eInvalidSubImageSize = -9,

		/// <summary> maximale Anzahl Iterationen bei der Teilbildregistrierung erreicht und Konvergenzkriterium nicht erfüllt </summary>
		eInvalidMaxNumberOfIterations = -10,

		/// <summary> trotz maximaler Anzehl Iterationen konnte der min. Score nicht erreicht werden </summary>
		eInvalidMinScoreAfterAllIterations = -11,

		/// <summary> Grobregistrierung: Scorewert &lt; (min. Score * 0,5) </summary>
		eInvalidRoughCorrelationMinScore = -12,

		/// <summary> Überprüfung der Grobregistrierung mit Hilfe der Teilbildregistrierung nicht erfolgreich </summary>
		eInvalidRoughCorrelationFlexibleChecked = -13,

		/// <summary> Scorewert bei der Teilbildregistrierung (lokale Entzerrung) ausreichend </summary>
		eValidSubImageScoreLocalDistortion = 14,

		/// <summary> Scorewert bei der Teilbildregistrierung (lokale Entzerrung) nicht ausreichend </summary>
		eInvalidSubImageScoreLocalDistortion = -14,

		/// <summary> Grobregistrierung mit Hilfe der Teilbildregistrierung überprüft Richtungswechsel mehrfach erfolgreich (Ausnahmefall) </summary>
		eInvalidRoughCorrelationBothChangedDirection = -15,

		/// <summary> Grobregistrierung Nullverschiebung nicht zulässig (Ausnahmefall) </summary>
		eInvalidZeroTranslationRigidRegistration = -16,

		/// <summary> Consistency Check Failed </summary>
		eConsistencyCheckFailed = -42,

		/// <summary> If the fromString Constructor failed </summary>
		eInvalidInit = -43,

		/// <summary> The Standart-Deviation was too low to find a maximum in the Phase-Image </summary>
		eInvalidMaximumDueToStd = -44,

		/// <summary> Was Explicitly Created as Invalid-Offset </summary>
		eDeliberatlyInvalid = -45,

		/// <summary> Invalid Due To High Residual </summary>
		eInvalidDueToHighResidual = -46,

		/// <summary> Removed lonely registration after ResidualFilter </summary>
		eLonelyRegistration = -47,

		/// <summary> ImagePairVerifier doesn't think these images will overlap. </summary>
		eInvalidImagePairVerification = -48,

		/// <summary> Subimage registration was thought plausible through the newly introduced plausibility-check </summary>
		eThoughtPlausible = 15
	};
};

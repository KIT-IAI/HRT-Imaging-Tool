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


#include "RegistrationStrategy.h"


/**	\brief Die Klasse CStrategyBackForth implementiert eine Strategie zur
 *	Auswahl von Bildpaaren bei der Registrierung von Bildserien, bei der
 *	bevorzugt Bildpaare mit kleinem Abstand in der Bildserie registriert
 *	werden.
 *
 *	Diese Strategie versucht zunächst, direkt benachbarte Bildpaare zu
 *	registrieren. Erst danach wird versucht, die verbleibenden
 *	"Bruch"-Stellen mit zunehmend größer werdenden Sprüngen zu überbrücken.
 *	Diese Strategie führt stets zu einem nicht-redundanten Satz an
 *	Bildregistrierungen, d.h. wenn für ein Bildpaar \f$i,j\f$ eine Kette von
 *	Registrierungen \f$(i,k_1),(k_1,k_2),\ldots,(k_{m-1},k_m),(k_m,j)\f$
 *	existiert, dann ist diese eindeutig. Diese Vorgehensweise ist für
 *	kontinuierlich aufgenommene Bildserien häufig recht effizient, arbeitet
 *	nach der initialen Registrierung aller benachbarter Bildpaare allerdings
 *	nur noch sequenziell und ist aufgrund des nicht-redundanten Entwurfs
 *	nicht in der Lage, fehlerbehaftete Registrierungen zu kompensieren.
 *
 *	Im Best Case werden für eine Bildserie mit \f$n\f$ Bildern nur alle
 *	Bildpaare \f$(i,j)\f$ mit \f$0\le i<i+1=j<n\f$ registriert. Der
 *	Laufzeitaufwand beläuft sich dann folglich auf \f$O(n)\f$ oder
 *	\f$T(n-1)\f$. Im Worst Case werden alle Bildpaare \f$(i,j)\f$ mit
 *	\f$0\le i<j<n\f$ registriert, der Laufzeitaufwand liegt dann bei
 *	\f$O(n^2)\f$ oder \f$T(n*(n-1)/2)\f$. Für Bildserien, die am Ende zu
 *	einer Gruppierung der Bilder zu einer einzigen großen Gruppe und wenigen
 *	kleinen Gruppen (oder nicht-registrierten Einzelbildern) führen, liegt
 *	der mittlere Aufwand eher im linearen Bereich. Je fragmentierter die
 *	resultierende Gruppierung der Bilder ausfällt, desto eher tendiert der
 *	mittlere Aufwand in den quadratischen Bereich.
 *
 *	Aufgrund des nicht-redundanten Vorgehens dieser Strategie beeinflusst der
 *	Registriererfolg eines Bildpaars die nachfolgend zu registrierenden
 *	Bildpaare. Aus diesem Grund kann nach der initialen Registrierung aller
 *	benachbarter Bildpaare nicht mehr parallel gearbeitet werden. Der
 *	Registriererfolg einer Bildregistrierung wird mit SetSuccess() übergeben.
 *	Intern werden auf der Basis dieser Erfolgswerte die Gruppen
 *	zusammenhängender Bilder verwaltet, die direkt oder über eine Folge
 *	mehrerer Registrierungen zueinander in Bezug gesetzt werden können. Diese
 *	Bildgruppen lassen sich nach der Registrierung der Bildserie durch
 *	GetRegistrationImageGroups() abfragen.
 *
 *	\author Stephan Allgeier
 */
class CStrategyBackForth : public CRegistrationStrategy
{

public:

	CStrategyBackForth();
	virtual ~CStrategyBackForth();

public:

	virtual void SetParameters(const CRegistrationStrategyParameters& parameters) override;
	size_t GetMaxDistance() const;

	virtual void Initialize(size_t nImages) override;

protected:

	virtual size_t InitialWorkloadEstimation() const override;
	virtual bool PrepareNextPair() override;

private:

	/**	\brief Der maximale Abstand eines zu registrierenden Bildpaars.*/
	size_t m_nMaxDistance;

	/**	\brief Der aktuelle Abstand für zu registrierende Bildpaare.*/
	size_t m_nDistance;

private:

	/**	\brief Definiert die möglichen Zustände des Zustandsautomaten des Objekts.*/
	enum EState
	{
		/**	\brief Das Objekt befindet sich in der Phase der initialen Registrierung benachbarter Bildpaare.*/
		eConsecutiveRegs,
		/**	\brief Das Objekt befindet sich in der Phase der Registrierung nicht benachbarter Bildpaare.*/
		eDistantRegs
	};

	/**	\brief Der aktuelle Zustand des Zustandsautomaten des Objekts.*/
	EState m_eState;

};

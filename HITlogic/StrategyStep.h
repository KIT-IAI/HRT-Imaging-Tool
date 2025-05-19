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


/**	\brief Die Klasse CStrategyStep implementiert eine Strategie zur Auswahl
 *	von Bildpaaren bei der Registrierung von Bildserien, bei der die
 *	registrierten Bildpaare in der Bildserie jeweils einen festem Abstand
 *	zueinander oder ein Vielfaches davon besitzen.
 *
 *	Die Größe des definierten Abstands eines Bildpaars, die Schrittweite, ist
 *	einstellbar, ebenso der maximale Abstand eines zu registrierenden
 *	Bildpaars. Diese Registrierstrategie ist prinzipiell in der Lage,
 *	beliebig große Lücken nicht kontinuierlich registrierbarer Bilder in der
 *	Bildserie zu "überspringen", wofür die Schrittweite \f$s\f$ aber
 *	ausreichend klein und gegebenenfalls der maximal Abstand \f$d\f$
 *	ausreichend groß gewählt werden müssen. Mit abnehmender Schrittweite und
 *	zunehmendem maximalen Abstand nimmt der Aufwand, bezogen auf die Anzahl
 *	der durchzuführenden Bildregistrierungen, zu. Die Menge der
 *	durchgeführten Einzelregistrierungen ist (bei geeignet gewählter
 *	Schrittweite) hoch-redundant, mit redundanten Registrierungen auch über
 *	größe Bildabstände hinweg, sodass die Qualität des Ergebnisses potenziell
 *	nah am erreichbaren Optimum liegt.
 *
 *	Für eine Bildserie mit \f$n\f$ Bildern werden alle Bildpaare \f$(i,j)\f$
 *	mit \f$0\le i<j<n\f$ und \f$j=i+1+k*s\le i+d (k\in N_0)\f$ registriert.
 *	Der Laufzeitaufwand beläuft sich auf etwa \f$O(n*m-m^2)\f$ oder
 *	\f$T(n*m-(m*(m+1))/2)\f$ mit \f$m=d/s\f$.
 *
 *	Der Registriererfolg eines Bildpaars hat bei dieser Strategie keinen
 *	Einfluss auf die nachfolgend zu registrierenden Bildpaare. Intern werden
 *	aber trotzdem anhand der mit SetSuccess() übergebenen Erfolgswerte die
 *	Gruppen zusammenhängender Bilder verwaltet, die direkt oder über eine
 *	Folge mehrerer Registrierungen zueinander in Bezug gesetzt werden können.
 *	Diese Bildgruppen lassen sich nach der Registrierung der Bildserie durch
 *	GetRegistrationImageGroups() abfragen.
 *
 *	\author Stephan Allgeier
 */
class CStrategyStep : public CRegistrationStrategy
{

public:

	CStrategyStep();
	virtual ~CStrategyStep();

public:

	virtual void SetParameters(const CRegistrationStrategyParameters&) override;

	virtual void Initialize(size_t nImages) override;

protected:

	virtual size_t InitialWorkloadEstimation() const override;
	virtual bool PrepareNextPair() override;


private:

	/**	\brief Die Fenstergröße.*/
	size_t m_nWindow;

	/**	\brief Die Schrittweite.*/
	size_t m_nStep;

	/**	\brief Der maximale Abstand eines zu registrierenden Bildpaars.*/
	size_t m_nMaxDistance;

	/**	\brief Der aktuelle Abstand für zu registrierende Bildpaare.*/
	size_t m_nDistance;
};

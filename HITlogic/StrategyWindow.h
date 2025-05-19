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


/**	\brief Die Klasse CStrategyWindow implementiert eine Strategie zur
 *	Auswahl von Bildpaaren bei der Registrierung von Bildserien, bei der
 *	Bildpaare registriert werden, die innerhalb eines zeitlichen Fensters
 *	zueinander aufgenommen wurden.
 *
 *	Die Größe des zeitlichen Fensters ist einstellbar nach der maximalen
 *	Entfernung der zu registrierenden Bildpaare in der Bildserie. Diese
 *	Registrierstrategie erzeugt für kleine Fenstergrößen \f$w\f$ relativ
 *	wenig Aufwand, bezogen auf die Anzahl der durchzuführenden
 *	Bildregistrierungen und erreicht bereits bei einer Fenstergröße von 2
 *	oder 3 in bestimmten Fällen eine deutliche Verbesserung des finalen
 *	Registrierergebnisses. Sie ist in der Lage, einzelne (maximal \f$w-1\f$)
 *	nicht registrierbare Bilder zu "überspringen". Die Menge der
 *	durchgeführten Einzelregistrierungen ist in der Umgebung jedes Bildes
 *	(bezogen auf die Bildposition in der Serie, nicht auf resultierende Lage
 *	des Bildes) redundant, über weitere Entfernungen können sich jedoch
 *	Registrierfehler aufsummieren.
 *
 *	Für eine Bildserie mit \f$n\f$ Bildern werden alle Bildpaare \f$(i,j)\f$
 *	mit \f$0\le i<j<n\f$ und \f$j\le i+w\f$ registriert. Der Laufzeitaufwand
 *	beläuft sich folglich auf \f$O(w*n)\f$ oder \f$T(w*n-w*(w+1)/2)\f$.
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
class CStrategyWindow : public CRegistrationStrategy
{

public:

	CStrategyWindow();
	virtual ~CStrategyWindow();

public:

	virtual void SetParameters(const CRegistrationStrategyParameters&) override;

protected:

	virtual size_t InitialWorkloadEstimation() const override;
	virtual bool PrepareNextPair() override;

private:

	/**	\brief Die Fenstergröße.*/
	size_t m_nWindow;

};

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


/**	\brief Die Klasse CStrategyComplete implementiert eine Strategie zur
 *	Auswahl von Bildpaaren bei der Registrierung von Bildserien, bei der
 *	sämtliche mögliche Bildpaare registriert werden.
 *
 *	Diese Strategie markiert das obere Ende unter den Strategien, sowohl
 *	hinsichtlich der potenziell erreichbaren Ergebnisqualität, als auch
 *	hinsichtlich der Laufzeit. Die Menge der durchgeführten
 *	Einzelregistrierungen ist maximal redundant, der verbleibende Restfehler
 *	ist potenziell minimal.
 *
 *	Für eine Bildserie mit \f$n\f$ Bildern werden alle Bildpaare \f$(i,j)\f$
 *	mit \f$0\le i<j<n\f$ registriert. Der Laufzeitaufwand beläuft sich
 *	folglich auf \f$O(n^2)\f$ oder \f$T(n*(n-1)/2)\f$.
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
class CStrategyComplete : public CRegistrationStrategy
{

public:

	CStrategyComplete();
	virtual ~CStrategyComplete();

protected:

	virtual size_t InitialWorkloadEstimation() const override;
	virtual bool PrepareNextPair() override;
	virtual void SetParameters(const CRegistrationStrategyParameters&);

};

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


/**	\brief Die Klasse CStrategySector implementiert eine Strategie zur
 *	Auswahl von Bildpaaren bei der Registrierung von Bildserien, bei der
 *	gezielt Bildpaare registriert werden, die sich mit hoher
 *	Wahrscheinlichkeit überlappen.
 *
 *	Diese Strategie versucht zunächst, direkt benachbarte oder in kurzer
 *	zeitlicher Folge aufgenommene Bildpaare zu registrieren. Die Größe des
 *	dieses zeitlichen Fensters ist einstellbar als der maximale Abstand
 *	der initial zu registrierenden Bildpaare in der Bildserie. Auf der Basis
 *	dieser initialen Registrierergebnisse wird für alle Bilder der Serie eine
 *	Lageschätzung durchgeführt, um daraus eine Vorhersage des Grads der
 *	Überlappung für jedes Bildpaars ableiten zu können. Anschließend wird für
 *	jedes Bild \f$i\f$ die verbleibende Teilserie in Sektoren fester Größe
 *	geteilt und für jeden Sektor maximal ein weiteres Bildpaar \f$(i,j)\f$
 *	registriert. Das Bild \f$j\f$ wird derart gewählt, dass die Differenz der
 *	geschätzten Lagewerte der Bilder \f$i\f$ und \f$j\f$ innerhalb des
 *	Sektors minimal ist und die Registrierung wird nur dann tatsächlich
 *	durchgeführt, wenn diese Differenz in beiden Dimensionen die
 *	Bildabmessungen nicht übersteigt, die Bilder sich also der Lageschätzung
 *	zufolge überlappen).
 *
 *	Diese Strategie erzeugt einen möglichst hohen Grad an Redundanz in der
 *	Menge der durchgeführten Registrierungen, der mit abnehmender Sektorgröße
 *	\f$s\f$ ansteigt. Durch die Schätzung des Überlappungsgrads bleibt der
 *	Aufwand bezogen auf die Anzahl der durchgeführten Registrierungen auf ein
 *	Minimum reduziert. Die Fähigkeit, nicht registrierbare Abschnitte der
 *	Bildserie zu überbrücken ist durch die Wahl der Fensterbreite \f$w\f$ in
 *	der initialen Phase begrenzt (siehe dazu auch CStrategyWindow).
 *
 *	Für eine Bildserie mit \f$n\f$ Bildern werden alle Bildpaare \f$(i,j)\f$
 *	mit \f$0\le i<j<n\f$ und \f$j\le i+w\f$ registriert, hinzu kommen können
 *	Bildpaare \f$(i,j)\f$ mit \f$0\le i<j<n\f$ und
 *	\f$j=i+w+k*s+s_k (k\in N_0, 0<s_k\le s)\f$. Die untere bzw. obere Grenze
 *	für den Laufzeitaufwand lässt sich (für kleine \f$w\f$) durch \f$O(n)\f$
 *	oder \f$T(w*n)\f$ bzw. durch \f$O(n*m-m^2)\f$ oder
 *	\f$T(n*m-(m*(m+1))/2)\f$ (mit \f$m=d/s\f$) abschätzen.
 *
 *	Der Registriererfolg einer Bildregistrierung wird mit SetSuccess()
 *	übergeben. Intern werden auf der Basis dieser Erfolgswerte die Gruppen
 *	zusammenhängender Bilder verwaltet, die direkt oder über eine Folge
 *	mehrerer Registrierungen zueinander in Bezug gesetzt werden können. Diese
 *	Bildgruppen lassen sich nach der Registrierung der Bildserie durch
 *	GetRegistrationImageGroups() abfragen.
 *
 *	\author Robert Illner
 *	\author Stephan Allgeier
 */
class CStrategySector : public CRegistrationStrategy
{

public:

	CStrategySector();
	virtual ~CStrategySector();

public:

	virtual void SetParameters(const CRegistrationStrategyParameters&) override;

	virtual void Initialize(size_t nImages) override;
	virtual void SetSuccess(size_t nImg1, size_t nImg2, const CRegistrationResult& correlationResult) override;
protected:

	virtual size_t InitialWorkloadEstimation() const override;
	virtual bool PrepareNextPair() override;

private:

	void InitializeSectors();
	bool GetPairFromSector(size_t& nImg1, size_t& nImg2);

private:

	/**	\brief Die Fenstergröße.*/
	size_t m_nWindow;

	/**	\brief Die Sektorgröße.*/
	size_t m_nSectorSize;

	/**	\brief Der maximale Abstand eines zu registrierenden Bildpaars.*/
	size_t m_nMaxDistance;

	/**	\brief Die Bildbreite.*/
	size_t m_nImageSizeX;

	/**	\brief Die Bildhöhe.*/
	size_t m_nImageSizeY;

	/**	\brief Verwaltet die geschätzten horizontalen Distanzen aller Bildpaare.*/
	CDenseMatrix m_matXOffsets;

	/**	\brief Verwaltet die geschätzten vertikalen Distanzen aller Bildpaare.*/
	CDenseMatrix m_matYOffsets;

	/**	\brief Definiert den nächsten zu untersuchenden Sektor.*/
	size_t m_aNextSector[2];

private:

	/**	\brief Definiert die möglichen Zustände des Zustandsautomaten des Objekts.*/
	enum EState
	{
		/**	\brief Das Objekt befindet sich in der initialen Fensterphase.*/
		eWindowRegs,
		/**	\brief Das Objekt wartet auf die letzten Ergebnisse der initialen Fensterphase.*/
		eWait,
		/**	\brief Das Objekt befindet sich in der Sektorphase.*/
		eSectorRegs
	};

	/**	\brief Der aktuelle Zustand des Zustandsautomaten des Objekts.*/
	EState m_eState;

	/**	\brief Bietet eine bessere Kontrolle des Multithreadings als omp critical sections*/
	std::mutex m_criticalMutex;
};

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

#include <atomic>
#include <mutex>
#include <map>

#include "ImagePairVerificator.h"
#include "ImageRegistrationData.h"
#include "RegistrationResult.h"
#include "RegistrationStrategyParameters.h"



enum class ERegistrationStrategy
{
	eSimpleForth = 0,
	//eForth = 1,
	//eBack = 2,
	eBackForth = 3,
	eComplete = 4,
	eWindow = 5,
	eStep = 6,
	eSector = 7,
	ePositionBased = 8,
	//eManualSelection = 9,
	ePlaneSelect = 10,
	eInvalid = 11,

};

/**	\brief Die abstrakte Klasse CRegistrationStrategy definiert eine
 *	einheitliche Schnittstelle für Strategien zur Auswahl von Bildpaaren bei
 *	der Registrierung von Bildserien.
 *
 *	Die Klasse selbst kann nicht instanziiert werden. Alle abgeleiteten
 *	Klassen müssen folgende Methoden implementieren:
 *	- Initialize() zur Initialisierung des Objekts
 *	- EstimateProgress() zum Abfragen des Fortschritts
 *	- GetNextPair() zum Abfragen des nächsten zu registrierenden Bildpaars
 *	- SetSuccess() für die Rückmeldung des Registrierergebnisses eines
 *		Bildpaars
 *	- IsFinished() zum Abfragen des Aktivitätsstatus
 *	- Finalize() zur Finalisierung des Objekts
 *
 *	Zum Zweck der effizienten Bildserienregistrierung sollten abgeleitete
 *	Klassen die Parallelverarbeitung durch OMP bestmöglich unterstützen.
 *	Insbesondere sind dazu zwei Dinge zu beachten:
 *	1. Die Methode GetNextPair() \e sollte so oft wie möglich ein gültiges,
 *		noch nicht registriertes Bildpaar liefern und nur dann \c false
 *		zurückgeben, wenn zwingend noch auf laufende Registrierergebnisse
 *		gewartet werden muss.
 *	2. In den Methoden EstimateProgress(), GetNextPair(), SetSuccess() und
 *		IsFinished() \e müssen alle Zugriffe (lesend und schreibend) auf
 *		Membervariablen durch
 *		<tt>#pragma omp critical (registration_strategy)</tt> geschützt
 *		werden.
 *
 *	\see CStrategyBackForth
 *	\see CStrategyComplete
 *	\see CStrategySector
 *	\see CStrategyStep
 *	\see CStrategyWindow
 *
 *	\author Stephan Allgeier
 */
class CRegistrationStrategy
{
public:

	CRegistrationStrategy();
	virtual ~CRegistrationStrategy();

	static std::map<ERegistrationStrategy, CString> ERegistrationStrategyMap;
	static CString GetRegistrationStrategyName(ERegistrationStrategy eRegistrationStrategy);
	static ERegistrationStrategy GetRegistrationStrategyID(CString sRegistrationStrategy);

	virtual void SetParameters(const CRegistrationStrategyParameters&) = 0;
	virtual void Initialize(size_t nImages);
	virtual void InitializeAdditionalParameters(CImageRegistrationData);
	virtual bool GetNextPair(size_t& nImg1, size_t& nImg2);
	virtual void SetSuccess(size_t nImg1, size_t nImg2, const CRegistrationResult& correlationResult);
	virtual bool EstimateProgress(size_t& nPairsDone, size_t& nPairsFinal);
	virtual bool IsFinished();
	virtual void Finalize();

	void GetRegistrationImageGroups(std::list<std::list<size_t>>& listRegImgGroups) const;
	void AddRegistrationVerificator(std::shared_ptr<CImagePairVerificator> Verificator);
	bool VerifyImagePair(size_t nImg1, size_t nImg2);

protected:

	/**	\brief Liefert eine initiale Schätzung der Anzahl durchzuführender
	 *	Bildregistrierungen.
	 *
	 *	\remark Diese Methode muss in abgeleiteten Klassen überschrieben
	 *		werden. Zum Abfragen der Größe der Bildserie kann direkt auf
	 *		\a m_nImageCount zugegriffen werden.
	 *
	 *	\return Eine initiale Schätzung der Anzahl durchzuführender
	 *		Bildregistrierungen.
	 *
	 *	\author Stephan Allgeier
	 */
	virtual size_t InitialWorkloadEstimation() const = 0;

	/**	\brief Berechnet die Bildindizes des nächsten zu registrierenden
	 *	Bildpaars.
	 *
	 *	Der Rückgabewert gibt an, ob die Indizes des nächsten zu
	 *	registrierenden Bildpaars ermittelt werden konnten. Ist das der Fall,
	 *	müssen diese in \a m_aNextPair abgelegt und \c true zurückgegeben
	 *	werden. Kann kein gültiges Bildpaar bestimmt werden (z.B. weil noch
	 *	auf die Ergebnisse bereits beauftragter Bildpaare gewartet werden
	 *	muss), dann darf \a m_aNextPair nicht verändert werden und es muss
	 *	\c false zurückgegeben werden.
	 *
	 *	Wird bei der Berechnung des nächsten Bildpaars festgestellt, dass
	 *	bereits sämtliche zu berücksichtigende Bildpaare beauftragt wurden,
	 *	so muss \a m_bFinished gesetzt werden (und die Methode muss \c false
	 *	zurückgeben).
	 *
	 *	\remark Diese Methode muss in abgeleiteten Klassen überschrieben
	 *		werden. Zum Abfragen des letzten beauftragten Bildpaars kann
	 *		direkt auf \a m_aNextPair zugegriffen werden.
	 *	\remark Bei der Implementierung der Methode müssen Zugriffe auf
	 *		Membervariablen von CRegistrationStrategy \e nicht vor
	 *		konkurrierenden Zugriffen geschützt werden, dies wird bereits
	 *		beim Aufruf in GetNextPair() berücksichtigt.
	 *
	 *	\return Gibt an, ob die Bildindizes des nächsten Bildpaars bestimmt
	 *		werden konnten.
	 *
	 *	\author Stephan Allgeier
	 */
	virtual bool PrepareNextPair() = 0;

protected:

	/**	\brief Die Länge der Bildserie.*/
	size_t m_nImageCount;

	/**	\brief Das nächste zu registrierende Bildpaar.*/
	size_t m_aNextPair[2];

	/**	\brief Die Gesamtzahl aller durchzuführenden Bildregistrierungen.*/
	std::atomic_size_t m_nPairsFinal = 1;

	/**	\brief Die Anzahl bisher vergebener Bildpaare.*/
	std::atomic_size_t m_nPairsDealt = 0;

	/**	\brief Die Anzahl bisher registrierter Bildpaare.*/
	std::atomic_size_t m_nPairsDone = 0;

	/**	\brief Der Aktivitätsstatus.*/
	std::atomic_bool m_bFinished;

	/**	\brief Verwaltet die Bildgruppen.*/
	std::list<std::list<size_t>> m_listRegImgGroups;

	/**	\brief Bietet schnellen Zugriff auf die Bildgruppen über den Bildindex.*/
	std::vector<std::list<std::list<size_t>>::iterator> m_arrayRegImgGroups;

	/**	\brief Bietet eine bessere Kontrolle des Multithreadings als omp critical sections*/
	std::mutex m_registrationStartMutex;
	std::mutex m_registrationEndMutex;

	std::vector<std::shared_ptr<CImagePairVerificator>> m_PairVerificators;
};

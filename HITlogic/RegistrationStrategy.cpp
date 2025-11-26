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
#include "RegistrationStrategy.h"



/**	\brief Standard-Konstruktor
 */
CRegistrationStrategy::CRegistrationStrategy()
	:m_nImageCount(0),
	m_bFinished(false)
{
}

/**	\brief Standard-Destruktor
 */
CRegistrationStrategy::~CRegistrationStrategy()
{
	m_arrayRegImgGroups.clear();
	m_listRegImgGroups.clear();
}

/**	\brief Initialisiert das Objekt.
 *
 *	Die Bildgruppen werden derart erstellt, dass jedes Bild seine eigene, nur
 *	ein Bild große, Gruppe bildet.
 *
 *	\remark Diese Methode kann in abgeleiteten Klassen vollständig
 *		überschrieben oder um eigenen Code ergänzt werden. Im letzteren Fall
 *		sollte die überschriebene Methode die Implementierung der Basisklasse
 *		(CRegistrationStrategy) \e vor dem eigenen Code aufrufen, sofern in
 *		der überschriebenen Methode Membervariablen von CRegistrationStrategy
 *		verändert werden.
 *
 *	\warning Diese Methode ist nicht Thread-sicher. Sie wird in der Regel
 *		aufgerufen, bevor die (potienziell parallele) Registrierung der
 *		Bildserie beginnt.
 *
 *	\param[in] nImages Die Länge der Bildserie.
 *
 *	\author Stephan Allgeier
 */
void CRegistrationStrategy::Initialize(size_t nImages)
{
	assert(nImages > 0);

	m_bFinished = false;

	// m_listRegImgGroups contains the image index lists, m_arrayRegImgGroups
	// points to the corresponding image index list for each image index
	assert(m_listRegImgGroups.empty());
	m_arrayRegImgGroups.resize(nImages);

	// Initialise the list for registered image groups with
	// single-image-index-lists for each image
	for (size_t i = 0; i < nImages; i++)
	{
		m_listRegImgGroups.push_back(std::list<size_t>());
		m_listRegImgGroups.back().push_back(i);
		m_arrayRegImgGroups[i] = std::prev(m_listRegImgGroups.end());
	}

	m_nImageCount = nImages;

	m_aNextPair[0] = 0;
	m_aNextPair[1] = 0;

	m_nPairsDealt = 0;
	m_nPairsDone = 0;
	m_nPairsFinal = InitialWorkloadEstimation();

	for (auto Verificator : m_PairVerificators)
		Verificator->Initialize(nImages);
}

//overwrite if additional Parameters needed
void CRegistrationStrategy::InitializeAdditionalParameters(CImageRegistrationData)
{
}


/**	\brief Liefert das nächste zu registrierende Bildpaar.
 *
 *	Der Rückgabewert muss abgefragt werden. Bei \c false dürfen die Werte der
 *	Ausgabeparameter \a nImg1 und \a nImg2 nicht verwendet werden.
 *
 *	\remark Diese Methode kann in abgeleiteten Klassen vollständig
 *		überschrieben oder um eigenen Code ergänzt werden. Im letzteren Fall
 *		sollte die überschriebene Methode die Implementierung der Basisklasse
 *		(CRegistrationStrategy) aufrufen.
 *	\remark Diese Methode ist Thread-sicher.
 *
 *	\param[out] nImg1 Der Index des ersten Bilds.
 *	\param[out] nImg2 Der Index des zweiten Bilds.
 *
 *	\return Gibt an, ob die Ausgabeparameter \a nImg1 und \a nImg2 mit
 *		gültigen Werten belegt wurden und nach der Rückkehr aus der Methode
 *		verwendet werden dürfen.
 *
 *	\author Stephan Allgeier
 */
bool CRegistrationStrategy::GetNextPair(size_t& nImg1, size_t& nImg2)
{
	std::lock_guard<std::mutex> lock(m_registrationStartMutex);		// replaces omp critical section

	if (m_bFinished || !PrepareNextPair())
	{
		return false;
	}

	nImg1 = m_aNextPair[0];
	nImg2 = m_aNextPair[1];
	++m_nPairsDealt;

	assert((0 <= m_aNextPair[0]) && (m_aNextPair[0] < m_aNextPair[1]) && (m_aNextPair[1] < m_nImageCount));


	return true;
}

/**	\brief Übergibt dem Objekt das Registrierergebnis eines registrierten
 *	Bildpaars.
 *
 *	\remark Diese Methode kann in abgeleiteten Klassen vollständig
 *		überschrieben oder um eigenen Code ergänzt werden. Im letzteren Fall
 *		sollte die überschriebene Methode die Implementierung der Basisklasse
 *		(CRegistrationStrategy) aufrufen.
 *	\remark Diese Methode ist Thread-sicher.
 *
 *	\param[in] nImg1 Der Index des ersten Bilds.
 *	\param[in] nImg2 Der Index des zweiten Bilds.
 *	\param[in] correlationResult Das Registrierergebnis.
 *
 *	\author Stephan Allgeier & Lorenzo Toso
 */
void CRegistrationStrategy::SetSuccess(size_t nImg1, size_t nImg2, const CRegistrationResult& correlationResult)
{
	std::lock_guard<std::mutex> lock(m_registrationEndMutex);		// replaces omp critical section

	++m_nPairsDone;

	assert(correlationResult.RigidRegistrationResult.GetReferenceImageIndex() == nImg1);
	assert(correlationResult.RigidRegistrationResult.GetTemplateImageIndex() == nImg2);

	bool bSuccess = (correlationResult.RigidRegistrationResult.GetValidity() > 0);
	if (bSuccess && (m_arrayRegImgGroups[nImg1] != m_arrayRegImgGroups[nImg2]))
	{
		// Let nImg1 refer to the image group with the smallest index of the
		// two groups; as both index lists are sorted, the smallest element
		// of each group is the first one
		if (m_arrayRegImgGroups[nImg1]->front() > m_arrayRegImgGroups[nImg2]->front())
		{
			std::swap(nImg1, nImg2);
		}

		// Replace the iterators for all elements of the second group with
		// iterators pointing to the first group (skip nImg2 itself for the
		// the moment; it would probably mess up the for loop, and we need it
		// later for removing the then-empty group)
		for (auto index : *m_arrayRegImgGroups[nImg2])
		{
			if (index != nImg2)
			{
				m_arrayRegImgGroups[index] = m_arrayRegImgGroups[nImg1];
			}
		}

		// Combine both groups by sorting all indexes of the second group
		// into the first group; the second group is empty afterwards
		m_arrayRegImgGroups[nImg1]->merge(*m_arrayRegImgGroups[nImg2]);

		// Remove the now-empty second list ...
		m_listRegImgGroups.erase(m_arrayRegImgGroups[nImg2]);

		// ... and finally replace the iterator for element nImg2
		m_arrayRegImgGroups[nImg2] = m_arrayRegImgGroups[nImg1];
	}

	for (auto Verificator : m_PairVerificators)
		Verificator->SetSuccess(nImg1, nImg2, correlationResult);
}

/**	\brief Liefert den bisherigen Fortschritt.
 *
 *	\remark Diese Methode kann in abgeleiteten Klassen vollständig
 *		überschrieben oder um eigenen Code ergänzt werden. Im letzteren Fall
 *		sollte die überschriebene Methode die Implementierung der Basisklasse
 *		(CRegistrationStrategy) aufrufen.
 *	\remark Diese Methode ist Thread-sicher.
 *
 *	\param[out] nPairsDone Die Anzahl der bereits registrierten Bildpaare.
 *	\param[out] nPairsFinal Die Anzahl der insgesamt zu registrierenden
 *		Bildpaare.
 *
 *	\return \c true
 *
 *	\author Stephan Allgeier
 */
bool CRegistrationStrategy::EstimateProgress(size_t& nPairsDone, size_t& nPairsFinal)
{
	nPairsDone = m_nPairsDone;
	nPairsFinal = m_nPairsFinal;
	return true;
}

/**	\brief Liefert den Aktivitätsstatus des Objekts.
 *
 *	\remark Diese Methode kann in abgeleiteten Klassen vollständig
 *		überschrieben oder um eigenen Code ergänzt werden. Im letzteren Fall
 *		sollte die überschriebene Methode die Implementierung der Basisklasse
 *		(CRegistrationStrategy) aufrufen.
 *	\remark Diese Methode ist Thread-sicher.
 *
 *	\return \c true, wenn die Registrierung der Bildserie beendet ist.
 *	\return \c false, wenn weitere zu registrierende Bildpaare vorhanden
 *		sind.
 *
 *	\author Stephan Allgeier
 */
bool CRegistrationStrategy::IsFinished()
{
	return  m_bFinished && (m_nPairsDone == m_nPairsDealt);
}

/**	\brief Finalisiert das Objekt.
 *
 *	\remark Diese Methode kann in abgeleiteten Klassen vollständig
 *		überschrieben werden, wenn der Objektzustand noch aufgeräumt werden
 *		muss, bevor die Ergebnisse abgefragt werden.
 *
 *	\warning Diese Methode ist nicht Thread-sicher. Sie wird in der Regel
 *		erst aufgerufen, wenn die (potienziell parallele) Registrierung der
 *		Bildserie komplett abgeschlossen ist.
 *
 *	\author Stephan Allgeier
 */
void CRegistrationStrategy::Finalize()
{
	for (auto Verificator : m_PairVerificators)
		Verificator->Finalize();
	// This method is intentionally empty. It is called from the framework,
	// so don't delete it. It is virtual, so derived classes can provide
	// their own implementation.
}

/**	\brief Liefert die während der Registrierung gebildeten Bildgruppen.
 *
 *	Eine Bildgruppe besteht jeweils aus allen Bildern der Bildserie, die
 *	direkt oder über eine Folge von Bildregistrierungen zueinander in Bezug
 *	gesetzt werden können.
 *
 *	\warning Diese Methode ist nicht Thread-sicher. Sie wird in der Regel
 *		erst aufgerufen, wenn die (potienziell parallele) Registrierung der
 *		Bildserie komplett abgeschlossen ist.
 *
 *	\param[out] listRegImgGroups Enthält nach der Rückkehr aus der Methode
 *		die Liste der Bildgruppen. Sowohl die Liste der Bildgruppen, als auch
 *		jede der Bildgruppen sind nach aufsteigendem Bildindex sortiert.
 *
 *	\author Stephan Allgeier
 */
void CRegistrationStrategy::GetRegistrationImageGroups(std::list<std::list<size_t>>& listRegImgGroups) const
{
	assert(listRegImgGroups.empty());

	listRegImgGroups = m_listRegImgGroups;
}

void CRegistrationStrategy::AddRegistrationVerificator(std::shared_ptr<CImagePairVerificator> Verificator)
{
	m_PairVerificators.push_back(Verificator);
}

bool CRegistrationStrategy::VerifyImagePair(size_t nImg1, size_t nImg2)
{
	for (auto verificator : m_PairVerificators)
	{
		if (!verificator->ValidateImagePair(nImg1, nImg2))
			return false;
	}
	return true;
}

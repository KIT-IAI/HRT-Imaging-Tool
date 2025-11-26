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



#include "StdAfx.h"

#include "StrategyStep.h"


/**	\brief Standard-Konstruktor
 */
CStrategyStep::CStrategyStep()
	: m_nWindow(3),
	m_nStep(10),
	m_nMaxDistance(1000),
	m_nDistance(1)
{
}

/**	\brief Standard-Destruktor
 */
CStrategyStep::~CStrategyStep()
{
}

void CStrategyStep::SetParameters(const CRegistrationStrategyParameters& parameters)
{
	m_nWindow = parameters.nWindowSize;
	m_nStep = parameters.nStepSize;
	m_nMaxDistance = parameters.nMaxDistance;

	if (m_nWindow == 0)
		throw std::exception("Window size cannot be zero.");
}


/**	\brief Initialisiert das Objekt.
 *
 *	Die Bildgruppen werden derart erstellt, dass jedes Bild seine eigene, nur
 *	ein Bild große, Gruppe bildet.
 *
 *	\param[in] nImages Die Länge der Bildserie.
 *
 *	\author Stephan Allgeier
 */
void CStrategyStep::Initialize(size_t nImages)
{
	if (m_nMaxDistance == 0 || m_nMaxDistance > nImages)
		m_nMaxDistance = nImages;

	CRegistrationStrategy::Initialize(nImages);


	m_nDistance = 1;
}


/**	\brief Liefert eine initiale Schätzung der Anzahl durchzuführender
 *	Bildregistrierungen.
 *
 *	\return Eine initiale Schätzung der Anzahl durchzuführender
 *		Bildregistrierungen.
 *
 *	\author Stephan Allgeier
 */
size_t CStrategyStep::InitialWorkloadEstimation() const
{
	INT_PTR nImages = static_cast<INT_PTR>(m_nImageCount);
	INT_PTR nStepsPerImage = (m_nMaxDistance - m_nWindow) / m_nStep;
	INT_PTR nMaxUsedDistance = nStepsPerImage * m_nStep + m_nWindow;
	INT_PTR nStepRegs = (nImages - nMaxUsedDistance) * nStepsPerImage + static_cast<INT_PTR>(m_nStep) * ((nStepsPerImage * (nStepsPerImage - 1)) / 2);
	INT_PTR nWindowRegs = (nImages - m_nWindow) * m_nWindow + ((m_nWindow * (m_nWindow - 1)) / 2);
	return nWindowRegs + nStepRegs; // The conversion can be implicit because the numbers will always be positive
}

/**	\brief Berechnet die Bildindizes des nächsten zu registrierenden
 *	Bildpaars.
 *
 *	Der Rückgabewert gibt an, ob die Indizes des nächsten zu registrierenden
 *	Bildpaars ermittelt werden konnten. Bei \c true wurden diese in
 *	\a m_aNextPair abgelegt. Bei \c false bleibt \a m_aNextPair unverändert.
 *
 *	Wird bei der Berechnung des nächsten Bildpaars festgestellt, dass bereits
 *	sämtliche zu berücksichtigende Bildpaare beauftragt wurden, so wird
 *	\a m_bFinished gesetzt und \c false zurückgegeben.
 *
 *	\return Gibt an, ob die Bildindizes des nächsten Bildpaars bestimmt
 *		werden konnten.
 *
 *	\author Stephan Allgeier
 */
bool CStrategyStep::PrepareNextPair()
{
	auto nImg1 = m_aNextPair[0];
	auto nImg2 = m_aNextPair[1];

	if (nImg1 == 0 && nImg2 == 0)
	{
		// Initial value for m_aNextPair is (0,0)
		nImg1 = 0;
		nImg2 = 1;
	}
	else
	{
		nImg1++;
		nImg2++;
	}

	assert(nImg2 - nImg1 == m_nDistance);
	assert(m_nDistance <= m_nMaxDistance);

	if (nImg2 >= m_nImageCount)
	{
		if (m_nDistance < m_nWindow)
		{
			m_nDistance++;
		}
		else
		{
			m_nDistance += m_nStep;
		}

		nImg1 = 0;
		nImg2 = nImg1 + m_nDistance;
	}

	if ((nImg2 >= m_nImageCount) || (m_nDistance > m_nMaxDistance))
	{
		m_bFinished = true;
		return false;
	}

	m_aNextPair[0] = nImg1;
	m_aNextPair[1] = nImg2;
	return true;
}
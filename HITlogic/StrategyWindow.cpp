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

#include "StrategyWindow.h"


/**	\brief Standard-Konstruktor
 */
CStrategyWindow::CStrategyWindow()
{
	m_nWindow = 3;
}

/**	\brief Standard-Destruktor
 */
CStrategyWindow::~CStrategyWindow()
{
}

void CStrategyWindow::SetParameters(const CRegistrationStrategyParameters& parameters)
{
	assert(parameters.nWindowSize > 0);
	m_nWindow = parameters.nWindowSize;
}

/**	\brief Liefert eine initiale Schätzung der Anzahl durchzuführender
 *	Bildregistrierungen.
 *
 *	\return Eine initiale Schätzung der Anzahl durchzuführender
 *		Bildregistrierungen.
 *
 *	\author Stephan Allgeier
 */
size_t CStrategyWindow::InitialWorkloadEstimation() const
{
	return m_nImageCount * m_nWindow - (m_nWindow * (m_nWindow + 1)) / 2;
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
bool CStrategyWindow::PrepareNextPair()
{
	size_t nImg1 = m_aNextPair[0];
	size_t nImg2 = m_aNextPair[1];

	if ((nImg1 == 0) && (nImg2 == 0))
	{
		// Initial value for m_aNextPair is (0,0)
		nImg1 = 0;
		nImg2 = 1;
	}
	else
	{
		nImg2++;
	}

	if ((nImg2 >= m_nImageCount) || (nImg2 > nImg1 + m_nWindow))
	{
		nImg1++;
		nImg2 = nImg1 + 1;
	}

	if (nImg2 < m_nImageCount)
	{
		m_aNextPair[0] = nImg1;
		m_aNextPair[1] = nImg2;
		return true;
	}
	else
	{
		m_bFinished = true;
		return false;
	}
}

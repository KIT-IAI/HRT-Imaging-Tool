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
#include "StrategySector.h"



/**	\brief Standard-Konstruktor
 */
CStrategySector::CStrategySector()
	: m_nWindow(3)
	, m_nSectorSize(60)
	, m_nMaxDistance(1000)
	, m_nImageSizeX(0)
	, m_nImageSizeY(0)
	, m_eState(eWindowRegs)
{

}

/**	\brief Standard-Destruktor
 */
CStrategySector::~CStrategySector()
{
}

void CStrategySector::SetParameters(const CRegistrationStrategyParameters& parameters)
{
	assert(parameters.nWindowSize > 0);
	assert(parameters.nSectorSize > 0);
	assert(parameters.nMaxDistance >= 0);
	assert(parameters.ImageSize.x > 0);
	assert(parameters.ImageSize.y > 0);

	m_nWindow = parameters.nWindowSize;
	m_nSectorSize = parameters.nSectorSize;
	m_nMaxDistance = parameters.nMaxDistance;
	m_nImageSizeX = parameters.ImageSize.x;
	m_nImageSizeY = parameters.ImageSize.y;
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
void CStrategySector::Initialize(size_t nImages)
{
	CRegistrationStrategy::Initialize(nImages);

	if (m_nMaxDistance == 0)
		m_nMaxDistance = nImages;

	m_eState = eWindowRegs;

	m_matXOffsets.AllocMatrix(nImages, nImages);
	m_matYOffsets.AllocMatrix(nImages, nImages);

	m_matXOffsets.Fill(0.0);
	m_matYOffsets.Fill(0.0);

	m_aNextSector[0] = 0;
	m_aNextSector[1] = 0;
}

/**	\brief Übergibt dem Objekt das Registrierergebnis eines registrierten
 *	Bildpaars.
 *
 *	\param[in] nImg1 Der Index des ersten Bilds.
 *	\param[in] nImg2 Der Index des zweiten Bilds.
 *	\param[in] reg Das Registrierergebnis.
 *
 *	\author Stephan Allgeier
 */
void CStrategySector::SetSuccess(size_t nImg1, size_t nImg2, const CRegistrationResult& correlationResult)
{
	CRegistrationStrategy::SetSuccess(nImg1, nImg2, correlationResult);

	std::lock_guard<std::mutex> lock(m_criticalMutex);		// replaces omp critical section
	{
		assert(correlationResult.RigidRegistrationResult.GetReferenceImageIndex() == nImg1);
		assert(correlationResult.RigidRegistrationResult.GetTemplateImageIndex() == nImg2);

		bool bSuccess = (correlationResult.RigidRegistrationResult.GetValidity() > 0);
		if (bSuccess)
		{
			m_matXOffsets[nImg1][nImg2] = correlationResult.RigidRegistrationResult.GetX();
			m_matYOffsets[nImg1][nImg2] = correlationResult.RigidRegistrationResult.GetY();
			m_matXOffsets[nImg2][nImg1] = -m_matXOffsets[nImg1][nImg2];
			m_matYOffsets[nImg2][nImg1] = -m_matYOffsets[nImg1][nImg2];
		}
	}
}

/**	\brief Liefert eine initiale Schätzung der Anzahl durchzuführender
 *	Bildregistrierungen.
 *
 *	\return Eine initiale Schätzung der Anzahl durchzuführender
 *		Bildregistrierungen.
 *
 *	\author Stephan Allgeier
 */
size_t CStrategySector::InitialWorkloadEstimation() const
{
	size_t nSectorsPerImage = static_cast<size_t>(ceil(static_cast<double>(m_nMaxDistance - m_nWindow) / static_cast<double>(m_nSectorSize)));
	size_t nEffectiveDistance = (nSectorsPerImage - 1) * m_nSectorSize + 1 + m_nWindow;
	size_t nSectorRegs = (m_nImageCount - nEffectiveDistance) * nSectorsPerImage + m_nSectorSize * ((nSectorsPerImage * (nSectorsPerImage - 1)) / 2);
	size_t nWindowRegs = (m_nImageCount - m_nWindow) * m_nWindow + ((m_nWindow * (m_nWindow - 1)) / 2);
	return nWindowRegs + nSectorRegs;
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
bool CStrategySector::PrepareNextPair()
{
	size_t nImg1 = m_aNextPair[0];
	size_t nImg2 = m_aNextPair[1];

	if (m_eState == eWindowRegs)
	{
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

		if ((nImg2 >= m_nImageCount) || (nImg2 > nImg1 + m_nWindow) || (nImg2 > nImg1 + m_nMaxDistance))
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
			m_eState = eWait;
		}
	}

	assert((m_eState == eWait) || (m_eState == eSectorRegs));

	if (m_eState == eWait)
	{
		if (m_nPairsDone != m_nPairsDealt)
		{
			return false;
		}
		else
		{
			InitializeSectors();
			m_eState = eSectorRegs;
		}
	}

	assert(m_eState == eSectorRegs);

	if ((m_aNextSector[0] < 0) && (m_aNextSector[1] < 0))
	{
		// Initial value for m_aNextSector is (-1,-1)
		m_aNextSector[0] = 0;
		m_aNextSector[1] = m_nWindow + 1;
	}
	else
	{
		m_aNextSector[1] += m_nSectorSize;
	}

	while (true)
	{
		if ((m_aNextSector[1] >= m_nImageCount) || (m_aNextSector[1] > m_aNextSector[0] + m_nMaxDistance))
		{
			m_aNextSector[0]++;
			m_aNextSector[1] = m_aNextSector[0] + m_nWindow + 1;
		}

		if ((m_aNextSector[1] >= m_nImageCount) || (m_aNextSector[1] > m_aNextSector[0] + m_nMaxDistance))
		{
			m_bFinished = true;
			return false;
		}
		else if (GetPairFromSector(nImg1, nImg2))
		{
			m_aNextPair[0] = nImg1;
			m_aNextPair[1] = nImg2;
			return true;
		}
		else
		{
			m_aNextSector[1] += m_nSectorSize;
			m_nPairsFinal--;
			continue;
		}
	}
}

/**	\brief Schätzt die Distanzen aller Bildpaare auf der Grundlage der
 *	Registrierergebnisse aus der initialen Fensterphase der Registrierung.
 *
 *	Die Schätzung geschieht in zwei Schritten. Im ersten Schritt wird
 *	versucht, die fehlenden Distanzwerte unmittelbar benachbarter Bildpaare
 *	\f$(i,i+1)\f$ als Summe von zwei oder drei Registrierungen aus der nahen
 *	Umgebung zu ergänzen. Im zweiten Schritt werden die fehlenden
 *	Distanzwerte aller anderen Bildpaare \f$(i,j)\f$ geschätzt, wobei aber
 *	nur Summen aus genau zwei bereits vorhandenen Distanzschätzungen
 *	\f$(i,k)\f$ und \f$(k,j)\f$ mit \f$i<k<j\f$ in Betracht gezogen werden.
 *
 *	Die geschätzten Distanzen werden in den Distanzmatrizen \a m_matXOffsets
 *	und m_matYOffsets gespeichert.
 *
 *	\author Robert Illner
 *	\author Stephan Allgeier
 */
void CStrategySector::InitializeSectors()
{
	std::lock_guard<std::mutex> lock(m_criticalMutex);		// replaces omp critical section
	{
		// try to estimate missing registration results for image pairs (i,i+1)
		for (size_t i = 0; i < m_nImageCount - 1; i++)
		{
			if (m_matXOffsets[i][i + 1] == 0.0)
			{
				// try image (i-1) as intermediate node
				if ((i > 0) && (m_matXOffsets[i][i - 1] != 0.0) && (m_matXOffsets[i - 1][i + 1] != 0.0))
				{
					m_matXOffsets[i][i + 1] = m_matXOffsets[i][i - 1] + m_matXOffsets[i - 1][i + 1];
					m_matYOffsets[i][i + 1] = m_matYOffsets[i][i - 1] + m_matYOffsets[i - 1][i + 1];
				}
				// try image (i+2) as intermediate node
				else if ((i < m_nImageCount - 2) && (m_matXOffsets[i][i + 2] != 0.0) && (m_matXOffsets[i + 2][i + 1] != 0.0))
				{
					m_matXOffsets[i][i + 1] = m_matXOffsets[i][i + 2] + m_matXOffsets[i + 2][i + 1];
					m_matYOffsets[i][i + 1] = m_matYOffsets[i][i + 2] + m_matYOffsets[i + 2][i + 1];
				}
				// try image (i-2) as intermediate node
				else if ((i > 1) && (m_matXOffsets[i][i - 2] != 0.0) && (m_matXOffsets[i - 2][i + 1] != 0.0))
				{
					m_matXOffsets[i][i + 1] = m_matXOffsets[i][i - 2] + m_matXOffsets[i - 2][i + 1];
					m_matYOffsets[i][i + 1] = m_matYOffsets[i][i - 2] + m_matYOffsets[i - 2][i + 1];
				}
				// try image (i+3) as intermediate node
				else if ((i < m_nImageCount - 3) && (m_matXOffsets[i][i + 3] != 0.0) && (m_matXOffsets[i + 3][i + 1] != 0.0))
				{
					m_matXOffsets[i][i + 1] = m_matXOffsets[i][i + 3] + m_matXOffsets[i + 3][i + 1];
					m_matYOffsets[i][i + 1] = m_matYOffsets[i][i + 3] + m_matYOffsets[i + 3][i + 1];
				}
				// try images (i-2) and (i-1) as intermediate nodes
				else if ((i > 1) && (m_matXOffsets[i][i - 2] != 0.0) && (m_matXOffsets[i - 2][i - 1] != 0.0) && (m_matXOffsets[i - 1][i + 1] != 0.0))
				{
					m_matXOffsets[i][i + 1] = m_matXOffsets[i][i - 2] + m_matXOffsets[i - 2][i - 1] + m_matXOffsets[i - 1][i + 1];
					m_matYOffsets[i][i + 1] = m_matYOffsets[i][i - 2] + m_matYOffsets[i - 2][i - 1] + m_matYOffsets[i - 1][i + 1];
				}
				// try images (i-1) and (i-2) as intermediate nodes
				else if ((i > 1) && (m_matXOffsets[i][i - 1] != 0.0) && (m_matXOffsets[i - 1][i - 2] != 0.0) && (m_matXOffsets[i - 2][i + 1] != 0.0))
				{
					m_matXOffsets[i][i + 1] = m_matXOffsets[i][i - 1] + m_matXOffsets[i - 1][i - 2] + m_matXOffsets[i - 2][i + 1];
					m_matYOffsets[i][i + 1] = m_matYOffsets[i][i - 1] + m_matYOffsets[i - 1][i - 2] + m_matYOffsets[i - 2][i + 1];
				}
				// try images (i-1) and (i+2) as intermediate nodes
				else if ((i > 0) && (i < m_nImageCount - 2) && (m_matXOffsets[i][i - 1] != 0.0) && (m_matXOffsets[i - 1][i + 2] != 0.0) && (m_matXOffsets[i + 2][i + 1] != 0.0))
				{
					m_matXOffsets[i][i + 1] = m_matXOffsets[i][i - 1] + m_matXOffsets[i - 1][i + 2] + m_matXOffsets[i + 2][i + 1];
					m_matYOffsets[i][i + 1] = m_matYOffsets[i][i - 1] + m_matYOffsets[i - 1][i + 2] + m_matYOffsets[i + 2][i + 1];
				}
				// try images (i+2) and (i-1) as intermediate nodes
				else if ((i > 0) && (i < m_nImageCount - 2) && (m_matXOffsets[i][i + 2] != 0.0) && (m_matXOffsets[i + 2][i - 1] != 0.0) && (m_matXOffsets[i - 1][i + 1] != 0.0))
				{
					m_matXOffsets[i][i + 1] = m_matXOffsets[i][i + 2] + m_matXOffsets[i + 2][i - 1] + m_matXOffsets[i - 1][i + 1];
					m_matYOffsets[i][i + 1] = m_matYOffsets[i][i + 2] + m_matYOffsets[i + 2][i - 1] + m_matYOffsets[i - 1][i + 1];
				}
				// try images (i+2) and (i+3) as intermediate nodes
				else if ((i < m_nImageCount - 3) && (m_matXOffsets[i][i + 2] != 0.0) && (m_matXOffsets[i + 2][i + 3] != 0.0) && (m_matXOffsets[i + 3][i + 1] != 0.0))
				{
					m_matXOffsets[i][i + 1] = m_matXOffsets[i][i + 2] + m_matXOffsets[i + 2][i + 3] + m_matXOffsets[i + 3][i + 1];
					m_matYOffsets[i][i + 1] = m_matYOffsets[i][i + 2] + m_matYOffsets[i + 2][i + 3] + m_matYOffsets[i + 3][i + 1];
				}
				// try images (i+3) and (i+2) as intermediate nodes
				else if ((i < m_nImageCount - 3) && (m_matXOffsets[i][i + 3] != 0.0) && (m_matXOffsets[i + 3][i + 2] != 0.0) && (m_matXOffsets[i + 2][i + 1] != 0.0))
				{
					m_matXOffsets[i][i + 1] = m_matXOffsets[i][i + 3] + m_matXOffsets[i + 3][i + 2] + m_matXOffsets[i + 2][i + 1];
					m_matYOffsets[i][i + 1] = m_matYOffsets[i][i + 3] + m_matYOffsets[i + 3][i + 2] + m_matYOffsets[i + 2][i + 1];
				}

				if (m_matXOffsets[i][i + 1] != 0.0)
				{
					m_matXOffsets[i + 1][i] = -m_matXOffsets[i][i + 1];
					m_matYOffsets[i + 1][i] = -m_matYOffsets[i][i + 1];
				}
			}
		}

		// try to estimate offsets for all other image pairs (i,j)
		for (ptrdiff_t i = m_nImageCount - 3; i >= 0; i--)
		{
			for (size_t j = i + 2; j < m_nImageCount; j++)
			{
				// Try to estimate the offset (i,j) as the sum of two offsets
				// (i,k) and (k,j) with i<k<j. Stop immediately as soon as a
				// viable intermediate node k has been found.
				for (size_t k = i + 1; k < j; k++)
				{
					if ((m_matXOffsets[i][j] == 0.0) && (m_matXOffsets[i][k] != 0.0) && (m_matXOffsets[k][j] != 0.0))
					{
						m_matXOffsets[i][j] = m_matXOffsets[i][k] + m_matXOffsets[k][j];
						m_matYOffsets[i][j] = m_matYOffsets[i][k] + m_matYOffsets[k][j];
						m_matXOffsets[j][i] = -m_matXOffsets[i][j];
						m_matYOffsets[j][i] = -m_matYOffsets[i][j];
						break;
					}
				}
			}
		}
	}
}

/**	\brief Liefert das nächste zu registrierende Bildpaar innerhalb eines
 *	Sektors während der Sektorphase der Registrierung.
 *
 *	Das erste Bild des Bildpaars ist durch \a m_aNextSector[0] gegeben. Das
 *	zweite Bild liegt im Sektor, dessen Beginn durch \a m_aNextSector[1] und
 *	dessen Länge durch \a m_nSectorSize eindeutig bestimmt ist. Aus allen
 *	Bildpaaren des Sektors wird dasjenige mit der minimalen geschätzten
 *	Distanz selektiert.
 *
 *	Der Rückgabewert muss abgefragt werden. Bei \c false dürfen die Werte der
 *	Ausgabeparameter \a nImg1 und \a nImg2 nicht verwendet werden. Dieser
 *	Fall tritt ein, wenn die minimale geschätzte Distanz innerhalb des
 *	Sektors entlang einer Bilddimension größer ist als die entsprechende
 *	Bildgröße, wenn sich also gemäß der Distanzschätzung keines der Bildpaare
 *	innerhalb des Sektors überlappt.
 *
 *	\param[out] nImg1 Der Index des ersten Bilds.
 *	\param[out] nImg2 Der Index des zweiten Bilds.
 *
 *	\return \c true, wenn die Ausgabeparameter mit gültigen Werten belegt
 *		sind.
 *	\return \c false, wenn sich gemäß der Distanzschätzung keines der
 *		Bildpaare innerhalb des Sektors überlappt. Die Werte der
 *		Ausgabeparameter \a nImg1 und \a nImg2 sind dann ungültig und dürfen
 *		nicht verwendet werden.
 *
 *	\author Robert Illner
 *	\author Stephan Allgeier
 */
bool CStrategySector::GetPairFromSector(size_t& nImg1, size_t& nImg2)
{
	bool bReturn;

	std::lock_guard<std::mutex> lock(m_criticalMutex);		// replaces omp critical section
	{
		assert(m_aNextSector[0] >= 0);
		assert(m_aNextSector[0] < m_aNextSector[1]);
		assert(m_aNextSector[1] <= m_aNextSector[0] + m_nMaxDistance);
		assert(m_aNextSector[1] < m_nImageCount);

		double fMinDist = -1.0;
		ptrdiff_t nMinInd = -1;

		size_t i = m_aNextSector[0];
		for (size_t j = m_aNextSector[1]; (j < m_aNextSector[1] + m_nSectorSize) && (j <= m_aNextSector[0] + m_nMaxDistance) && (j < m_nImageCount); j++)
		{
			if (m_matXOffsets[i][j] != 0.0)
			{
				double fDist = m_matXOffsets[i][j] * m_matXOffsets[i][j] + m_matYOffsets[i][j] * m_matYOffsets[i][j];
				if ((nMinInd < 0) || (fDist < fMinDist))
				{
					fMinDist = fDist;
					nMinInd = j;
				}
			}
		}

		if ((nMinInd > 0) && (abs(m_matXOffsets[i][nMinInd]) < m_nImageSizeX) && (abs(m_matYOffsets[i][nMinInd]) < m_nImageSizeY))
		{
			assert(fMinDist > 0);
			nImg1 = i;
			nImg2 = nMinInd;
			bReturn = true;
		}
		else
		{
			bReturn = false;
		}
	}

	return bReturn;
}

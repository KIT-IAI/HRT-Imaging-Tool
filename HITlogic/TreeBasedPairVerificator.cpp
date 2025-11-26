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
#include "TreeBasedPairVerificator.h"


CTreeBasedPairVerificator::CTreeBasedPairVerificator(const CRegistrationVerificationParameters& param)
	: CImagePairVerificator(param),
	m_nImageCount(0)
{
	m_nImageSizeX = param.ImageSize.x;
	m_nImageSizeY = param.ImageSize.y;
}


CTreeBasedPairVerificator::~CTreeBasedPairVerificator()
{
}

bool CTreeBasedPairVerificator::ValidateImagePair(size_t nReferenceIndex, size_t nTemplateIndex)
{
	return (abs(m_matXOffsets[nReferenceIndex][nTemplateIndex]) < m_nImageSizeX) && (abs(m_matYOffsets[nReferenceIndex][nTemplateIndex]) < m_nImageSizeY);
}

void CTreeBasedPairVerificator::SetSuccess(size_t nReferenceIndex, size_t nTemplateIndex, const CRegistrationResult& correlationResult)
{
	if (correlationResult.IsValid() && correlationResult.eClassification >= EClassification::eCorrect)
	{
		UpdateOffsets(nReferenceIndex, nTemplateIndex, correlationResult.RigidRegistrationResult);
	}
}
/**	\brief Schätzt die Bildversätze für alle Bildpaare aus mit \a nImg1 und
*	\a nImg2 bereits verbundenen Bildern.
*
*	Die Versatzschätzung wird nur durchgeführt, wenn der Gütewert der
*	Registrierung \a reg über dem Schwellwert \a m_fCertaintyScoreLevel
*	liegt. Dieser Schwellwert muss hoch genug angesetzt sein, sodass auf
*	keinen Fall falsche Registrierungen zur Versatzschätzung beitragen. Die
*	geschätzen Bildversätze werden in die Matrizen \a m_matXOffsets und
*	\a m_matYOffsets eingetragen.
*
*	\param[in] nImg1 Der Index des ersten Bilds.
*	\param[in] nImg2 Der Index des zweiten Bilds.
*	\param[in] reg Das Registrierergebnis.
*
*	\todo Die Implementierung mit der doppelten Schleife über alle Bilder ist
*		nicht optimal. Besser wäre es, analog zu den durch Registrierungen
*		verbundenen Bildgruppen in \a m_arrayRegImgGroups die durch
*		Versatzschätzung verbundenen Bildgruppen zu verwalten, und dann
*		gezielt nur über die entsprechenden Bildgruppen von \a nImg1 und
*		\a nImg2 zu iterieren. Die Bildgruppen in \a m_arrayRegImgGroups sind
*		dazu nicht geeignet, weil die Distanzschätzung nicht für alle
*		erfolgreichen Registrierungen durchgeführt wird, sondern zusätzlich
*		dem (im Normalfall restriktiveren) Schwellwert
*		\a m_fCertaintyScoreLevel unterliegt.
*
*	\author Stephan Allgeier
*/
void CTreeBasedPairVerificator::UpdateOffsets(size_t nImg1, size_t nImg2, const CRigidRegistrationResult& reg)
{
	std::lock_guard<std::mutex> lock(m_criticalMutex);		// replaces omp critical section
	{
		assert(reg.GetReferenceImageIndex() == nImg1);
		assert(reg.GetTemplateImageIndex() == nImg2);
		assert(reg.GetValidity() > 0);
		assert((m_matXOffsets[nImg1][nImg2] != 0.0) == (m_matYOffsets[nImg1][nImg2] != 0.0));

		if ((m_matXOffsets[nImg1][nImg2] == 0.0) && (m_matYOffsets[nImg1][nImg2] == 0.0))
		{
			// Update the direct connection
			m_matXOffsets[nImg1][nImg2] = reg.GetX();
			m_matYOffsets[nImg1][nImg2] = reg.GetY();
			m_matXOffsets[nImg2][nImg1] = -m_matXOffsets[nImg1][nImg2];
			m_matYOffsets[nImg2][nImg1] = -m_matYOffsets[nImg1][nImg2];

			// Update all linked connections
			// NOTE: This is not a very efficient implementation. At least at
			//       the beginning of the registration process most of the
			//       loop iterations do nothing (and for sequences that
			//       separate into multiple big image groups this remains the
			//       case throughout the registration process).
			for (size_t nIndex1 = 0; nIndex1 < m_nImageCount - 1; nIndex1++)
			{
				if ((m_matXOffsets[nIndex1][nImg1] != 0.0) || (m_matYOffsets[nIndex1][nImg1] != 0.0) || (nIndex1 == nImg1))
				{
					for (size_t nIndex2 = nIndex1 + 1; nIndex2 < m_nImageCount; nIndex2++)
					{
						if ((m_matXOffsets[nImg2][nIndex2] != 0.0) || (m_matYOffsets[nImg2][nIndex2] != 0.0) || (nImg2 == nIndex2))
						{
							m_matXOffsets[nIndex1][nIndex2] = m_matXOffsets[nIndex1][nImg1] + m_matXOffsets[nImg1][nImg2] + m_matXOffsets[nImg2][nIndex2];
							m_matYOffsets[nIndex1][nIndex2] = m_matYOffsets[nIndex1][nImg1] + m_matYOffsets[nImg1][nImg2] + m_matYOffsets[nImg2][nIndex2];
							m_matXOffsets[nIndex2][nIndex1] = -m_matXOffsets[nIndex1][nIndex2];
							m_matYOffsets[nIndex2][nIndex1] = -m_matYOffsets[nIndex1][nIndex2];
						}
					}
				}
			}
		}
	}
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
void CTreeBasedPairVerificator::Initialize(size_t nImages)
{
	m_nImageCount = nImages;

	m_matXOffsets.AllocMatrix(nImages, nImages);
	m_matYOffsets.AllocMatrix(nImages, nImages);

	m_matXOffsets.Fill(0.0);
	m_matYOffsets.Fill(0.0);
}

void CTreeBasedPairVerificator::Finalize()
{
}
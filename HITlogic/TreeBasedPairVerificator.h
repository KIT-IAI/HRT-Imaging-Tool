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
#include "ImagePairVerificator.h"
#include <mutex>

class CTreeBasedPairVerificator :
	public CImagePairVerificator
{
public:
	explicit CTreeBasedPairVerificator(const CRegistrationVerificationParameters& Parameters);
	~CTreeBasedPairVerificator();
	virtual bool ValidateImagePair(size_t nReferenceIndex, size_t nTemplateIndex) override;
	virtual void SetSuccess(size_t nReferenceIndex, size_t nTemplateIndex, const CRegistrationResult& registration_result) override;
	void UpdateOffsets(size_t nImg1, size_t nImg2, const CRigidRegistrationResult& reg);
	virtual void Initialize(size_t nImages) override;
	virtual void Finalize() override;

private:
	/**	\brief Verwaltet die geschätzten horizontalen Distanzen aller Bildpaare.*/
	CDenseMatrix m_matXOffsets;

	/**	\brief Verwaltet die geschätzten vertikalen Distanzen aller Bildpaare.*/
	CDenseMatrix m_matYOffsets;

	/**	\brief Bietet eine bessere Kontrolle des Multithreadings als omp critical sections*/
	std::mutex m_criticalMutex;
	size_t m_nImageCount;
	size_t m_nImageSizeX;
	size_t m_nImageSizeY;
};


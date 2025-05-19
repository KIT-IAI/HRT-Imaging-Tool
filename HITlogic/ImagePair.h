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
#include "StlImage.h"
class CImagePair
{
public:
	CImagePair(StlImage<float>* pReferenceImage, StlImage<float>* pTemplateImage, size_t nReferenceIndex = 0, size_t nTemplateIndex = 1);
	CImagePair(StlImage<float>* pReferenceImage, StlImage<float>* pTemplateImage, StlImage<float>* pReferenceImageSmooth, StlImage<float>* pTemplateImageSmooth, size_t nReferenceIndex = 0, size_t nTemplateIndex = 1);
	CImagePair(StlImage<float>&& pReferenceImage, StlImage<float>&& pTemplateImage);
	CImagePair() = default;
	~CImagePair();

	size_t GetReferenceImageIndex() const;
	size_t GetTemplateImageIndex() const;

	StlImage<float>* GetReferenceImage() const;
	StlImage<float>* GetTemplateImage() const;
	StlImage<float>* GetReferenceImageSmooth() const;
	StlImage<float>* GetTemplateImageSmooth() const;

	static std::pair<StlImageRect, StlImageRect> GetOverlappingRectangles(StlImageSize refImageSize, StlImageSize tempImageSize, StlImagePoint offset);

	StlImageSize GetFFTSize() const;
	void SetUseMinimalFFTSize(bool bUse);
	void SetUseFixedFFTSize(StlImageSize fftSize);
	void SetUseDefaultFFTSize();

private:

	size_t m_nReferenceIndex = 0;
	size_t m_nTemplateIndex = 1;

	StlImage<float>* m_pReferenceImage = nullptr;
	StlImage<float>* m_pTemplateImage = nullptr;

	StlImage<float>* m_pReferenceImageSmooth = nullptr;
	StlImage<float>* m_pTemplateImageSmooth = nullptr;

	// These two are only used if the Move-Constructor is used
	std::shared_ptr<StlImage<float>> m_ReferenceImage = nullptr;
	std::shared_ptr<StlImage<float>> m_TemplateImage = nullptr;

	bool m_bUseMinimalFFTSize = false;
	bool m_bUseDefaultFFTSize = true;
	StlImageSize m_fftSize;
};

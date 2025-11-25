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
#include "ImagePair.h"


CImagePair::CImagePair(StlImage<float>* pReferenceImage, StlImage<float>* pTemplateImage, size_t nReferenceIndex, size_t nTemplateIndex)
	: m_pReferenceImage(pReferenceImage)
	, m_pTemplateImage(pTemplateImage)
	, m_nReferenceIndex(nReferenceIndex)
	, m_nTemplateIndex(nTemplateIndex)
	, m_fftSize()
{
}

CImagePair::CImagePair(StlImage<float>* pReferenceImage, StlImage<float>* pTemplateImage, StlImage<float>* pReferenceImageSmooth, StlImage<float>* pTemplateImageSmooth, size_t nReferenceIndex, size_t nTemplateIndex)
	:m_pReferenceImage(pReferenceImage)
	, m_pTemplateImage(pTemplateImage)
	, m_nReferenceIndex(nReferenceIndex)
	, m_nTemplateIndex(nTemplateIndex)
	, m_pTemplateImageSmooth(pTemplateImageSmooth)
	, m_pReferenceImageSmooth(pReferenceImageSmooth)
	, m_fftSize()
{
}

CImagePair::CImagePair(StlImage<float>&& ReferenceImage, StlImage<float>&& TemplateImage)
	:m_ReferenceImage(std::make_shared<StlImage<float>>(std::move(ReferenceImage))),
	m_TemplateImage(std::make_shared<StlImage<float>>(std::move(TemplateImage)))
{
	m_pReferenceImage = m_ReferenceImage.get();
	m_pTemplateImage = m_TemplateImage.get();
}

CImagePair::~CImagePair()
{
}




size_t CImagePair::GetReferenceImageIndex() const
{
	return m_nReferenceIndex;
}

size_t CImagePair::GetTemplateImageIndex() const
{
	return m_nTemplateIndex;
}

StlImage<float>* CImagePair::GetReferenceImage() const
{
	return m_pReferenceImage;
}

StlImage<float>* CImagePair::GetTemplateImage() const
{
	return m_pTemplateImage;
}
StlImage<float>* CImagePair::GetReferenceImageSmooth() const
{
	return m_pReferenceImageSmooth;
}

StlImage<float>* CImagePair::GetTemplateImageSmooth() const
{
	return m_pTemplateImageSmooth;
}

std::pair<StlImageRect, StlImageRect> CImagePair::GetOverlappingRectangles(StlImageSize refImageSize, StlImageSize tempImageSize, StlImagePoint offset)
{
	StlImageRect referenceImage(StlImagePoint(0, 0), refImageSize);
	StlImageRect templateImage(offset, tempImageSize);

	StlImageRect intersectingRegion;
	intersectingRegion.Intersection(referenceImage, templateImage);

	StlImageRect referenceIntersectingRegion(intersectingRegion);
	StlImageRect templateIntersectingRegion(intersectingRegion);

	if (offset.x < 0 && offset.y < 0)
	{
		referenceIntersectingRegion.MoveToXY(0, 0);
		templateIntersectingRegion.MoveToXY(-offset.x, -offset.y);
	}
	else if (offset.x < 0)
	{
		referenceIntersectingRegion.MoveToXY(0, offset.y);
		templateIntersectingRegion.MoveToXY(-offset.x, 0);
	}
	else if (offset.y < 0)
	{
		referenceIntersectingRegion.MoveToXY(offset.x, 0);
		templateIntersectingRegion.MoveToXY(0, -offset.y);
	}
	else
	{
		referenceIntersectingRegion.MoveToXY(offset.x, offset.y);
		templateIntersectingRegion.MoveToXY(0, 0);
	}

	return { referenceIntersectingRegion, templateIntersectingRegion };
}

StlImageSize CImagePair::GetFFTSize() const
{
	if (m_bUseDefaultFFTSize)
	{
		// This means that the imagesize + 10% is used to determine the FFTSize. This solves a problem with activated 
		// Shift-correction, where an image is only 382px wide, but still needs a proper FFTSize
		float fIncreasmentFactor = 1.1f;

		if (m_bUseMinimalFFTSize)
		{
			fIncreasmentFactor = 1;
		}

		auto refSize = GetReferenceImage()->GetSize();
		auto tempSize = GetTemplateImage()->GetSize();
		StlImageSize maxSize({ std::max(refSize.x, tempSize.x), std::max(refSize.y, tempSize.y) });

		auto FFTSizeX = CMathTools::NextPowOf2(static_cast<long long>(maxSize.x * fIncreasmentFactor));
		auto FFTSizeY = CMathTools::NextPowOf2(static_cast<long long>(maxSize.y * fIncreasmentFactor));

		return { FFTSizeX, FFTSizeY };
	}
	else
	{
		return m_fftSize;
	}
}

void CImagePair::SetUseMinimalFFTSize(bool bUse)
{
	m_bUseMinimalFFTSize = bUse;
}

void CImagePair::SetUseFixedFFTSize(StlImageSize fftSize)
{
	m_bUseDefaultFFTSize = false;
	m_fftSize = fftSize;
}

void CImagePair::SetUseDefaultFFTSize()
{
	m_bUseDefaultFFTSize = true;
}

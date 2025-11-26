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
#include "ImageMasker.h"
#include <ppl.h>

CImageMasker::CImageMasker(EMaskOptions eMaskType, StlImageSize MaskSize)
	:m_eMaskOption(eMaskType),
	m_MaskSize(MaskSize)
{
}


CImageMasker::~CImageMasker()
{
}

void CImageMasker::ProcessImages(const vector<StlImage<float>*>& SourceImages, vector<StlImage<float>*>& DestinationImages)
{
	m_nTotalImages += SourceImages.size();
	bool bInplace = SourceImages == DestinationImages;


	StlImageSize MaskingSize = DetectMaskingSize(SourceImages);

	concurrency::parallel_for(size_t(0), SourceImages.size(), [&](size_t nIndex)
		{
			if (bInplace)
				ProcessImage(SourceImages[nIndex], MaskingSize);
			else
			{
				(*DestinationImages[nIndex]) = (*SourceImages[nIndex]);
				ProcessImage(DestinationImages[nIndex], MaskingSize);
			}
			m_nProcessedImages++;
		});


}
void CImageMasker::ProcessImages(vector<StlImage<float>*>& Images)
{
	ProcessImages(Images, Images);
}
void CImageMasker::ProcessImages(StlImage<float>* pImage)
{
	vector<StlImage<float>*> ImageVector{ pImage };
	ProcessImages(ImageVector);
}

void CImageMasker::DoWork(const vector<StlImage<float>*>& SourceImages, vector<StlImage<float>*>& DestinationImages)
{
	ProcessImages(SourceImages, DestinationImages);
}
void CImageMasker::GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits)
{
	nTotalWorkUnits = m_nTotalImages;
	nCompletedWorkUnits = m_nProcessedImages;
}

void CImageMasker::ProcessImage(StlImage<float>* pImage, StlImageSize MaskingSize) const
{
	if (MaskingSize == StlImageSize())
		return;
	if (
		MaskingSize.x < pImage->GetSize().x ||
		MaskingSize.y < pImage->GetSize().y)
		throw std::exception("Masking Size was set lower than the biggest image size. Information would have been thrown away.");

	if (MaskingSize != pImage->GetSize())
		pImage->AddBorders(*pImage, 0, 0, MaskingSize.x - pImage->GetSize().x, MaskingSize.y - pImage->GetSize().y, 0.0);
}

StlImageSize CImageMasker::DetectMaskingSize(const vector<StlImage<float>*>& SourceImages) const
{
	StlImageSize MaskSize;
	switch (m_eMaskOption)
	{
	case EMaskOptions::eNone:
		return StlImageSize();
	case EMaskOptions::eMaskToFixSize:
		MaskSize = m_MaskSize;
	case EMaskOptions::eMaskToBiggest:
		MaskSize = DetermineBiggestImageSize(SourceImages);
		break;
	case EMaskOptions::eMaskToBiggestButMultipleOfN:
		MaskSize = DetermineBiggestImageSize(SourceImages);
		MaskSize = MakeMultipleOfN(MaskSize, m_MaskSize);
		break;
	default:
		break;
	}
	return MaskSize;
}
StlImageSize CImageMasker::DetermineBiggestImageSize(const vector<StlImage<float>*>& SourceImages) const
{
	StlImageSize MaxSize;
	for (const auto& pImage : SourceImages)
	{
		StlImageSize ImSize = pImage->GetSize();

		MaxSize.x = std::max(MaxSize.x, ImSize.x);
		MaxSize.y = std::max(MaxSize.y, ImSize.y);
	}
	return MaxSize;
}
StlImageSize CImageMasker::MakeMultipleOfN(StlImageSize& MaskSize, StlImageSize MultipleOf) const
{
	StlImageSize newSize;
	newSize.x = static_cast<LONG>(ceil(static_cast<double>(MaskSize.x) / static_cast<double>(MultipleOf.x))) * MultipleOf.x;
	newSize.y = static_cast<LONG>(ceil(static_cast<double>(MaskSize.y) / static_cast<double>(MultipleOf.y))) * MultipleOf.y;

	assert(newSize.x % MultipleOf.x == 0);
	assert(newSize.y % MultipleOf.y == 0);
	assert(newSize.x >= MaskSize.x);
	assert(newSize.y >= MaskSize.y);

	return newSize;
}
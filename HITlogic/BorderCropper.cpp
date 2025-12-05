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
#include "BorderCropper.h"



void CBorderCropper::ProcessImages(const vector<StlImage<float>*>& SourceImages, vector<StlImage<float>*>& DestinationImages)
{
	m_nTotalImages += SourceImages.size();
	bool bInplace = SourceImages == DestinationImages;

#ifdef _WIN32
	concurrency::parallel_for(size_t(0), SourceImages.size(), [&](size_t nIndex)
		{
			if (!bInplace && !DestinationImages[nIndex]->IsAllocated())
			{
				(*DestinationImages[nIndex]) = (*SourceImages[nIndex]);
			}
			CropBorder(DestinationImages[nIndex]);
			++m_nProcessedImages;
		});
#else
	for (size_t nIndex = 0; nIndex < SourceImages.size(); nIndex++)
	{
		if (!bInplace && !DestinationImages[nIndex]->IsAllocated())
		{
			(*DestinationImages[nIndex]) = (*SourceImages[nIndex]);
		}
		CropBorder(DestinationImages[nIndex]);
		++m_nProcessedImages;
	}
#endif
}
void CBorderCropper::ProcessImages(vector<StlImage<float>*>& Images)
{
	ProcessImages(Images, Images);
}

void CBorderCropper::DoWork(const vector<StlImage<float>*>& SourceImages, vector<StlImage<float>*>& DestinationImages)
{
	ProcessImages(SourceImages, DestinationImages);
}
void CBorderCropper::GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits)
{
	nTotalWorkUnits = m_nTotalImages;
	nCompletedWorkUnits = m_nProcessedImages;
}
void CBorderCropper::CropBorder(StlImage<float>* pImage)
{
	if (ContainsNegativeValues(pImage))
		throw std::exception("Image contains negative values.");

	pImage->CropBorders(*pImage);
}
bool CBorderCropper::ContainsNegativeValues(StlImage<float>* pImage)
{
	return std::get<0>(pImage->FindMin()) < 0;
}

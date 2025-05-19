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
#include "BrightnessCorrection.h"
#include <ppl.h>

CBrightnessCorrection::CBrightnessCorrection()
{
}


CBrightnessCorrection::~CBrightnessCorrection()
{
}

void CBrightnessCorrection::ProcessImages(const vector<StlImage<float>*>& SourceImages, vector<StlImage<float>*>& DestinationImages)
{
	m_nTotalImages = SourceImages.size();
	bool bInplace = SourceImages == DestinationImages;

	auto MeanImageBrightnessAndContrast = CalculateMeanBrightnessAndContrast(SourceImages);
	auto SeriesBrightnessAndContrast = CalculateSeriesBrightnessAndContrast(MeanImageBrightnessAndContrast);


	concurrency::parallel_for(size_t(0), SourceImages.size(), [&](size_t nIndex)
		{
			if (!m_bIsCanceled)
			{
				if (!bInplace && !DestinationImages[nIndex]->IsAllocated())
				{
					(*DestinationImages[nIndex]) = (*SourceImages[nIndex]);
				}

				ProcessImage(DestinationImages[nIndex],
					MeanImageBrightnessAndContrast[nIndex].first,
					MeanImageBrightnessAndContrast[nIndex].second,
					SeriesBrightnessAndContrast.first,
					SeriesBrightnessAndContrast.second);

				if (m_nProcessedImages++ % 10 == 0)
					ReportProgress();
			}
		});


}
void CBrightnessCorrection::ProcessImages(vector<StlImage<float>*>& Images)
{
	ProcessImages(Images, Images);
}


void CBrightnessCorrection::DoWork(const vector<StlImage<float>*>& SourceImages, vector<StlImage<float>*>& DestinationImages)
{
	ProcessImages(SourceImages, DestinationImages);
}
void CBrightnessCorrection::GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits)
{
	nTotalWorkUnits = m_nTotalImages;
	nCompletedWorkUnits = m_nProcessedImages;
}

void CBrightnessCorrection::ProcessImage(StlImage<float>* pImage, double fImageBrightness, double fImageContrast, double fSeriesBrightness, double fSeriesContrast) const
{
	auto sizeS = pImage->GetSize();

	vector<float> vInImage = pImage->AsArray();
	vector<float> vOutImage(vInImage.size());

	for (size_t j = 0; j < vInImage.size(); j++)
	{
		vOutImage[j] = static_cast<float>(((255.0 - fSeriesBrightness) / (255.0 - fImageBrightness)) * ((fSeriesContrast * (vInImage[j] - fImageBrightness)) / fImageContrast) + fSeriesBrightness);
		//vOutImage[j] = vInImage[j];
		if (vOutImage[j] < 0.0f)
			vOutImage[j] = 0.0f;
		else if (vOutImage[j] > 255.0f)
			vOutImage[j] = 255.0f;
	}

	pImage->Put(vOutImage.data());
}
vector<std::pair<double, double>> CBrightnessCorrection::CalculateMeanBrightnessAndContrast(const vector<StlImage<float>*>& SourceImages)
{
	vector<std::pair<double, double>> MeanBrightnessAndContrast(SourceImages.size());

	concurrency::parallel_for(size_t(0), SourceImages.size(), [&](size_t nIndex)
		{
			auto pImage = SourceImages[nIndex];
			double MeanBrightness = pImage->Mean();
			double MeanContrast = sqrt(pImage->MeanSquare());
			MeanBrightnessAndContrast[nIndex] = { MeanBrightness, MeanContrast };
		});

	return MeanBrightnessAndContrast;
}
std::pair<double, double> CBrightnessCorrection::CalculateSeriesBrightnessAndContrast(vector<std::pair<double, double>> MeanImageBrightnessAndContrast)
{
	double fSeriesBrightness = 0.0;
	double fSeriesContrast = 0.0;

	for (auto pair : MeanImageBrightnessAndContrast)
	{
		fSeriesBrightness += pair.first;
		fSeriesContrast += pair.second;
	}
	fSeriesBrightness /= MeanImageBrightnessAndContrast.size();
	fSeriesContrast /= MeanImageBrightnessAndContrast.size();
	return{ fSeriesBrightness , fSeriesContrast };
}
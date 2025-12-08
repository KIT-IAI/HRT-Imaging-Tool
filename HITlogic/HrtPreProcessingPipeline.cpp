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
#include "HrtPreProcessingPipeline.h"

#include "BorderCropper.h"
#include "ImageMasker.h"
//#include "HRTShiftCorrection.h"
#include "BrightnessCorrection.h"
#include "VignettingCorrection.h"



CHrtPreProcessingPipeline::CHrtPreProcessingPipeline()
	:CHrtPreProcessingPipeline(CHrtPreProcessingParameters())
{

}
CHrtPreProcessingPipeline::CHrtPreProcessingPipeline(CHrtPreProcessingParameters parameters)
	:m_Parameters(parameters)
{
	InitPipeline();
}

void CHrtPreProcessingPipeline::InitPipeline()
{
	if (m_Parameters.bCrobBlackBorder)
		AddSubsequentComputationStep(std::make_shared<CBorderCropper>());

	AddSubsequentComputationStep(std::make_shared<CImageMasker>(m_Parameters.eMaskImages, m_Parameters.MaskSize));

	/*if (m_Parameters.bShiftCorrection)
		AddSubsequentComputationStep(std::make_shared<CHRTShiftCorrection>());*/

	if (m_Parameters.bVigAdjustment)
	{
		if (m_Parameters.sVigAdjustmentProfilePath.empty())
			AddSubsequentComputationStep(std::make_shared<CVignettingCorrection>(m_Parameters.nVigCreationMaxImages, m_Parameters.nVigSmoothingIterations));
		else
			AddSubsequentComputationStep(std::make_shared<CVignettingCorrection>(m_Parameters.sVigAdjustmentProfilePath, m_Parameters.nVigSmoothingIterations));
	}

	if (m_Parameters.bBrightnessCorrection)
		AddSubsequentComputationStep(std::make_shared<CBrightnessCorrection>());
}
void CHrtPreProcessingPipeline::GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits)
{
	nCompletedWorkUnits = m_nProcessedImages;
	nTotalWorkUnits = m_nImageCount;
}
void CHrtPreProcessingPipeline::ProcessImagesInplace(std::vector<StlImage<float>*>& Images)
{
	Compute(Images, Images);
}
void CHrtPreProcessingPipeline::ProcessImages(const std::vector<StlImage<float>*>& InImages, std::vector<StlImage<float>*>& OutImages)
{
	Compute(InImages, OutImages);
}
std::vector<StlImage<float>*> CHrtPreProcessingPipeline::ProcessImages(const std::vector<StlImage<float>*>& InImages)
{
	std::vector<StlImage<float>*> OutImages(InImages.size());
	std::generate(OutImages.begin(), OutImages.end(), []() {return new StlImage<float>(); });

	Compute(InImages, OutImages);

	return OutImages;
}
void CHrtPreProcessingPipeline::DoWork(const std::vector<StlImage<float>*>& SourceImages, std::vector<StlImage<float>*>& /*DestinationImages*/)
{
	m_nImageCount += SourceImages.size();
	// Intentionally empty
}
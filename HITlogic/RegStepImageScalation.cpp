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
#include "RegStepImageScalation.h"



CRegStepImageScalation::CRegStepImageScalation(double fScalationFactor)
	:m_bScaleUp(false),
	m_rScalationFactor(boost::rational<int>(1, static_cast<int>(fScalationFactor))),
	m_nTotalWorkUnits(1),
	m_nCompletedWorkUnits(0)
{
}
CRegStepImageScalation::~CRegStepImageScalation()
{
	if (m_OriginalImages.size() > 0)
		FreeResizedImages(m_OriginalImages);
}

void CRegStepImageScalation::FreeResizedImages(vector<StlImage<float>*>& images)
{
	for (auto pImage : images)
	{
		delete pImage;
		m_nCompletedWorkUnits++;
	}
}

void CRegStepImageScalation::RescaleImagesToOriginalSize(vector<StlImage<float>*>& images)
{
	FreeResizedImages(images);
	images.swap(m_OriginalImages);

	m_OriginalImages.resize(0);
}

void CRegStepImageScalation::ResizeImages(vector<StlImage<float>*>& images)
{
	if (m_bScaleUp)
		return RescaleImagesToOriginalSize(images);

	m_OriginalImages.resize(images.size());
	for (size_t i = 0; i < images.size(); i++)
	{
		auto imSize = images[i]->GetSize();
		m_OriginalImages[i] = new StlImage<float>();
		m_OriginalImages[i]->Alloc({ static_cast<int>(imSize.x * boost::rational_cast<double>(m_rScalationFactor)), static_cast<int>(imSize.y * boost::rational_cast<double>(m_rScalationFactor)) });
		m_OriginalImages[i]->Resize(*images[i], m_rScalationFactor);
		m_nCompletedWorkUnits++;
	}
	m_OriginalImages.swap(images);
}

void CRegStepImageScalation::ResizeRegResults(CImageRegistrationResult& reg_results)
{
	auto rScalationFactor = m_rScalationFactor;

	if (m_bScaleUp)
		rScalationFactor = 1 / rScalationFactor;

	for (auto& result : reg_results.RegistrationResults)
	{
		result.Scale(result, boost::rational_cast<float>(rScalationFactor));
		m_nCompletedWorkUnits++;
	}
}


void CRegStepImageScalation::DoWork(CImageRegistrationData& registrationData, CImageRegistrationResult& RegResults)
{
	m_nCompletedWorkUnits = 0;
	m_nTotalWorkUnits = registrationData.Images.size() + RegResults.RegistrationResults.size();
	ResizeImages(registrationData.Images);
	ResizeRegResults(RegResults);

	m_bScaleUp = !m_bScaleUp;
}
void CRegStepImageScalation::GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits)
{
	nCompletedWorkUnits = m_nCompletedWorkUnits;
	nTotalWorkUnits = m_nTotalWorkUnits;
}
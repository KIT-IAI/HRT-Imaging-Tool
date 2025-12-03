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
#include "VignettingCorrection.h"

#include <filesystem>

#ifdef _WIN32
#include <ppl.h>
#endif



CVignettingCorrection::CVignettingCorrection(const wstring& ProfileFilePath)
	:CVignettingCorrection(ProfileFilePath, 0)
{
}

CVignettingCorrection::CVignettingCorrection(const wstring& ProfileFilePath, size_t nSmoothIterations)
{
	m_nSmoothIterations = nSmoothIterations;
	if (ProfileFilePath == L"")	// Autocreate
		return;
	wstring sLocalPath = ProfileFilePath;
	if (CFileUtilities::IsRelative(ProfileFilePath))
		sLocalPath = CFileUtilities::GetAbsolutePath(CFileUtilities::GetProgramFolder(), ProfileFilePath);

	m_pVignettingProfile->LoadConvert(sLocalPath);
	SmoothProfile(nSmoothIterations, *m_pVignettingProfile);
	*m_pVignettingProfile /= std::get<0>(m_pVignettingProfile->FindMax());
}

CVignettingCorrection::CVignettingCorrection(const StlImage<float>& VigProfile)
	:CVignettingCorrection(VigProfile, 0)
{
}

CVignettingCorrection::CVignettingCorrection(const StlImage<float>& VigProfile, size_t nSmoothIterations)
{
	m_nSmoothIterations = nSmoothIterations;
	(*m_pVignettingProfile) = (VigProfile);
	SmoothProfile(nSmoothIterations, *m_pVignettingProfile);
	*m_pVignettingProfile /= std::get<0>(m_pVignettingProfile->FindMax());
}

CVignettingCorrection::CVignettingCorrection(size_t nCreationMaxImages)
	:CVignettingCorrection(nCreationMaxImages, 0)
{
}

CVignettingCorrection::CVignettingCorrection(size_t nCreationMaxImages, size_t nSmoothIterations)
{
	m_nCreationMaxImages = nCreationMaxImages;
	m_nSmoothIterations = nSmoothIterations;
}

void CVignettingCorrection::SmoothProfile(size_t nTimes, StlImage<float>& Profile)
{
	for (size_t i = 0; i < nTimes; i++)
	{
		Profile.Smooth(Profile);
	}
}

StlImage<float> CVignettingCorrection::CreateVignettingProfile(const vector<StlImage<float>*>& InputImages, size_t nMaxImages, size_t nSmoothIterations)
{
	if (InputImages.size() == 0)
		throw std::exception("Invalid argument \"InputImages\". An empty vector was passed.");

	StlImage<float> NewProfile;
	NewProfile.Alloc(InputImages[0]->GetSize());
	NewProfile.Clear(0.0);

	if (nMaxImages == 0)
		nMaxImages = InputImages.size();

	assert(nMaxImages > 0);

	size_t nCount = 0;
	for (const auto& pImage : InputImages)
	{
		if (pImage->GetSize() != NewProfile.GetSize())
			throw std::exception("Image sizes differ. Could not create VignettingProfile.");
		NewProfile += *pImage;
		nCount++;
		if (nCount >= nMaxImages)
			break;
	}
	NewProfile /= static_cast<int>(nCount);

	SmoothProfile(nSmoothIterations, NewProfile);

	NewProfile /= std::get<0>(NewProfile.FindMax());

	return NewProfile;
}

void CVignettingCorrection::DoWork(const vector<StlImage<float>*>& SourceImages, vector<StlImage<float>*>& DestinationImages)
{
	ProcessImages(SourceImages, DestinationImages);
}

void CVignettingCorrection::ProcessImages(vector<StlImage<float>*>& Images)
{
	ProcessImages(Images, Images);
}

void CVignettingCorrection::ProcessImages(const vector<StlImage<float>*>& SourceImages, vector<StlImage<float>*>& DestinationImages)
{
	nTotalPairs = SourceImages.size();
	nPairsDone = 0;

	bool bFreeProfile = !m_pVignettingProfile->IsAllocated();
	if (bFreeProfile)
	{
		auto VigProfile = CreateVignettingProfile(SourceImages, m_nCreationMaxImages, m_nSmoothIterations);
		(*m_pVignettingProfile) = (VigProfile);
	}

	bool bInplace = SourceImages == DestinationImages;

#ifdef _WIN32
	concurrency::parallel_for(size_t(0), SourceImages.size(), [&](size_t nIndex)
		{
			if (!bInplace && !DestinationImages[nIndex]->IsAllocated())
			{
				(*DestinationImages[nIndex]) = (*SourceImages[nIndex]);
			}

			ProcessImage(DestinationImages[nIndex]);

			if (nPairsDone++ % 10 == 0)
			{
				ReportProgress();
			}
		});
#else
	for (size_t nIndex = 0; nIndex < SourceImages.size(); nIndex++)
	{
		if (!bInplace && !DestinationImages[nIndex]->IsAllocated())
		{
			(*DestinationImages[nIndex]) = (*SourceImages[nIndex]);
		}

		ProcessImage(DestinationImages[nIndex]);

		if (nPairsDone++ % 10 == 0)
		{
			ReportProgress();
		}
	}
#endif

	if (bFreeProfile)
		m_pVignettingProfile->Free();
}

void CVignettingCorrection::ProcessImage(StlImage<float>* pImage)
{
	auto sizeX = (m_pVignettingProfile->GetSize().x - pImage->GetSize().x) / 2;
	auto sizeY = (m_pVignettingProfile->GetSize().y - pImage->GetSize().y) / 2;
	StlImage<float> smallVigImage;
	smallVigImage.Child2dIndep(*m_pVignettingProfile, { sizeX,sizeY }, pImage->GetSize());
	*pImage /= smallVigImage;
	pImage->Clamp(0, 255);
}

void CVignettingCorrection::GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits)
{
	nCompletedWorkUnits = nPairsDone;
	nTotalWorkUnits = nTotalPairs;
}

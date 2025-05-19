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
#include "ScoreNormalizer.h"

const float CScoreNormalizer::s_fAreaNormalizationConstant = 1/*std::numeric_limits<float>::denorm_min()*/;

CScoreNormalizer::CScoreNormalizer()
{
}

void CScoreNormalizer::CalculateScoreNorm1(const StlImage<float>& Phase_Image, StlImage<float>& Normalized_Image) const
{
	return NormToStandardDeviation(Phase_Image, Normalized_Image);
}

void CScoreNormalizer::NormToStandardDeviation(const StlImage<float>& Phase_Image, StlImage<float>& Normalized_Image) const
{
	double std2 = Phase_Image.StandardDeviation();
	Normalized_Image = Phase_Image / std2;
}

void CScoreNormalizer::NormToStandardDeviation(StlImage<float>& Normalized_Image) const
{
	return NormToStandardDeviation(Normalized_Image, Normalized_Image);
}

//void CScoreNormalizer::AllocMask(CMeasureImage& image, CSize fft_size, CSize image_size) const
//{
//	image.Alloc2d(fft_size, M_FLOAT | 32);
//	image.Clear(0.0);
//	CMeasureImage child;
//	child.Child2d(image, 0, 0, image_size.cx, image_size.cy);
//	child.Clear(1.0);
//	child.Free();
//}
//
//void CScoreNormalizer::CreateFFTOfMask(CComplexImage& image, CSize fft_size, CSize image_size) const
//{
//	CMeasureImage mask;
//	AllocMask(mask, fft_size, image_size);
//
//	image.Alloc2d(fft_size, M_FLOAT | 32);
//	image.Transform(mask, M_FFT, M_FORWARD);
//}
//
//void CScoreNormalizer::GenerateOverlappingPixelMask(CMeasureImage& overlapping_pixels, CSize FFTSize, CSize image_size1, CSize image_size2) const
//{
//	CComplexImage image1;
//	CComplexImage image2;
//
//	CreateFFTOfMask(image1, FFTSize, image_size1);
//	CreateFFTOfMask(image2, FFTSize, image_size2);
//
//	image2.Conjugate();
//	image1.Arith(image1, image2, M_MULT);
//	image1.Transform(image1, M_FFT, M_REVERSE);
//
//	overlapping_pixels.Alloc2d(FFTSize, M_FLOAT | 32);
//	overlapping_pixels.Copy(*image1.GetRealBuffer());
//	//overlapping_pixels.Export();
//	overlapping_pixels.Arith(overlapping_pixels, s_fAreaNormalizationConstant, M_ADD_CONST);
//	//overlapping_pixels.Export();
//}
//
//bool CScoreNormalizer::HasBeenCalculated(std::pair<CSize, CSize> size_pair) const
//{
//	return GetImageFor(size_pair) != nullptr;
//}
//
//void CScoreNormalizer::AddToList(std::shared_ptr<CMeasureImage> overlapping_pixels, std::pair<CSize, CSize> size_pair) const
//{
//	auto pairToAdd = std::pair < std::pair<CSize, CSize>, std::shared_ptr<CMeasureImage>>(size_pair, overlapping_pixels);
//	m_OverlappingPixelImagesList.push_front(pairToAdd);
//
//	if (m_OverlappingPixelImagesList.size() > s_nMaximumOverlapImagesToStore)
//		m_OverlappingPixelImagesList.pop_back();
//}
//
//void CScoreNormalizer::CalculateOverlappingPixels(CSize fft_size, std::pair<CSize, CSize> size_pair) const
//{
//	auto overlappingPixels = std::make_shared<CMeasureImage>();
//	GenerateOverlappingPixelMask(*overlappingPixels, fft_size, size_pair.first, size_pair.second);
//	AddToList(overlappingPixels, size_pair);
//}
//
//std::shared_ptr<CMeasureImage> CScoreNormalizer::GetImageFor(std::pair<CSize, CSize> size_pair) const
//{
//	for (auto pair : m_OverlappingPixelImagesList)
//	{
//		if (pair.first == size_pair)
//			return pair.second;
//	}
//	return nullptr;
//}
//
//void CScoreNormalizer::NormToOverlappingArea(const CMeasureImage& Phase_Image, CMeasureImage& Normalized_Image, const CMeasureImage& OverlappingPixels) const
//{
//	Normalized_Image.Arith(Phase_Image, OverlappingPixels, M_DIV);
//}
//
//void CScoreNormalizer::MoveToFrontOfList(std::pair<CSize, CSize> size_pair) const
//{
//	for (auto pair : m_OverlappingPixelImagesList)
//	{
//		if (pair.first == size_pair)
//		{
//			m_OverlappingPixelImagesList.remove(pair);
//			m_OverlappingPixelImagesList.push_front(pair);
//			return;
//		}
//	}
//}
//
//void CScoreNormalizer::NormToOverlappingArea(const CMeasureImage& Phase_Image, CMeasureImage& Normalized_Image, CSize Image_Size1, CSize Image_Size2) const
//{
//	auto pOverlappingPixels = InquireOverlappingPixelMask(Phase_Image.GetSize(), Image_Size1, Image_Size2);
//	return NormToOverlappingArea(Phase_Image, Normalized_Image, *pOverlappingPixels);
//}
//
//std::shared_ptr<CMeasureImage> CScoreNormalizer::InquireOverlappingPixelMask(CSize fft_size, CSize Image_Size1, CSize Image_Size2) const
//{
//	auto SizePair = std::pair<CSize, CSize>(Image_Size1, Image_Size2);
//
//	OverlappingPixelGenerationMutex.lock();
//	if (!HasBeenCalculated(SizePair))
//	{
//		CalculateOverlappingPixels(fft_size, SizePair);
//	}
//	auto pImage = GetImageFor(SizePair);
//	MoveToFrontOfList(SizePair);
//	OverlappingPixelGenerationMutex.unlock();
//	return pImage;
//}

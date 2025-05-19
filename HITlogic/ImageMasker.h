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
#include "Computable.h"
#include <atomic>
#include "StlImage.h"

enum class EMaskOptions
{
	eNone = 0,
	eMaskToBiggest = 1,
	eMaskToFixSize = 2,
	eMaskToBiggestButMultipleOfN = 3,
};

class CImageMasker :
	public IComputable<const std::vector<StlImage<float>*>&, std::vector<StlImage<float>*>&>
{
public:
	CImageMasker(EMaskOptions eMaskType, StlImageSize MaskSize = StlImageSize());
	~CImageMasker();

	virtual void ProcessImages(const std::vector<StlImage<float>*>& SourceImages, std::vector<StlImage<float>*>& DestinationImages);
	virtual void ProcessImages(std::vector<StlImage<float>*>& Images);
	virtual void ProcessImages(StlImage<float>* pImage);

protected:
	void DoWork(const std::vector<StlImage<float>*>& SourceImages, std::vector<StlImage<float>*>& DestinationImages) override;
	void GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits) override;
private:
	void ProcessImage(StlImage<float>* pImage, StlImageSize MaskingSize) const;
	StlImageSize DetectMaskingSize(const std::vector<StlImage<float>*>& SourceImages) const;
	StlImageSize DetermineBiggestImageSize(const std::vector<StlImage<float>*>& SourceImages) const;
	StlImageSize MakeMultipleOfN(StlImageSize& MaskSize, StlImageSize MultipleOf) const;

	EMaskOptions m_eMaskOption;
	StlImageSize m_MaskSize;

	size_t m_nTotalImages = 0;
	std::atomic_size_t m_nProcessedImages = 0;
};


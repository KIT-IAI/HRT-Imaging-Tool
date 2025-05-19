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

class CVignettingCorrection :
	public IComputable<const std::vector<StlImage<float>*>&, std::vector<StlImage<float>*>&>
{
public:
	explicit CVignettingCorrection(const std::wstring& ProfileFilePath);
	explicit CVignettingCorrection(const std::wstring& ProfileFilePath, size_t nSmoothIterations);
	explicit CVignettingCorrection(const StlImage<float>& VigProfile);
	explicit CVignettingCorrection(const StlImage<float>& VigProfile, size_t nSmoothIterations);
	explicit CVignettingCorrection(size_t nCreationMaxImages);
	explicit CVignettingCorrection(size_t nCreationMaxImages, size_t nSmoothIterations);
	CVignettingCorrection() = default;
	~CVignettingCorrection() = default;

	static StlImage<float> CreateVignettingProfile(const std::vector<StlImage<float>*>& InputImages, size_t nMaxImages, size_t nSmoothIterations);

	virtual void ProcessImages(const std::vector<StlImage<float>*>& SourceImages, std::vector<StlImage<float>*>& DestinationImages);
	virtual void ProcessImages(std::vector<StlImage<float>*>& Images);

protected:
	void DoWork(const std::vector<StlImage<float>*>& SourceImages, std::vector<StlImage<float>*>& DestinationImages) override;
	void GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits) override;

private:
	static void SmoothProfile(size_t nTimes, StlImage<float>& Profile);
	virtual void ProcessImage(StlImage<float>* pImage);
	static std::filesystem::path GetExePath();

	std::unique_ptr<StlImage<float>> m_pVignettingProfile = std::make_unique<StlImage<float>>();
	size_t m_nCreationMaxImages = 0;
	size_t m_nSmoothIterations = 0;

	std::atomic_size_t nPairsDone = 0;
	size_t nTotalPairs = 0;
};

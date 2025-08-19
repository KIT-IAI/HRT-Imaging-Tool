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

#include "ImageRegistrationResult.h"

CImageRegistrationResult::CImageRegistrationResult(const vector<CRegistrationResult>& RegistrationResults, const vector<std::list<size_t>>& ImageGroups)
	: RegistrationResults(RegistrationResults)
	, ImageGroups(ImageGroups)
{
};

CImageRegistrationResult::CImageRegistrationResult(const vector<CRigidRegistrationResult>& RigidRegistrations, const vector<CRigidRegistrationResult>& FlexibleRegistrations)
{
	RegistrationResults = GenerateRegistrationResults(RigidRegistrations, FlexibleRegistrations);
	RecalulateImageGroups();
}

/*
Recalculates all image groups based on the successful registration results.
*/
void CImageRegistrationResult::RecalulateImageGroups()
{
	ImageGroups.clear();

	CGraph registrationGraph = CreateRegistrationGraph();

	registrationGraph.ListSubGraphs(ImageGroups);
}
CGraph CImageRegistrationResult::CreateRegistrationGraph() const
{
	size_t nImageCount = DetermineImageCount();

	CGraph registrationGraph(nImageCount);

	for (auto registration : RegistrationResults)
	{
		if (!registration.IsValid())
			continue;
		auto refIndex = registration.RigidRegistrationResult.GetReferenceImageIndex();
		auto tempIndex = registration.RigidRegistrationResult.GetTemplateImageIndex();
		registrationGraph.AddBiDirectionalEdge(refIndex, tempIndex);
	}
	return registrationGraph;
}

/*
Determines the image count based on the highest template image index
*/
size_t CImageRegistrationResult::DetermineImageCount() const
{
	if (RegistrationResults.size() == 0)
		return 0;

	size_t nMaxTemplateImageIndex = RegistrationResults[0].GetTemplateImageIndex();

	for (const auto& RegResult : RegistrationResults)
	{
		nMaxTemplateImageIndex = std::max(nMaxTemplateImageIndex, RegResult.GetTemplateImageIndex());
	}
	return nMaxTemplateImageIndex + 1;
}

vector<CRegistrationResult> CImageRegistrationResult::GenerateRegistrationResults(const vector<CRigidRegistrationResult>& RigidRegistrations, const vector<CRigidRegistrationResult>& FlexibleRegistrations) const
{
	vector<CRegistrationResult> Result;
	for (const auto& RigidReg : RigidRegistrations)
	{
		CRegistrationResult Reg(RigidReg);

		std::copy_if(FlexibleRegistrations.begin(), FlexibleRegistrations.end(), std::back_inserter(Reg.FlexibleRegistrationResults),
			[&](const CRigidRegistrationResult& FlexReg)
			{
				return	FlexReg.GetReferenceImageIndex() == RigidReg.GetReferenceImageIndex()
					&& FlexReg.GetTemplateImageIndex() == RigidReg.GetTemplateImageIndex();
			});

		Result.push_back(Reg);
	}
	return Result;
}

bool CImageRegistrationResult::HasOnlySingleImageGroups() const
{
	return all_of(ImageGroups.begin(), ImageGroups.end(), [&](const std::list<size_t>& group) {return group.size() == 1; });
}

vector<CRigidRegistrationResult> CImageRegistrationResult::GetRigidRegistrationResults() const
{
	vector<CRigidRegistrationResult> RigidRegistrationResults;
	RigidRegistrationResults.reserve(RegistrationResults.size());

	for (auto result : RegistrationResults)
		RigidRegistrationResults.push_back(result.RigidRegistrationResult);
	return RigidRegistrationResults;
}

vector<CRigidRegistrationResult> CImageRegistrationResult::GetFlexibleRegistrationResults() const
{
	vector<CRigidRegistrationResult> FlexibleRegistrationResults;

	for (auto result : RegistrationResults)
		FlexibleRegistrationResults.insert(FlexibleRegistrationResults.end(), result.FlexibleRegistrationResults.begin(), result.FlexibleRegistrationResults.end());

	return FlexibleRegistrationResults;
}

/*
* Returns a vector of ImageRegistrationResults. Each element of the resulting vector represents one image group.
* It contains all valid registration results of this group.
* Additionally ImageGroups[0] contains a list of all image indices in this group.
*/
vector<CImageRegistrationResult> CImageRegistrationResult::GetResultsByGroup() const
{
	vector<CImageRegistrationResult> ResultsByGroup;

	for (auto ImageGroup : ImageGroups)
	{
		ImageGroup.sort();

		CImageRegistrationResult GroupResult;
		GroupResult.RegistrationResults = FindRegistrationResultsByGroup(ImageGroup);
		GroupResult.ImageGroups.push_back(ImageGroup);
		//GroupResult.RemapIndices();

		// if there are NO registrations, then the will be no image groups... questionable
		// Still only 1 image group
		ASSERT(GroupResult.ImageGroups.size() <= 1);

		ResultsByGroup.push_back(GroupResult);
	}

	return ResultsByGroup;
}

vector<CRegistrationResult> CImageRegistrationResult::FindRegistrationResultsByGroup(const std::list<size_t>& ImageGroup) const
{
	// maximum image index (in group)
	size_t maxImageIndex = *std::max_element(ImageGroup.begin(), ImageGroup.end());

	// lookup table (LUT) that marks the group images
	vector<bool> groupLUT(maxImageIndex + 1, false);
	std::for_each(ImageGroup.begin(), ImageGroup.end(), [&groupLUT](size_t imageIndex) { groupLUT[imageIndex] = true; });

	vector<CRegistrationResult> GroupRegResults;
	for (auto& RegResult : RegistrationResults)
	{
		// reference and template image of a valid registration have to be in the same image group
		ASSERT(RegResult.GetReferenceImageIndex() < RegResult.GetTemplateImageIndex());
		ASSERT(!RegResult.IsValid() || (RegResult.GetTemplateImageIndex() > maxImageIndex) || (groupLUT[RegResult.GetReferenceImageIndex()] == groupLUT[RegResult.GetTemplateImageIndex()]));

		if (RegResult.IsValid() && (RegResult.GetReferenceImageIndex() <= maxImageIndex) && groupLUT[RegResult.GetReferenceImageIndex()])
			GroupRegResults.push_back(RegResult);
	}
	return GroupRegResults;
}

CImageRegistrationResult CImageRegistrationResult::GetValidResults() const
{
	vector<CRegistrationResult> ValidResults;

	for (const auto& Result : RegistrationResults)
	{
		if (Result.IsValid())
			ValidResults.push_back(Result);
	}

	return CImageRegistrationResult(ValidResults, ImageGroups);
}

void CImageRegistrationResult::RemapIndices()
{
	ASSERT(ImageGroups.size() == 1);

	auto& group = ImageGroups[0];

	// maximum image index (in group)
	size_t maxImageIndex = *std::max_element(group.begin(), group.end());

	// group image index table (-1 indicates images not in group)
	vector<ptrdiff_t> indexMap(maxImageIndex + 1, -1);
	size_t newIndex = 0;
	std::for_each(group.begin(), group.end(), [&indexMap, &newIndex](size_t oldIndex) { indexMap[oldIndex] = newIndex++; });

	// remap indexes in registration results
	for (auto& NewRegistrationResult : RegistrationResults)
	{
		size_t referenceImage = NewRegistrationResult.GetReferenceImageIndex();
		size_t templateImage = NewRegistrationResult.GetTemplateImageIndex();

		if ((referenceImage > maxImageIndex) || (indexMap[referenceImage] < 0) || (templateImage > maxImageIndex) || (indexMap[templateImage] < 0))
			throw std::exception("Error when trying to remap indices for a registration group.");

		NewRegistrationResult.SetReferenceImageIndex(indexMap[referenceImage]);
		NewRegistrationResult.SetTemplateImageIndex(indexMap[templateImage]);
	}

	// remap indexes in group image list
	newIndex = 0;
	std::for_each(group.begin(), group.end(), [&newIndex](size_t& oldIndex) { oldIndex = newIndex++; });
}

bool CImageRegistrationResult::operator==(const CImageRegistrationResult& rhs) const
{
	return RegistrationResults == rhs.RegistrationResults
		&& ImageGroups == rhs.ImageGroups;
}
bool CImageRegistrationResult::operator!=(const CImageRegistrationResult& rhs) const
{
	return !(*this == rhs);
}

void CImageRegistrationResult::Sort()
{
	std::sort(RegistrationResults.begin(), RegistrationResults.end(), [](const auto& lhs, const auto& rhs)
		{
			if (lhs.GetReferenceImageIndex() != rhs.GetReferenceImageIndex())
				return lhs.GetReferenceImageIndex() < rhs.GetReferenceImageIndex();
			if (lhs.GetTemplateImageIndex() != rhs.GetTemplateImageIndex())
				return lhs.GetTemplateImageIndex() < rhs.GetTemplateImageIndex();
			return lhs.RigidRegistrationResult.GetSubImageRowIndex() < rhs.RigidRegistrationResult.GetSubImageRowIndex();
		});
}

void CImageRegistrationResult::ForceSingleImageGroup()
{
	size_t imageCount = DetermineImageCount();
	std::list<size_t> singleGroup;
	for (size_t i = 0; i < imageCount; ++i)
		singleGroup.push_back(i);

    ImageGroups.clear();
	ImageGroups.push_back(singleGroup);

}
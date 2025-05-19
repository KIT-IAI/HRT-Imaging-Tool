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

#include "ImageRegistrationResult.h"
#include "SNPDatasetParameters.h"
#include "MosaicImageProperties.h"
#include "SNPDatasetOutputParameters.h"
#include "Compositing.h"				//enthält typedef für CSNPCompositingResult
#include "SNPGroupResult.h"

class CSNPDatasetResultLoader
{
public:
	~CSNPDatasetResultLoader() = default;
	static CSNPDatasetResultLoader FromImagePath(const std::wstring& sImagePath);
	static CSNPDatasetResultLoader FromOutputFolder(const std::wstring& sOutputFolderPath);
	static CSNPDatasetResultLoader FromSQLitePath(const std::wstring& sSQLitePath);

	std::wstring GetDatabasePath() const;

	void CreateFolderStructure(size_t nImageGroupCount) const;
	void ExportRegistrations(const CImageRegistrationResult& Result) const;
	void ExportImageList(const std::vector<std::wstring>& ImageFiles, const std::vector<std::list<size_t>>& ImageGroups) const;
	void ExportImageList(const std::vector<std::wstring>& ImageFiles) const;
	void ExportInputParameters(const CSNPDatasetParameters& Parameters) const;
	void ExportOutputParameters(const CSNPDatasetOutputParameters& Parameters) const;
	void ExportPositioningSolution(const CDenseMatrix& Solution, size_t nGroup) const;
	void ExportPositioningSolutionToSpecialTable(const CDenseMatrix& Solution, size_t nGroup, std::wstring fileName) const;
	void ExportPositioningSolutionToSpecialTable(const CDenseMatrix& Solution, std::vector<int> groupIDPerRow, std::wstring fileName) const;
	void ExportPositioningSolution(const std::vector<CDenseMatrix>& Solutions) const;
	void ExportStlMosaics(const std::vector<StlImage<float>*>& MosaicImages) const;
	void ExportStlMosaic(const StlImage<float>& MosaicImage, size_t nGroupID) const;
	void ExportResults(const std::vector<CSNPCompositingResult*>& results) const;
	void ExportResult(const CSNPCompositingResult* result, size_t nGroupID) const;
	void Export3DCloud(const C3DCloud& result, size_t nGroupID) const;
	void ExportVoxelBuffer(const C3DBuffer<float>& result, size_t nGroupID) const;
	void ExportMotionCorrectedImages(const std::vector<CSNPGroupResult>& groupResults) const;
	void ExportMotionCorrectedImages(const CSNPGroupResult& groupResult, size_t nGroupID) const;

	std::vector<CDenseMatrix> ImportSolutions() const;
	CDenseMatrix ImportSolution(size_t nGroupID) const;
	CSNPDatasetOutputParameters ImportOutputParameters() const;
	CImageRegistrationResult ImportRegistrationResults() const;
	std::vector<std::list<size_t>> ImportImageGroups() const;
	CSNPDatasetParameters ImportInputParameters() const;
	std::vector<std::wstring> ImportImageList() const;
	std::vector<std::wstring> ImportImageListAndNormalizeRelativePaths() const;
	std::vector<CMosaicImageProperties> ImportMosaicImageProperties() const;
	CMosaicImageProperties ImportMosaicImageProperties(size_t nGroupId) const;
	std::vector<std::unique_ptr<StlImage<float>>> ImportMosaicImages() const;

private:
	static std::wstring DetermineRootPathFromImagePath(const std::wstring& sImagePath);
	static std::wstring DetermineSeriesNameFromRootPath(const std::wstring& sImagePath);

	explicit CSNPDatasetResultLoader(const std::wstring& sSQLitePath);

	std::wstring GetGroupName(size_t nGroupID) const;
	std::wstring GetGroupRootFolder(size_t nGroupID) const;
	std::vector<std::list<size_t>> GenerateSingleImageGroup(size_t size) const;
	static CDenseMatrix ExpandSolutionWithGroupId(const CDenseMatrix& Solution, size_t nGroup);

private:
	std::wstring sDatabasePath;
	std::wstring sRootPath;
	std::wstring sSeriesName;

	static const size_t nGroupStartIndex = 1;
};



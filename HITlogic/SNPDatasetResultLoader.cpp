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
#include "SNPDatasetResultLoader.h"

#include "DIPLOMTableRow.h"
#include "DIPLOMTable.h"
#include "ImageLoader.h"
#include "RegistrationResultLoader.h"
#include "SNPFusionException.h"



CSNPDatasetResultLoader CSNPDatasetResultLoader::FromImagePath(const std::wstring& sImagePath)
{
	auto sRootPath = DetermineRootPathFromImagePath(sImagePath);
	auto sSeriesName = DetermineSeriesNameFromRootPath(sRootPath);
	return CSNPDatasetResultLoader(CFileUtilities::FullFile({ sRootPath, sSeriesName + L".sqlite" }));
}

CSNPDatasetResultLoader CSNPDatasetResultLoader::FromOutputFolder(const std::wstring& sOutputFolderPath)
{
	auto sSeriesName = DetermineSeriesNameFromRootPath(sOutputFolderPath);
	return CSNPDatasetResultLoader(CFileUtilities::FullFile({ sOutputFolderPath, sSeriesName + L".sqlite" }));
}

CSNPDatasetResultLoader CSNPDatasetResultLoader::FromSQLitePath(const std::wstring& sSQLitePath)
{
	return CSNPDatasetResultLoader(sSQLitePath);
}

CSNPDatasetResultLoader::CSNPDatasetResultLoader(const std::wstring& sSQLitePath)
{
	sRootPath = CFileUtilities::GetParentDirectory(sSQLitePath);
	sSeriesName = DetermineSeriesNameFromRootPath(sRootPath);
	sDatabasePath = sSQLitePath;
}

std::wstring CSNPDatasetResultLoader::DetermineRootPathFromImagePath(const std::wstring& sImagePath)
{
	auto imageName = CFileUtilities::GetFileName(sImagePath);
	auto seriesName = imageName.substr(0, imageName.rfind(L"_"));

	auto snpFolder = CFileUtilities::GetParentDirectory(sImagePath);
	auto seriesFolder = CFileUtilities::GetParentDirectory(snpFolder);
	auto mFolder = CFileUtilities::FullFile({ seriesFolder, seriesName + L"_m" });

	return mFolder;
}

std::wstring CSNPDatasetResultLoader::DetermineSeriesNameFromRootPath(const std::wstring& sRootPath)
{
	auto FileParts = CFileUtilities::FileParts(sRootPath);
	auto mFolder = FileParts.back();
	auto seriesName = mFolder.substr(0, mFolder.rfind(L"_"));

	return seriesName;
}

void CSNPDatasetResultLoader::ExportRegistrations(const CImageRegistrationResult& Result) const
{
	CSQLiteDatabase Database(GetDatabasePath());

	CRegistrationResultLoader::SaveToSQLite(Result, Database);
	CRegistrationResultLoader::SaveToTextFile(Result, sRootPath, sSeriesName);

	size_t nGroupID = nGroupStartIndex;
	for (const auto& ResultsForGroup : Result.GetResultsByGroup())
	{
		CRegistrationResultLoader::SaveToTextFile(ResultsForGroup, GetGroupRootFolder(nGroupID), GetGroupName(nGroupID));
		nGroupID++;
	}
}

std::wstring CSNPDatasetResultLoader::GetDatabasePath() const
{
	return sDatabasePath;
}

void CSNPDatasetResultLoader::ExportImageList(const std::vector<std::wstring>& ImageFiles) const
{
	return ExportImageList(ImageFiles, GenerateSingleImageGroup(ImageFiles.size()));
}

std::vector<std::list<size_t>> CSNPDatasetResultLoader::GenerateSingleImageGroup(size_t size) const
{
	std::vector<std::list<size_t>> ImageGroups;
	std::list<size_t> SingleImageGroup(size);

	size_t i = 0;
	std::generate_n(SingleImageGroup.begin(), size, [&]() {return i++; });
	ImageGroups.push_back(SingleImageGroup);

	return ImageGroups;
}

void CSNPDatasetResultLoader::ExportImageList(const std::vector<std::wstring>& ImageFiles, const std::vector<std::list<size_t>>& ImageGroups) const
{
	CSQLiteTable ImageTable(L"Images");
	ImageTable.AddColumn(L"ID", L"INTEGER PRIMARY KEY");
	ImageTable.AddColumn(L"Path", L"TEXT");
	ImageTable.AddColumn(L"ImageGroup", L"INTEGER");

	size_t nGroupIndex = nGroupStartIndex;
	for (const auto& GroupIndices : ImageGroups)
	{
		auto ImagesInGroup = CArrayUtilities::FilterByIndex(ImageFiles, GroupIndices);
		size_t i = 0;
		for (const auto& Image : ImagesInGroup)
		{
			auto sRelativeImagePath = CFileUtilities::GetRelativePath(sDatabasePath, Image);

			CSQLiteTableRow Row;
			Row.AddKeyValuePair(L"ID", *std::next(GroupIndices.begin(), i++));
			Row.AddKeyValuePair(L"Path", sRelativeImagePath);
			Row.AddKeyValuePair(L"ImageGroup", nGroupIndex);
			ImageTable.AddRow(Row);
		}
		nGroupIndex++;
	}

	CSQLiteDatabase Database(GetDatabasePath());
	ImageTable.InsertIntoDatabase(Database);
}

void CSNPDatasetResultLoader::ExportInputParameters(const CSNPDatasetParameters& Parameters) const
{
	CSQLiteDatabase Database(GetDatabasePath());
	CDIPLOMTable::From(CSNPDatasetParameters::TableName, Parameters).InsertIntoDatabase(Database);
}

void CSNPDatasetResultLoader::ExportOutputParameters(const CSNPDatasetOutputParameters& Parameters) const
{
	CSQLiteDatabase Database(GetDatabasePath());
	CDIPLOMTable::From(L"OutputParameters", Parameters).InsertIntoDatabase(Database);
}

void CSNPDatasetResultLoader::ExportPositioningSolution(const CDenseMatrix& Solution, size_t nGroup) const
{
	auto ExpandedSolution = ExpandSolutionWithGroupId(Solution, nGroup);
	CSQLiteDatabase Database(GetDatabasePath());
	CSQLiteTable::From(L"pSolution", ExpandedSolution).InsertIntoDatabase(Database, false);

	auto TextFileName = CFileUtilities::FullFile({ GetGroupRootFolder(nGroup) , GetGroupName(nGroup) + L"pSolution.txt" });
	ExpandedSolution.WriteMatrix(TextFileName);
}

void CSNPDatasetResultLoader::ExportPositioningSolutionToSpecialTable(const CDenseMatrix& Solution, size_t nGroup, std::wstring fileName) const
{
	auto ExpandedSolution = ExpandSolutionWithGroupId(Solution, nGroup);
	CSQLiteDatabase Database(GetDatabasePath());
	CSQLiteTable::From(fileName, ExpandedSolution).InsertIntoDatabase(Database, false);

	auto TextFileName = CFileUtilities::FullFile({ GetGroupRootFolder(nGroup) , GetGroupName(nGroup) + fileName + L".txt" });
	ExpandedSolution.WriteMatrix(TextFileName);
}

void CSNPDatasetResultLoader::ExportPositioningSolutionToSpecialTable(const CDenseMatrix& Solution, std::vector<int> groupIDPerRow, std::wstring fileName) const
{
	assert(Solution.Rows() == groupIDPerRow.size());

	CDenseMatrix ExtendedSolution(Solution.Rows(), Solution.Cols() + 1);
	ExtendedSolution.CopyRoi(0, 0, Solution, 0, 0, Solution.Rows(), Solution.Cols());

	for (size_t r = 0; r < ExtendedSolution.Rows(); r++)
	{
		ExtendedSolution.SetValueAt(r, ExtendedSolution.Cols() - 1, groupIDPerRow.at(r));
	}

	CSQLiteDatabase Database(GetDatabasePath());
	CSQLiteTable::From(fileName, ExtendedSolution).InsertIntoDatabase(Database, false);

	auto TextFileName = CFileUtilities::FullFile({ GetGroupRootFolder(1) , GetGroupName(1) + fileName + L".txt" });
	ExtendedSolution.WriteMatrix(TextFileName);
}

void CSNPDatasetResultLoader::ExportPositioningSolution(const std::vector<CDenseMatrix>& Solutions) const
{
	CSQLiteDatabase Database(GetDatabasePath());
	Database.DropTableIfExists(L"pSolution");

	size_t nGroupID = nGroupStartIndex;
	for (const auto& GroupSolution : Solutions)
		ExportPositioningSolution(GroupSolution, nGroupID++);
}

void CSNPDatasetResultLoader::ExportStlMosaics(const std::vector<StlImage<float>*>& MosaicImages) const
{
	CSQLiteDatabase Database(GetDatabasePath());
	CDIPLOMTable::From(CMosaicImageProperties::TableName, std::vector<CMosaicImageProperties>()).InsertIntoDatabaseWithoutContent(Database);
	size_t nGroupID = nGroupStartIndex;
	for (const auto& Mosaic : MosaicImages)
		ExportStlMosaic(*Mosaic, nGroupID++);
}

void CSNPDatasetResultLoader::ExportStlMosaic(const StlImage<float>& MosaicImage, size_t nGroupID) const
{
	auto FileName = CFileUtilities::FullFile({ sRootPath,sSeriesName + L"_m" + std::to_wstring(nGroupID) + L".tif" });
	StlImage<uint8_t> tmp;
	MosaicImage.ExportAs8bitUnsigned(FileName.c_str());
	auto sRelativePath = CFileUtilities::GetRelativePath(sDatabasePath, FileName);

	CMosaicImageProperties Properties;
	Properties.MosaicImageSize = MosaicImage.GetSize();
	Properties.nDefinedMosaicArea = MosaicImage.NumberOfNonzeroPixels();
	Properties.nGroupID = nGroupID;
	Properties.sFilePath = sRelativePath;

	CSQLiteDatabase Database(GetDatabasePath());
	Database.InsertInto(CMosaicImageProperties::TableName, CDIPLOMTableRow::From(Properties));
}

void CSNPDatasetResultLoader::ExportResults(const std::vector<CSNPCompositingResult*>& results) const
{
	CSQLiteDatabase Database(GetDatabasePath());
	CDIPLOMTable::From(CMosaicImageProperties::TableName, std::vector<CMosaicImageProperties>()).InsertIntoDatabaseWithoutContent(Database);
	size_t nGroupID = nGroupStartIndex;
	for (const auto result : results)
		ExportResult(result, nGroupID++);
}

void CSNPDatasetResultLoader::ExportResult(const CSNPCompositingResult* result, size_t nGroupID) const
{
	if (std::holds_alternative<C3DCloud>(*result)) {
		Export3DCloud(std::get<C3DCloud>(*result), nGroupID);
	}
	else if (std::holds_alternative<C3DBuffer<float>>(*result)) {
		ExportVoxelBuffer(std::get<C3DBuffer<float>>(*result), nGroupID);
	}
	else if (std::holds_alternative<StlImage<float>>(*result)) {
		ExportStlMosaic(std::get<StlImage<float>>(*result), nGroupID);
	}
}

void CSNPDatasetResultLoader::Export3DCloud(const C3DCloud& result, size_t nGroupID) const
{
	auto FileName = CFileUtilities::FullFile({ sRootPath,sSeriesName + L"_m" + std::to_wstring(nGroupID) + L".ply" });
	result.savePLY(std::wstring_view(FileName));
	auto sRelativePath = CFileUtilities::GetRelativePath(sDatabasePath, FileName);

	/*CMosaicImageProperties Properties;
	Properties.MosaicImageSize = MosaicImage.GetSize();
	Properties.nDefinedMosaicArea = MosaicImage.NumberOfNonzeroPixels();
	Properties.nGroupID = nGroupID;
	Properties.sFilePath = sRelativePath;

	CSQLiteDatabase Database(GetDatabasePath());
	Database.InsertInto(CMosaicImageProperties::TableName, CDIPLOMTableRow::From(Properties));*/
}

void CSNPDatasetResultLoader::ExportVoxelBuffer(const C3DBuffer<float>& result, size_t nGroupID) const
{
	auto FolderName = CFileUtilities::FullFile({ sRootPath,sSeriesName + L"_m" + std::to_wstring(nGroupID) });
	result.saveAsImageSeries(FolderName);
}

void CSNPDatasetResultLoader::ExportMotionCorrectedImages(const std::vector<CSNPGroupResult>& groupResults) const
{
	size_t nGroupID = nGroupStartIndex;
	for (const CSNPGroupResult& groupResult : groupResults)
		ExportMotionCorrectedImages(groupResult, nGroupID++);
}

void CSNPDatasetResultLoader::ExportMotionCorrectedImages(const CSNPGroupResult& groupResult, size_t nGroupID) const
{
	assert(groupResult.UsedImagePaths.size() == groupResult.MotionCorrectedImages.size());

	auto FolderName = GetGroupRootFolder(nGroupID);
	CFileUtilities::MakeDirectory(std::wstring(FolderName));

	size_t n = groupResult.MotionCorrectedImages.size();
	if (n < 1)
		return;

	ptrdiff_t sxmax = 0;
	ptrdiff_t symax = 0;
	for (size_t i = 0; i < n; i++)
	{
		ptrdiff_t sx = groupResult.MotionCorrectedImages[i].TopLeftCorner.x + groupResult.MotionCorrectedImages[i].pUndistoredImage->GetSize().x;
		ptrdiff_t sy = groupResult.MotionCorrectedImages[i].TopLeftCorner.y + groupResult.MotionCorrectedImages[i].pUndistoredImage->GetSize().y;
		sxmax = std::max(sx, sxmax);
		symax = std::max(sy, symax);
	}

	long long border = groupResult.Parameters.nBorder;

	StlImage<float> image;
	image.Alloc({ sxmax + border, symax + border });

	for (size_t i = 0; i < n; i++)
	{
		image.Clear();
		ptrdiff_t px = groupResult.MotionCorrectedImages[i].TopLeftCorner.x;
		ptrdiff_t py = groupResult.MotionCorrectedImages[i].TopLeftCorner.y;
		ptrdiff_t sx = groupResult.MotionCorrectedImages[i].pUndistoredImage->GetSize().x;
		ptrdiff_t sy = groupResult.MotionCorrectedImages[i].pUndistoredImage->GetSize().y;

		image.CopyAtOffset({ px, py }, *(groupResult.MotionCorrectedImages[i].pUndistoredImage));

		auto FileName = CFileUtilities::GetFileName(groupResult.UsedImagePaths[i]);
		auto FilePath = CFileUtilities::FullFile({ FolderName, FileName + L"_mc.tif" });
		image.ExportAs8bitUnsigned(FilePath.c_str());
	}

	image.Free();
}

void CSNPDatasetResultLoader::CreateFolderStructure(size_t nImageGroupCount) const
{
	size_t nGroupID = nGroupStartIndex;

	for (size_t i = 0; i < nImageGroupCount; i++)
		CFileUtilities::MakeDirectory(GetGroupRootFolder(nGroupID++), true);
}

CDenseMatrix CSNPDatasetResultLoader::ExpandSolutionWithGroupId(const CDenseMatrix& Solution, size_t nGroup)
{
	CDenseMatrix ExtendedSolution(Solution.Rows(), Solution.Cols() + 1);
	ExtendedSolution.CopyRoi(0, 0, Solution, 0, 0, Solution.Rows(), Solution.Cols());

	CDenseVector GroupVector(ExtendedSolution.Rows());
	GroupVector.Fill(static_cast<double>(nGroup));

	ExtendedSolution.SetCol(GroupVector, ExtendedSolution.Cols() - 1);

	return ExtendedSolution;
}

std::wstring CSNPDatasetResultLoader::GetGroupRootFolder(size_t nGroupID) const
{
	return CFileUtilities::FullFile({ sRootPath, GetGroupName(nGroupID) });
}

std::wstring CSNPDatasetResultLoader::GetGroupName(size_t nGroupID) const
{
	return sSeriesName + L"_m" + std::to_wstring(nGroupID);
}

std::vector<CDenseMatrix> CSNPDatasetResultLoader::ImportSolutions() const
{
	CSQLiteDatabase Database(GetDatabasePath());
	auto nImageGroupCount = Database.SelectDistinct(L"pSolution", { CSQLiteTable::GetDefaultColumnName(2) }).size();

	std::vector<CDenseMatrix> Result;

	size_t nGroupNumber = nGroupStartIndex;
	for (size_t i = 0; i < nImageGroupCount; i++)
	{
		Result.push_back(ImportSolution(nGroupNumber++));
	}

	return Result;
}

CDenseMatrix CSNPDatasetResultLoader::ImportSolution(size_t nGroupID) const
{
	CSQLiteDatabase Database(GetDatabasePath());
	auto ResultTable = Database.SelectAll(L"pSolution", { CSQLiteTable::GetDefaultColumnName(0), CSQLiteTable::GetDefaultColumnName(1) }, CSQLiteTable::GetDefaultColumnName(2) + L"=" + std::to_wstring(nGroupID));
	return ResultTable.Convert<CDenseMatrix>();
}
CSNPDatasetOutputParameters CSNPDatasetResultLoader::ImportOutputParameters() const
{
	CSQLiteDatabase Database(GetDatabasePath());
	return Database.SelectAll(CSNPDatasetOutputParameters::TableName).Convert<CSNPDatasetOutputParameters>();
}

CImageRegistrationResult CSNPDatasetResultLoader::ImportRegistrationResults() const
{
	CSQLiteDatabase Database(GetDatabasePath());
	return CRegistrationResultLoader::LoadFromSQLite(Database);
}

std::vector<std::list<size_t>> CSNPDatasetResultLoader::ImportImageGroups() const
{
	CSQLiteDatabase Database(GetDatabasePath());
	auto RowRepresentingAllGroupsByIndex = Database.SelectAll(L"Images", { L"ImageGroup" }).GetColumn(0);

	std::vector<std::list<size_t>> ImageGroups;


	for (size_t i = 0; i < RowRepresentingAllGroupsByIndex.size(); i++)
	{
		auto Val = RowRepresentingAllGroupsByIndex.Get<size_t>(i);

		while (ImageGroups.size() < Val)
			ImageGroups.push_back({});

		ImageGroups.at(Val - 1).push_back(i);
	}
	return ImageGroups;
}

CSNPDatasetParameters CSNPDatasetResultLoader::ImportInputParameters() const
{
	CSQLiteDatabase Database(GetDatabasePath());
	return Database.SelectAll(CSNPDatasetParameters::TableName).Convert<CSNPDatasetParameters>();
}

std::vector<std::wstring> CSNPDatasetResultLoader::ImportImageList() const
{
	CSQLiteDatabase Database(GetDatabasePath());
	return Database.SelectAll(L"Images", { L"Path" }).Convert<std::vector<std::wstring>>();
}

std::vector<std::wstring> CSNPDatasetResultLoader::ImportImageListAndNormalizeRelativePaths() const
{
	auto ImageList = ImportImageList();
	decltype(ImageList) NormalizedImageList;
	for (const auto& Image : ImageList)
	{
		auto sNormalizedPath = CFileUtilities::GetAbsolutePath(GetDatabasePath(), Image);
		NormalizedImageList.push_back(sNormalizedPath);
	}
	return NormalizedImageList;
}

std::vector<CMosaicImageProperties> CSNPDatasetResultLoader::ImportMosaicImageProperties() const
{
	CSQLiteDatabase Database(GetDatabasePath());
	auto nImageGroupCount = Database.SelectFirst(L"OutputParameters", { L"Value" }, L"Name='DetectedImageGroups'").Get<size_t>(0);
	std::vector<CMosaicImageProperties> Result;

	size_t nGroupNumber = nGroupStartIndex;
	for (size_t i = 0; i < nImageGroupCount; i++)
	{
		Result.push_back(ImportMosaicImageProperties(nGroupNumber++));
	}

	return Result;
}

CMosaicImageProperties CSNPDatasetResultLoader::ImportMosaicImageProperties(size_t nGroupId) const
{
	CSQLiteDatabase Database(GetDatabasePath());
	auto Table = Database.SelectAll(L"MosaicImageProperties", { L"*" }, L"GroupID=" + std::to_wstring(nGroupId));

	if (Table.size() != 1)
		throw CSNPFusionException(L"Could not load MosaicImageProperties with GroupID= " + std::to_wstring(nGroupId));

	return Table.GetRow(0).Convert<CMosaicImageProperties>();
}

std::vector<std::unique_ptr<StlImage<float>>> CSNPDatasetResultLoader::ImportMosaicImages() const
{
	std::vector<std::unique_ptr<StlImage<float>>> out;

	auto MosaicImageProperties = ImportMosaicImageProperties();
	for (const auto& Property : MosaicImageProperties)
	{
		auto sAbsolutePath = CFileUtilities::GetAbsolutePath(sDatabasePath, Property.sFilePath);
		auto pIm = CImageLoader::LoadImageFromFile(sAbsolutePath);
		std::unique_ptr<StlImage<float>> uniquePtr(pIm);
		out.push_back(move(uniquePtr));
	}
	return out;
}

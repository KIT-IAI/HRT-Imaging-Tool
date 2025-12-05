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
#include "SNPDatasetLoader.h"



CSNPFusionDataset CSNPDatasetLoader::FromMultiPageTifFile(const std::wstring& datasetFilePath, bool exportImages, const std::wstring& exportPath)
{
	if (datasetFilePath.empty() || (CFileUtilities::GetExtension(datasetFilePath) != L"tif" && CFileUtilities::GetExtension(datasetFilePath) != L"tiff"))
		throw std::runtime_error("Invalid Inputfile.");

	std::vector<StlImage<float>> images;
	if (!StlImage<float>::ImportFromMultiPageTiffFile(datasetFilePath, images))
		throw std::runtime_error("Extraction of image sequence from multi-page TIFF file failed.");

	// we need a vector of StlImage pointers further down; the dataset object will
	// take possession of the pointers
	std::vector<StlImage<float>*> imagePointers;
	for (auto& img : images)
	{
		imagePointers.push_back(new StlImage<float>(img));
	}

	std::wstring imageExportFolderPath = exportPath.empty() ? DefaultImageExportFolderPath(datasetFilePath) : exportPath;
	std::wstring datasetName(CFileUtilities::GetFileName(datasetFilePath));

	if (exportImages)
	{
		CFileUtilities::MakeDirectory(imageExportFolderPath, true);
	}

	std::vector<std::wstring> extractedImageFiles;
	for (size_t i = 0; i < images.size(); i++)
	{
		auto formatted = boost::wformat(L"%s_%.5d.tif") % datasetName % i;
		std::wstring extractedImageFileName = formatted.str();
		std::wstring extractedImageFilePath = CFileUtilities::FullFile({ imageExportFolderPath, extractedImageFileName });
		if (exportImages)
		{
			images[i].ExportAs8bitUnsigned(extractedImageFilePath);
		}
		extractedImageFiles.push_back(extractedImageFilePath);
	}

	assert(!extractedImageFiles.empty());

	CSNPFusionDataset Dataset;
	Dataset.Name = datasetName;
	Dataset.SetImageFileList(extractedImageFiles);
	Dataset.SetVolImageList(imagePointers);

	return Dataset;
}

vector<CSNPFusionDataset> CSNPDatasetLoader::FromTextFile(wstring sPath)
{
	auto fileByLine = CFileUtilities::ByLine(sPath);

	vector<vector<wstring>> ListOfFileLists;
	vector<wstring> ListOfDatasetNames;

	for (auto& line : fileByLine)
	{
		if (line.empty())			// Empty Line
			continue;				// ignore empty lines
		else if (line[0] == L'%')	// Dataset Header
		{
			ListOfDatasetNames.push_back(CStringUtilities::Trim(line.substr(1)));
			ListOfFileLists.push_back({});
		}
		else						// File Path
		{
			if (ListOfFileLists.empty())
				throw std::runtime_error("No dataset header was found.");

			line = CFileUtilities::GetAbsolutePath(sPath, line);

			ListOfFileLists.back().push_back(line);
		}
	}

	vector<CSNPFusionDataset> Datasets(ListOfDatasetNames.size());
	for (size_t i = 0; i < ListOfDatasetNames.size(); i++)
	{
		Datasets[i].Name = ListOfDatasetNames[i];
		Datasets[i].SetImageFileList(ListOfFileLists[i]);

		if (ListOfFileLists[i].empty())
			throw std::runtime_error("Empty dataset detected.");
	}
	return Datasets;
}

vector<CSNPFusionDataset> CSNPDatasetLoader::FromAnyFile(wstring sPath)
{
	auto Extension = CFileUtilities::GetExtension(sPath);
	if (Extension == L"txt")
		return FromTextFile(sPath);
	if (Extension == L"sql" || Extension == L"sqlite")
		return { FromSQLiteDatabase(sPath) };
	if (Extension == L"tif" || Extension == L"tiff")
		return { FromMultiPageTifFile(sPath) };

	throw std::runtime_error("Unknown file format.");
}

CSNPFusionDataset CSNPDatasetLoader::FromSQLiteDatabase(CSQLiteDatabase& Database)
{
	auto Parameters = Database.SelectAll(L"Parameters").Convert<CSNPDatasetParameters>();
	auto Images = ReadImagesFromSQLite(Database);

	CSNPFusionDataset Dataset;
	Dataset.SetImageFileList(Images);
	Dataset.SetParameters(Parameters);
	Dataset.Name = Database.GetName();

	return Dataset;
}

CSNPFusionDataset CSNPDatasetLoader::FromSQLiteDatabase(wstring sPath)
{
	CSQLiteDatabase Database(sPath);
	return FromSQLiteDatabase(Database);
}

vector<wstring> CSNPDatasetLoader::ReadImagesFromSQLite(CSQLiteDatabase& Database)
{
	auto resultRows = Database.SelectAll(L"Images", { L"Path" }, L"1", L"ID");

	vector<wstring> ImageFiles;
	for (const auto& row : resultRows)
	{
		auto sRelativePath = row.Get<wstring>(0);
		auto sAbsolutePath = CFileUtilities::GetAbsolutePath(Database.GetPath(), sRelativePath);
		ImageFiles.push_back(sAbsolutePath);
	}
	return ImageFiles;
}

wstring CSNPDatasetLoader::DefaultImageExportFolderPath(const std::wstring& datasetFilePath)
{
	std::wstring baseFolderPath(CFileUtilities::GetParentDirectory(datasetFilePath));
	std::wstring datasetName(CFileUtilities::GetFileName(datasetFilePath));

	return CFileUtilities::FullFile({ baseFolderPath, datasetName, datasetName + L"_snp" });
}

wstring CSNPDatasetLoader::DefaultTissueClassFilePath(const std::wstring& datasetFilePath)
{
	std::wstring baseFolderPath(CFileUtilities::GetParentDirectory(datasetFilePath));
	std::wstring datasetName(CFileUtilities::GetFileName(datasetFilePath));
	return CFileUtilities::FullFile({ baseFolderPath, datasetName + L"_results.csv" });
}

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
#include "ImageLoader.h"



CImageLoader::CImageLoader()
{

}

void CImageLoader::DoWork(vector<wstring>& param, vector<StlImage<float>*>& result)
{
	return LoadImages(param, result);
}
void CImageLoader::GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits)
{
	nTotalWorkUnits = m_nImageCount;
	nCompletedWorkUnits = m_nLoadedImages;
}
StlImage<float>* CImageLoader::LoadImageFromFile(const wstring& fileName)
{
	StlImage<float>* Image = new StlImage<float>();
	Image->LoadConvert(fileName);
	return Image;
}
void CImageLoader::LoadImages(const vector<wstring>& ImageFiles, vector<StlImage<float>*>& result)
{
	m_nImageCount = ImageFiles.size();
	for (const auto& fileName : ImageFiles)
	{
		if (m_bIsCanceled)
			break;

		auto Image = LoadImageFromFile(fileName);
		result.push_back(Image);

		if (++m_nLoadedImages % 10 == 0)
			ReportProgress();
	}
	ReportProgress();
}
vector<StlImage<float>*> CImageLoader::LoadImages(const vector<wstring>& ImageFiles)
{
	vector<StlImage<float>*> out;
	CImageLoader loader;
	loader.LoadImages(ImageFiles, out);
	return out;
}

vector<StlImage<float>*> CImageLoader::LoadImagesFromFolder(const wstring& folderPath)
{
	return LoadImagesFromFolder(folderPath, 0);
}
vector<StlImage<float>*> CImageLoader::LoadImagesFromFolder(const wstring& folderPath, size_t nCount)
{
	return LoadImagesFromFolder(folderPath, CRegex(L".*"), nCount);
}

vector<StlImage<float>*> CImageLoader::LoadImagesFromFolder(const wstring& folderPath, const  CRegex& regex)
{
	return LoadImagesFromFolder(folderPath, regex, 0);
}
vector<StlImage<float>*> CImageLoader::LoadImagesFromFolder(const wstring& folderPath, const  CRegex& regex, size_t nCount)
{
	vector<StlImage<float>*> resultImages;
	auto filePaths = CFileUtilities::GetFilesInDirectory(folderPath, regex);

	if (nCount > 0 && nCount < filePaths.size())
	{
		decltype(filePaths) certainAmountOfPaths;
		std::copy_n(filePaths.begin(), nCount, std::back_inserter(certainAmountOfPaths));
		filePaths = certainAmountOfPaths;
	}

	CImageLoader loader;
	loader.LoadImages(filePaths, resultImages);

	return resultImages;
}

vector<StlImage<float>*> CImageLoader::Load(const vector<wstring>& FilePaths)
{
	vector<StlImage<float>*> resultImages;
	CImageLoader loader;
	loader.LoadImages(FilePaths, resultImages);
	return resultImages;
}
vector<StlImage<float>*> CImageLoader::LoadImagesFromSQLite(CSQLiteDatabase& Database, const wstring& sTable, const wstring sColumn)
{
	auto ResultTable = Database.SelectAll(sTable, { sColumn });
	vector<wstring> filePaths;
	for (const auto& Row : ResultTable)
	{
		assert(Row.size() == 1);
		filePaths.push_back(Row.Get <wstring>(0));
	}
	return Load(filePaths);
}

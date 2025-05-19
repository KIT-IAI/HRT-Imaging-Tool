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
#include "Regex.h"
#include "SQLiteDatabase.h"
#include "StlImage.h"


class CImageLoader : public IComputable<std::vector<std::wstring>, std::vector<StlImage<float>*>&>
{
public:
	explicit CImageLoader();
	~CImageLoader() = default;

	static StlImage<float>* LoadImageFromFile(const std::wstring&);
	void						LoadImages(const std::vector<std::wstring>& ImageFiles, std::vector<StlImage<float>*>& result);
	static std::vector<StlImage<float>*> LoadImages(const std::vector<std::wstring>& ImageFiles);

	static std::vector<StlImage<float>*> LoadImagesFromFolder(const std::wstring& folderPath);
	static std::vector<StlImage<float>*> LoadImagesFromFolder(const std::wstring& folderPath, const CRegex& regex);
	static std::vector<StlImage<float>*> LoadImagesFromFolder(const std::wstring& folderPath, size_t nCount);
	static std::vector<StlImage<float>*> LoadImagesFromFolder(const std::wstring& folderPath, const CRegex& regex, size_t nCount);

	static std::vector<StlImage<float>*> LoadImagesFromSQLite(CSQLiteDatabase& Database, const std::wstring& sTable, const std::wstring sColumn);

private:
	static std::vector<StlImage<float>*> Load(const std::vector<std::wstring>& FilePaths);
protected:
	void DoWork(std::vector<std::wstring>& param, std::vector<StlImage<float>*>& result) override;
	void GetWorkUnits(size_t& nCompletedWorkUnits, size_t& nTotalWorkUnits) override;

private:
	size_t m_nImageCount = 1;
	size_t m_nLoadedImages = 0;
};

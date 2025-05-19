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

#include "SNPFusionDataset.h"
#include "SQLiteDatabase.h"

class CSNPDatasetLoader
{
public:
	static CSNPFusionDataset FromMultiPageTifFile(const std::wstring& datasetFilePath, bool exportImages = true, const std::wstring& exportPath = L"");
	static std::vector<CSNPFusionDataset> FromTextFile(std::wstring sPath);
	static std::vector<CSNPFusionDataset> FromAnyFile(std::wstring sPath);
	static CSNPFusionDataset FromSQLiteDatabase(std::wstring sPath);
	static CSNPFusionDataset FromSQLiteDatabase(CSQLiteDatabase& Database);

	static std::wstring DefaultImageExportFolderPath(const std::wstring& datasetFilePath);
	static std::wstring DefaultTissueClassFilePath(const std::wstring& datasetFilePath);

private:

	static std::vector<std::wstring> ReadImagesFromSQLite(CSQLiteDatabase& Database);

};

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

#include <filesystem>
#include <vector>

#include "Regex.h"

class CFileUtilities
{

public:

	CFileUtilities() = delete;
	virtual ~CFileUtilities() = delete;

public:

	static bool MakeDirectory(const std::wstring& sPath, bool bRecursive = false);
	static bool MakeDirectory(const std::string& sPath, bool bRecursive = false);
	static bool DeleteDirectory(const std::wstring& sPath, bool bOnlyIfEmptry = true);

	static bool PathExists(const std::string& path);
	static bool PathExists(const std::wstring& path);
	static bool PathExists(const std::wstring_view path);
	static bool FileExists(const std::wstring& file);
	static bool FileExists(const std::string& file);

	static bool IsUNCPath(const std::wstring& sPath);
	static bool IsDrivePath(const std::wstring& sPath);

	static size_t CountNumberOfLines(const std::wstring& sPath, bool bSkipEmptyLines = false);
	static size_t CountNumberOfColumns(const std::wstring& sPath, const std::wstring& sDelimiter = L"\t");

	static void UNCPathComponents(const std::wstring& sPath, std::wstring& sServer, std::wstring& sShare, std::wstring& sFilePath);
	static void DrivePathComponents(const std::wstring& sPath, std::wstring& sDrive, std::wstring& sFilepath);

	static std::wstring CommonParentDirectory(const std::wstring& sPath1, const std::wstring& sPath2);
	static std::wstring GetParentDirectory(const std::wstring& File);

	static bool IsAbsolute(const std::wstring& sPath);
	static bool IsRelative(const std::wstring& sPath);

	static std::wstring GetAbsolutePath(const std::wstring& sReferencePath, const std::wstring& sTargetPath);
	static std::wstring GetRelativePath(const std::wstring& sReferencePath, const std::wstring& sTargetPath);

	static void DeleteOldFiles(const std::wstring& directory, int OlderThenDays);
	static void DeleteOldFiles(const std::wstring& directory, CRegex fileNamePattern, int OlderThenDays);

	static std::vector<std::wstring> GetFilesInDirectory(const std::wstring& directory);
	static std::vector<std::wstring> GetFilesInDirectory(const std::wstring& directory, CRegex fileNamePattern);
	static std::vector<std::wstring> GetDirectorysInDirectory(const std::wstring& directory);
	static std::vector<std::wstring> GetDirectorysInDirectory(const std::wstring& directory, CRegex DirectoryPattern);

	static std::wstring FullFile(const std::vector<std::wstring>& FileParts);

	static std::vector<std::wstring> FileParts(const std::wstring& Path);

	static std::wstring GetExtension(const std::wstring& fileName);
	static std::wstring GetFileName(const std::wstring& fileName);

	static std::vector<std::wstring> ByLine(const std::wstring& fileName);

	static std::wstring NormalizeSlashes(const std::wstring& path);

	static void CopyFile(const std::wstring& source, const std::wstring& destination, bool bCreateDestinationFolders = false);

	static std::filesystem::path GetProgramFullPath();
	static std::filesystem::path GetProgramFolder();

};

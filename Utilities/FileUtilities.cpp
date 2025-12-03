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
#include "FileUtilities.h"

#include <chrono>
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>

#include "StringUtilities.h"



bool CFileUtilities::MakeDirectory(const std::wstring& sPath, bool bRecursive /*= false*/)
{
	// Hier nur grobe Funktionalität, Überprüfungen aus CPathDialog nehmen und hier einfügen!

	if (sPath.empty())
	{
		return false;
	}

	if (bRecursive)
	{
		std::wstring ParentDirectory = GetParentDirectory(sPath);
		if (!PathExists(ParentDirectory))
			MakeDirectory(ParentDirectory, true);
	}

	return std::filesystem::create_directory(sPath);
}

bool CFileUtilities::MakeDirectory(const std::string& sPath, bool bRecursive /*= false*/)
{
	return MakeDirectory(CStringUtilities::ConvertToStdWstring(sPath), bRecursive);
}

bool CFileUtilities::DeleteDirectory(const std::wstring& sPath, bool bOnlyIfEmpty)
{
	if (bOnlyIfEmpty)
	{
		auto Files = GetFilesInDirectory(sPath);
		if (!Files.empty())
			return false;
	}
	try
	{
		return boost::filesystem::remove_all(sPath) > 0;
	}
	catch (boost::filesystem::filesystem_error ex)
	{
		return false;
	}
}

bool CFileUtilities::PathExists(const std::wstring& path)
{
	return boost::filesystem::exists(path) && boost::filesystem::is_directory(path);
}

bool CFileUtilities::PathExists(const std::string& path)
{
	return boost::filesystem::exists(path) && boost::filesystem::is_directory(path);
}

bool CFileUtilities::PathExists(const std::wstring_view path)
{
	return PathExists(std::wstring(path));
}

bool CFileUtilities::FileExists(const std::wstring& file)
{
	return boost::filesystem::exists(file) && boost::filesystem::is_regular_file(file);
}

bool CFileUtilities::FileExists(const std::string& file)
{
	return boost::filesystem::exists(file) && boost::filesystem::is_regular_file(file);
}

/**	\brief Gibt an, ob die Zeichenkette \a sPath ein gültiger UNC-Pfad ist.
 *
 *	Ein UNC-konformer vollständiger Dateipfad hat die Form
 *	_T("\\\\\<Server\>\\\<Freigabe\>\\\<Dateipfad\>"). Diese
 *	Methode prüft aber nur, ob der Pfad mit genau zwei Backslashes beginnt.
 *
 *	\return \c true, wenn \a sPath ein gültiger UNC-Pfad ist.
 *	\return \c false, wenn \a sPath kein gültiger UNC-Pfad ist.
 *
 *	\param[in] sPath Eine Zeichenkette.
 *
 *	\see UNCPathComponents()
 */
bool CFileUtilities::IsUNCPath(const std::wstring& sPath)
{
	if (sPath.substr(0, 2).compare(L"\\\\") != 0)
	{
		// path doesn't start with two backslashes
		return false;
	}
	else if (sPath.substr(0, 3).compare(L"\\\\\\") == 0)
	{
		// path starts with three backslashes
		return false;
	}
	else
	{
		return true;
	}
}

/**	\brief Gibt an, ob die Zeichenkette \a sPath ein gültiger Laufwerkspfad
 *	ist.
 *
 *	Ein vollständiger Dateipfad auf einem Laufwerk hat die Form
 *	_T("\<Laufwerk\>:\\\<Dateipfad\>"), wobei die Bezeichnung des Laufwerks aus
 *	genau einem Buchstaben bestehen muss. Diese Methode prüft nur, ob der
 *	Pfad mit einem Buchstaben und einem Doppelpunkt beginnt.
 *
 *	\return \c true, wenn \a sPath ein gültiger Laufwerkspfad ist.
 *	\return \c false, wenn \a sPath kein gültiger Laufwerkspfad ist.
 *
 *	\param[in] sPath Eine Zeichenkette.
 *
 *	\see DrivePathComponents()
 */
bool CFileUtilities::IsDrivePath(const std::wstring& sPath)
{
	if (std::toupper(sPath[0], std::locale()) < L'A')
	{
		// path doesn't start with a drive letter
		return false;
	}
	else if (std::toupper(sPath[0], std::locale()) > L'Z')
	{
		// path doesn't start with a drive letter
		return false;
	}
	else if (sPath[1] != L':')
	{
		// path doesn't contain a colon after the drive letter
		return false;
	}
	else
	{
		return true;
	}
}

/**	\brief Liefert die Anzahl der Zeilen oder der nicht-leeren Zeilen in der
 *	Datei mit dem Pfad \a sPath.
 *
 *	\return Die ermittelte Zeilenanzahl.
 *
 *	\param[in] sPath Der Dateipfad.
 *	\param[in] bSkipEmptyLines Gibt an, ob leere Zeilen beim Zählen ignoriert
 *		werden.
 *
 *	\author Lorenzo Toso
 *	\author Stephan Allgeier
 */
size_t CFileUtilities::CountNumberOfLines(const std::wstring& sPath, bool bSkipEmptyLines /*= false*/)
{
	size_t rows = 0;

	std::wifstream file;
	std::wstring line;

	file.open(std::filesystem::path(sPath));

	if (!file.good())
	{
		throw std::ios_base::failure("Could not open file");
	}

	while (file.good())
	{
		std::getline(file, line);

		if (bSkipEmptyLines && line.empty())
		{
			continue;
		}

		rows++;
	}

	return rows;
}

/**	\brief Liefert die Anzahl der durch eines der Zeichen in \a sDelimiter
 *	separierten Spalten in der Datei mit dem Pfad \a sPath.
 *
 *	Es wird von einer tabellarischen Dateistruktur ausgegangen, in der jede
 *	Zeile die gleiche Anzahl an durch Trennzeichen separierte Elemente
 *	enthält. Diese Anzahl definiert die Anzahl der Spalten der Datei.
 *
 *	\note Die Spaltenanzahl wird ausschließlich anhand des Inhalts der ersten
 *	Zeile der Datei ermittelt. Es findet keinerlei Prüfung der Inhalte der
 *	verbleibenden Zeilen statt, insbesondere also keine Prüfung der
 *	angenommenen tabellarischen Struktur der Datei.
 *
 *	\note Mehrere unmittelbar aufeinander folgende Trennzeichen werden als
 *	ein einzelnes Trennzeichen interpretiert.
 *
 *	\return Die ermittelte Spaltenanzahl.
 *
 *	\param[in] sPath Der Dateipfad.
 *	\param[in] sDelimiter Eine Zeichenkette, die die Menge der Trennzeichen
 *		zwischen einzelnen Elementen definiert.
 *
 *	\author Lorenzo Toso
 *	\author Stephan Allgeier
 */
size_t CFileUtilities::CountNumberOfColumns(const std::wstring& sPath, const std::wstring& sDelimiter /*= L"\t"*/)
{
	typedef std::wstring string_t;
	typedef boost::char_separator<string_t::value_type> tokenizerfunc_t;
	typedef boost::tokenizer<tokenizerfunc_t, string_t::const_iterator, string_t> tokenizer_t;

	tokenizerfunc_t delimiters(sDelimiter.c_str());

	size_t cols = 0;

	std::wifstream file;
	std::wstring line;

	file.open(std::filesystem::path(sPath));

	if (!file.good())
	{
		throw std::ios_base::failure("Could not open file");
	}

	std::getline(file, line);

	tokenizer_t tokens(line, delimiters);
	for (auto token : tokens)
	{
		cols++;
	}

	return cols;
}

/**	\brief Zerlegt einen UNC-Pfad in Komponenten.
 *
 *	Ein vollständiger UNC-Pfad besteht aus der Angabe eines Servers, einer
 *	Freigabe und eines Dateipfads, in der Form
 *	_T("\\\\\<Server\>\\\<Freigabe\>\\\<Dateipfad\>").
 *
 *	\param[in] sPath Eine Zeichenkette.
 *	\param[out] sServer Der Servername.
 *	\param[out] sShare Der Freigabename.
 *	\param[out] sFilepath Der Dateipfad.
 *
 *	\see IsUNCPath()
 */
void CFileUtilities::UNCPathComponents(const std::wstring& sPath, std::wstring& sServer, std::wstring& sShare, std::wstring& sFilepath)
{
	sServer.clear();
	sShare.clear();
	sFilepath.clear();

	if (!IsUNCPath(sPath))
	{
		return;
	}

	auto fileParts = FileParts(sPath);

	size_t nBackslashIndex = 2;
	size_t nNextBackslashIndex = sPath.find(L'\\', nBackslashIndex);

	if (nNextBackslashIndex == std::wstring::npos)
	{
		sServer = sPath.substr(nBackslashIndex);
		return;
	}

	sServer = sPath.substr(nBackslashIndex, nNextBackslashIndex - nBackslashIndex);

	nBackslashIndex = nNextBackslashIndex + 1;
	nNextBackslashIndex = sPath.find(L'\\', nBackslashIndex);

	if (nNextBackslashIndex == std::wstring::npos)
	{
		sShare = sPath.substr(nBackslashIndex);
		return;
	}

	sShare = sPath.substr(nBackslashIndex, nNextBackslashIndex - nBackslashIndex);

	sFilepath = sPath.substr(nNextBackslashIndex + 1);
}

/**	\brief Zerlegt einen Laufwerkspfad in Komponenten.
 *
 *	Ein vollständiger Laufwerkspfad besteht aus der Angabe eines Laufwerks
 *	und eines Dateipfads, in der Form
 *	_T("\<Laufwerk\>:\\\<Dateipfad\>").
 *
 *	\param[in] sPath Eine Zeichenkette.
 *	\param[out] sDrive Der Laufwerksname (einschließlich dem Doppelpunkt).
 *	\param[out] sFilepath Der Dateipfad.
 *
 *	\see IsDrivePath()
 */
void CFileUtilities::DrivePathComponents(const std::wstring& sPath, std::wstring& sDrive, std::wstring& sFilepath)
{
	sDrive.clear();
	sFilepath.clear();

	if (!IsDrivePath(sPath))
	{
		return;
	}

	size_t nBackslashIndex = 0;
	size_t nNextBackslashIndex = sPath.find(L'\\', nBackslashIndex);

	if (nNextBackslashIndex == std::wstring::npos)
	{
		sDrive = sPath.substr(nBackslashIndex);
		return;
	}

	sDrive = sPath.substr(nBackslashIndex, nNextBackslashIndex - nBackslashIndex);

	sFilepath = sPath.substr(nNextBackslashIndex + 1);
}

/**	\brief Bestimmt das tiefste gemeinsame Verzeichnis der Dateipfade
 *	\a sFilepath1 und \a sFilepath2.
 *
 *	\return Der Verzeichnispfad des tiefsten gemeinsamen Verzeichnisses der
 *		Dateipfade \a sFilepath1 und \a sFilepath2.
 *	\return Eine leere Zeichenkette, wenn die Dateipfade kein gemeinsames
 *		Verzeichnis besitzen.
 *
 *	\param[in] sFilepath1 Ein Dateipfad.
 *	\param[in] sFilepath2 Ein Dateipfad.
 */
std::wstring CFileUtilities::CommonParentDirectory(const std::wstring& sFilepath1, const std::wstring& sFilepath2)
{
	std::wstring sCommonDirectory;
	sCommonDirectory = CStringUtilities::CommonPrefix(sFilepath1.c_str(), sFilepath2.c_str());
	sCommonDirectory = sCommonDirectory.substr(0, sCommonDirectory.rfind(L'\\'));

	// If the file paths are UNC paths the common prefix could contain only
	// the server name or even only the two backslashes. In this case the two
	// paths do not share a common parent directory.
	if (IsUNCPath(sCommonDirectory))
	{
		std::wstring sServer;
		std::wstring sShare;
		std::wstring sFilepath;
		UNCPathComponents(sCommonDirectory, sServer, sShare, sFilepath);

		if (sShare.empty())
		{
			sCommonDirectory.clear();
		}
	}

	return sCommonDirectory;
}

/// <summary> Creates the relative path from a reference to a target </summary>
/// <param name="sReferencePath"> Reference path </param>
/// <param name="sTargetPath"> Target path </param>
/// <returns> Relative path from reference to target </returns>
std::wstring CFileUtilities::GetRelativePath(const std::wstring& sReferencePath, const std::wstring& sTargetPath)
{
	std::filesystem::path relPath;
	if (std::filesystem::is_directory(sReferencePath))
	{
		relPath = std::filesystem::proximate(sTargetPath, sReferencePath);
	}
	else
	{
		relPath = std::filesystem::proximate(sTargetPath, std::filesystem::path(sReferencePath).parent_path());
	}
	if (relPath.is_relative() && (relPath.wstring().at(0) != L'.'))
	{
		// we want relative paths that refer to a file in the same folder or a
		// sub-folder to start with a dot
		relPath = std::filesystem::path(L".") / relPath;
	}
	return relPath.wstring();
}

/// <summary> Converts a relative path to a target to an absolute path </summary>
/// <param name="sReferencePath"> Reference path (must be absolute) </param>
/// <param name="sTargetPath"> Target path (can be relative) </param>
/// <returns> Absolute path to target </returns>
std::wstring CFileUtilities::GetAbsolutePath(const std::wstring& sReferencePath, const std::wstring& sTargetPath)
{
	if (std::filesystem::path(sTargetPath).is_absolute())
	{
		return sTargetPath;
	}

	std::filesystem::path absPath(sReferencePath);
	if (!std::filesystem::is_directory(absPath))
	{
		absPath = absPath.parent_path();
	}
	return std::filesystem::weakly_canonical(absPath / sTargetPath).wstring();
}

/**	\brief Löscht alte Dateien in einem Verzeichnis.
 *
 *	In dem Verzeichnis \a directory werden alle Dateien gelöscht, die vor mehr
 *	als \a OlderThanDays Tagen gespeichert wurden (relevant ist die letzte Zeit
 *	eines schreibenden Zugriffs).
 *
 *	\param[in] directory Der Pfad zum Verzeichnis, aus dem Dateien gelöscht
 *		werden sollen.
 *	\param[in] OlderThenDays Das Mindestalter der zu löschenden Dateien (in
 *		Tagen).
 *
 *	\author Klaus-Martin Reichert
 *	\author Stephan Allgeier
 *
 *	\see DeleteOldFiles(const std::wstring&, CRegex, int)
 *	\see GetFilesInDirectory(const std::wstring&)
 *	\see DeleteDirectory()
 */
void CFileUtilities::DeleteOldFiles(const std::wstring& directory, int OlderThenDays)
{
	time_t tNow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	auto files = GetFilesInDirectory(directory);
	try
	{
		for (const auto& file : files)
		{
			time_t tFile = boost::filesystem::last_write_time(file);
			auto tDiffDays = (tNow - tFile) / (60 * 60 * 24);
			if (tDiffDays > OlderThenDays)
			{
				std::filesystem::remove(file);
			}
		}
	}
	catch (const boost::filesystem::filesystem_error&)
	{
	}
}

/**	\brief Löscht alte Dateien in einem Verzeichnis.
 *
 *	In dem Verzeichnis \a directory werden alle Dateien gelöscht, deren
 *	Dateiname dem Muster \a fileNamePattern entspricht und die vor mehr als
 *	\a OlderThanDays Tagen gespeichert wurden (relevant ist die letzte Zeit
 *	eines schreibenden Zugriffs).
 *
 *	\param[in] directory Der Pfad zum Verzeichnis, aus dem Dateien gelöscht
 *		werden sollen.
 *	\param[in] fileNamePattern Das Muster für die Dateinamen der zu löschenden
 *		Dateien.
 *	\param[in] OlderThenDays Das Mindestalter der zu löschenden Dateien (in
 *		Tagen).
 *
 *	\author Klaus-Martin Reichert
 *	\author Stephan Allgeier
 *
 *	\see DeleteOldFiles(const std::wstring&, int)
 *	\see GetFilesInDirectory(const std::wstring&, CRegex)
 *	\see DeleteDirectory()
 */
void CFileUtilities::DeleteOldFiles(const std::wstring& directory, CRegex fileNamePattern, int OlderThenDays)
{
	time_t tNow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	auto files = GetFilesInDirectory(directory, fileNamePattern);
	try
	{
		for (const auto& file : files)
		{
			time_t tFile = boost::filesystem::last_write_time(file);
			auto tDiffDays = (tNow - tFile) / (60 * 60 * 24);
			if (tDiffDays > OlderThenDays)
			{
				std::filesystem::remove(file);
			}
		}
	}
	catch (const boost::filesystem::filesystem_error&)
	{
	}
}

std::vector<std::wstring> CFileUtilities::GetFilesInDirectory(const std::wstring& directory)
{
	std::filesystem::path dirPath(directory);

	if (!exists(dirPath))
	{
		throw std::exception("Path does not exist!.");
	}
	if (!is_directory(dirPath))
	{
		throw std::exception("Path is not a directory!.");
	}

	std::vector<std::wstring> files;

	for (auto& entry : std::filesystem::directory_iterator(dirPath))
	{
		auto p = entry.path();
		if (is_regular_file(p))
			files.push_back(p.wstring());
	}

	return files;
}

std::vector<std::wstring> CFileUtilities::GetFilesInDirectory(const std::wstring& directory, CRegex fileNameRegex)
{
	std::vector<std::wstring> allFiles = GetFilesInDirectory(directory);
	std::vector<std::wstring> matchingFiles;

	for (auto file : allFiles)
	{
		std::filesystem::path p(file);
		auto fileName = p.filename().wstring();
		if (fileNameRegex.Match(fileName))
			matchingFiles.push_back(file);
	}
	return matchingFiles;
}

std::vector<std::wstring> CFileUtilities::GetDirectorysInDirectory(const std::wstring& directory)
{
	std::filesystem::path dirPath(directory);

	if (!exists(dirPath))
	{
		throw std::exception("Path does not exist!.");
	}
	if (!is_directory(dirPath))
	{
		throw std::exception("Path is not a directory!.");
	}

	std::vector<std::wstring> dirs;

	for (auto& entry : std::filesystem::directory_iterator(dirPath))
	{
		auto p = entry.path();
		if (is_directory(p))
			dirs.push_back(p.wstring());
	}

	return dirs;
}

std::vector<std::wstring> CFileUtilities::GetDirectorysInDirectory(const std::wstring& directory, CRegex DirectoryRegex)
{
	std::vector<std::wstring> allDirs = GetDirectorysInDirectory(directory);
	std::vector<std::wstring> matchingDirs;

	for (auto file : allDirs)
	{
		std::filesystem::path p(file);
		auto fileName = p.filename().wstring();
		if (DirectoryRegex.Match(fileName))
			matchingDirs.push_back(file);
	}
	return matchingDirs;
}

std::wstring CFileUtilities::FullFile(const std::vector<std::wstring>& FileParts)
{
	auto iterator = FileParts.begin();
	std::wstring fullFile = *iterator++;

	while (iterator != FileParts.end())
	{
		if (*iterator == L"")
		{
			++iterator;
			continue;
		}

		if (!boost::ends_with(fullFile, L"\\") && !boost::starts_with(*iterator, L"\\"))
		{
			fullFile += L"\\";
		}
		else if (boost::ends_with(fullFile, L"\\") && boost::starts_with(*iterator, L"\\"))
		{
			fullFile = fullFile.substr(0, fullFile.size() - 1);
		}

		fullFile += *iterator;
		++iterator;
	}
	return fullFile;
}

std::wstring CFileUtilities::GetExtension(const std::wstring& fileName)
{
	auto index = fileName.rfind('.');
	return fileName.substr(index + 1, fileName.size());
}

std::vector<std::wstring> CFileUtilities::ByLine(const std::wstring& fileName)
{
	std::vector<std::wstring> out;

	std::wifstream file;
	file.open(std::filesystem::path(fileName));

	// These two lines fix the UTF8-Problem
	// Careful, it looks like a memory leak, but is internally deleted
	// source: http://en.cppreference.com/w/cpp/locale/locale/locale
	std::locale ulocale(std::locale(), new std::codecvt_utf8<wchar_t>);
	file.imbue(ulocale);

	if (file.fail())
		return{};

	for (std::wstring line; std::getline(file, line); )
	{
		out.push_back(line);
	}

	return out;
}

std::wstring CFileUtilities::NormalizeSlashes(const std::wstring& path)
{
	return FullFile(FileParts(path));
}

std::vector<std::wstring> CFileUtilities::FileParts(const std::wstring& Path)
{
	auto slashes = std::wstring(L"\\/");
	auto Delimiter = slashes.c_str();
	boost::char_separator<std::wstring::value_type> delimiters(Delimiter);
	boost::tokenizer<boost::char_separator<std::wstring::value_type>, std::wstring::const_iterator, std::wstring> tokens(Path, delimiters);

	std::vector<std::wstring> Parts;

	for (auto token : tokens)
	{
		Parts.push_back(token);
	}
	if (!Parts.empty() && CStringUtilities::StartsWith(Path, L"\\\\"))
		Parts[0] = L"\\\\" + Parts[0];

	return Parts;
}

std::wstring CFileUtilities::GetParentDirectory(const std::wstring& File)
{
	if (File == L"")
		return L"";

	auto Parts = FileParts(File);
	if (Parts.size() == 1)
		return L"";

	std::vector<std::wstring> subVector(Parts.begin(), Parts.end() - 1);
	return FullFile(subVector);
}

bool CFileUtilities::IsAbsolute(const std::wstring& sPath)
{
	return IsDrivePath(sPath) || IsUNCPath(sPath);
}

bool CFileUtilities::IsRelative(const std::wstring& sPath)
{
	return !IsAbsolute(sPath);
}

/*
Returns the file name without the extension.
If the passed string is a directory, the directory name is returned instead.
*/
std::wstring CFileUtilities::GetFileName(const std::wstring& fileName)
{
	auto Parts = FileParts(fileName);
	auto LastPart = Parts.back();

	auto Extension = GetExtension(LastPart);

	if (Extension.size() > 0)
		return LastPart.substr(0, LastPart.size() - Extension.size() - 1);
	else
		return LastPart;
}

void CFileUtilities::CopyFile(const std::wstring& source, const std::wstring& destination, bool bCreateDestinationFolders)
{
	if (bCreateDestinationFolders)
	{
		auto Parent = GetParentDirectory(destination);
		MakeDirectory(Parent, true);
	}
	std::filesystem::copy_file(source, destination);
}

std::filesystem::path CFileUtilities::GetProgramFullPath()
{
	return boost::dll::program_location().wstring();
}

std::filesystem::path CFileUtilities::GetProgramFolder()
{
	std::filesystem::path path = GetProgramFullPath();
	return path.parent_path();
}

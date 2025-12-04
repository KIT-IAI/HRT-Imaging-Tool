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
#include "TextFileOutput.h"

#include "DenseMatrix.h"



/**	\brief Der Standard-Konstruktor legt standardmäßig die Länge der zu
 *	schreibenden Zahlen auf 12 und die Anzahl der Nachkommastellen auf 3
 *	fest.
 */
CTextFileOutput::CTextFileOutput()
{
	m_sDump = L"";
}

/**	\brief Der Standard-Destruktor.
 */
CTextFileOutput::~CTextFileOutput()
{
}

/**	\brief Diese Methode löscht den Ausgabedump.
 */
void CTextFileOutput::Reset()
{
	m_sDump = L"";
}

/**	\brief Fügt einen String hinzu.
 *
 *	\param[in] sString Die einzufügende Zeichenkette.
 */
void CTextFileOutput::Write(std::wstring sString)
{
	m_sDump += sString;
}

/**	\brief Fügt einen String und ein Newline hinzu.
 *
 *	\param[in] sString Die einzufügende Zeichenkette.
 */
void CTextFileOutput::Writeln(std::wstring sString)
{
	m_sDump += sString + L"\n";
}

void CTextFileOutput::Writeln(const CDenseMatrix& mat)
{
	Writeln(mat.ToString());
}
void CTextFileOutput::Writeln(const CDenseVector& mat)
{
	Writeln(mat.ToString());
}

/**	\brief Speichert den Ausgabe-Dump unter dem angegebenen Pfad.
 *
 *	\return true, wenn der Ausgabe-Dump erfolgreich war.
 *	\return false, wenn die angegebene Datei nicht geöffnet werden konnte.
 *
 *	\param[in] sPathname Pfadangabe.
 */
bool CTextFileOutput::Save(std::wstring sPathname) const
{
	std::wofstream file;
	file.open(std::filesystem::path(sPathname));

	if (!file.is_open())
		return false;

	file << m_sDump;

	return true;
}

/**	\brief Fügt eine Integer-Zahl hinzu.
 *
 *	\param[in] nInt Eine Integer-Zahl.
 */
void CTextFileOutput::Write(int nInt)
{
	m_sDump += std::to_wstring(nInt) + L"\t";
}

/**	\brief Fügt eine Integer-Zahl und ein Newline hinzu.
 *
 *	\param[in] nInt Eine Integer-Zahl.
 */
void CTextFileOutput::Writeln(int nInt)
{
	m_sDump += std::to_wstring(nInt) + L"\n";
}

/**	\brief Fügt eine Float-Zahl hinzu.
 *
 *	\param[in] fFloat Eine Float-Zahl.
 */
void CTextFileOutput::Write(float fFloat)
{
	m_sDump += std::to_wstring(fFloat) + L"\t";
}

/**	\brief Fügt eine Float-Zahl und ein Newline hinzu.
 *
 *	\param[in] fFloat Eine Float-Zahl.
 */
void CTextFileOutput::Writeln(float fFloat)
{
	m_sDump += std::to_wstring(fFloat) + L"\n";
}

/**	\brief Fügt eine Double-Zahl hinzu.
 *
 *	\param[in] fDouble Eine Double-Zahl.
 */
void CTextFileOutput::Write(double fDouble)
{
	m_sDump += std::to_wstring(fDouble) + L"\t";
}

/**	\brief Fügt eine Double-Zahl und ein Newline hinzu.
 *
 *	\param[in] fDouble Eine Double-Zahl.
 */
void CTextFileOutput::Writeln(double fDouble)
{
	m_sDump += std::to_wstring(fDouble) + L"\n";
}

/**	\brief Fügt ein eindimensionales Float-Array hinzu.
 *
 *	Alle Werte werden in eine Zeile geschrieben. Am Ende folgt ein Newline.
 *
 *	\param[in] fArray Das Array mit den Float-Werten.
 *	\param[in] nSize Die Anzahl der Float-Werte im Array.
 *
 *	\see Writeln(float *, int)
 */
void CTextFileOutput::Write(float* fArray, int nSize)
{
	std::wstring sSave;
	for (int i = 0; i < nSize - 1; i++)
	{
		m_sDump += std::to_wstring(fArray[i]) + L"\t";
	}
	m_sDump += std::to_wstring(fArray[nSize - 1]) + L"\n";
}

/**	\brief Fügt ein eindimensionales Float-Array hinzu.
 *
 *	Jeder Wert wird in eine eigene Zeile geschrieben. Am Ende folgt ein
 *	Newline.
 *
 *	\param[in] fArray Das Array mit den Float-Werten.
 *	\param[in] nSize Die Anzahl der Float-Werte im Array.
 *
 *	\see Write(float *, int)
 */
void CTextFileOutput::Writeln(float* fArray, int nSize)
{
	std::wstring sSave;
	for (int i = 0; i < nSize; i++)
	{
		m_sDump += std::to_wstring(fArray[i]) + L"\n";
	}
}

/**	\brief Fügt ein zweidimensionales Float-Array hinzu.
 *
 *	Jede _T("Zeile") des zweidimensionalen Arrays wird in eine eigene Zeile
 *	geschrieben. Am Ende folgt ein Newline.
 *
 *	\param[in] fArray Das Array mit den Float-Werten.
 *	\param[in] nSizeY Die Größe des Arrays in der ersten Koordinate.
 *	\param[in] nSizeX Die Größe des Arrays in der zweiten Koordinate.
 */
void CTextFileOutput::Write(float** fArray, int nSizeY, int nSizeX)
{
	std::wstring sSave;

	for (int j = 0; j < nSizeY; j++)
	{
		for (int i = 0; i < nSizeX - 1; i++)
		{
			m_sDump += std::to_wstring(fArray[j][i]) + L"\t";
		}
		m_sDump += std::to_wstring(fArray[j][nSizeX - 1]) + L"\n";
	}
}

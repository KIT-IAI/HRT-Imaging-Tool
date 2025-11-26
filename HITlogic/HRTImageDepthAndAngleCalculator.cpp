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
#include "HRTImageDepthAndAngleCalculator.h"

#include "PiezoImageTime.h"



CHRTImageDepthAndAngleCalculator::CHRTImageDepthAndAngleCalculator()
{
}

CHRTImageDepthAndAngleCalculator::~CHRTImageDepthAndAngleCalculator()
{
}

bool CHRTImageDepthAndAngleCalculator::giveBasicInfo(int ImageCount, int PlaneCount, int PlaneIncrement)
{
	if (ImageCount < 1 || PlaneCount < 1 || PlaneIncrement <= 0)
	{
		return false;
	}

	m_ImageData.setCount(ImageCount);
	double calcDepth = 0.0;
	int planeCounter = 0;
	bool direction = false;
	int i = 0;

	while (i < ImageCount)
	{
		//if (planeCounter >= PlaneCount)
		if ((i % PlaneCount) == 0 && i >= 1)
		{
			direction = !direction; //reverse direction	
			if (direction)
			{
				calcDepth += (0.5 * PlaneIncrement);
			}
			else
			{
				calcDepth -= (0.5 * PlaneIncrement);
			}
			m_ImageData.setImageDepth(i - 1, calcDepth);//2x same value (overwrite previous value)
		}
		m_ImageData.setImageDepth(i, calcDepth);

		if (direction)
		{
			calcDepth += (0.5 * PlaneIncrement);
		}
		else
		{
			calcDepth -= (0.5 * PlaneIncrement);
		}

		planeCounter++;
		i++;
	}
	return false;
}

bool CHRTImageDepthAndAngleCalculator::givePiezoData(int /*ImageCount*/, wstring filePathAndSeriesName, wstring snpFolder, bool useOffsetVar, double Offset)
{
	wstring seriesName;
	vector<wstring> filecut = CFileUtilities::FileParts(filePathAndSeriesName);
	seriesName = filecut.back();
	wstring ParameterFile = snpFolder + L"\\" + seriesName + L"_Parameters.txt";
	wstring PiezoFile = filePathAndSeriesName + L"_piezo1.txt";

	if (!((CFileUtilities::FileExists(ParameterFile)) && (CFileUtilities::FileExists(PiezoFile))))
	{
		CLog::Log(CLog::eWarning, L"CHRTImageDepthAndAngleCalculator", L"Ein Teil der Piezo- / Bilddaten fehlt");
		return false; //wenn Daten fehlen kann Höhe nicht aus Piezo bestimmt werden
	}

	m_ImageData.setPiezoFilePath(filePathAndSeriesName);
	m_ImageData.setSnpPath(snpFolder);

	PiezoImageTime DepthCalc;
	DepthCalc.readPiezo(PiezoFile);
	DepthCalc.readImageTime(ParameterFile);

	if (!useOffsetVar)
	{
		//Offest wird nicht via Argument gegeben
		std::filesystem::path filepath(filePathAndSeriesName + L"_offset.txt");
		if (CFileUtilities::FileExists(filepath))
		{
			//gibts in die Zusatzinformationen als Datei?
			std::wifstream ifstream(filepath);
			if (ifstream.is_open() && (ifstream >> Offset))
			{
				CLog::Log(CLog::eNotice, L"CHRTImageDepthAndAngleCalculator", L"Nutze Offset aus Offset Datei");
			}
			else
			{
				CLog::Log(CLog::eNotice, L"CHRTImageDepthAndAngleCalculator", L"Fehler beim Lesen aus Offsetdatei. Berechne Offset aus Zeitdifferenz Piezo- und Bild-Start. ");
				Offset = 0;
			}
		}
		else
		{
			CLog::Log(CLog::eNotice, L"CHRTImageDepthAndAngleCalculator", L"Kein Offset gesetzt und keine Offsetdatei gefunden. Berechne Offset aus Zeitdifferenz Piezo- und Bild-Start. ");
			Offset = 0;
		}
	}

	DepthCalc.syncManualOffset(Offset);

	int i = static_cast<int>(DepthCalc.getAllImageValCount());
	double avgHeight = DepthCalc.getAvergaeHeight();
	m_ImageData.setCount(i);
	m_ImageData.setInclination(true);
	//kein Winkelrechner sondern nur setzen Parameter hasInclination (Kennzeichnen, dass Serie aus geneigten Bildern besteht)

	while (i > 0)
	{
		i--;
		m_ImageData.setImageDepth(i, DepthCalc.getImageHeightAt(i) - avgHeight);
	}

	return true;
}

bool CHRTImageDepthAndAngleCalculator::giveCombined(int ImageCount, int PlaneCount, wstring filePathAndSeriesName, wstring snpFolder, int PlaneIncrement, bool useOffsetVar, double Offset)
{
	if (!givePiezoData(ImageCount, filePathAndSeriesName, snpFolder, useOffsetVar, Offset))
	{
		CLog::Log(CLog::eWarning, L"CHRTImageDepthAndAngleCalculator", L"Höhenberechnung aus Piezo gescheitert, fallback auf BasisInformationen");
		return giveBasicInfo(ImageCount, PlaneCount, PlaneIncrement);
	}
	return true;
}

bool CHRTImageDepthAndAngleCalculator::isResultAvailable()
{
	return m_ImageData.isAvailable();
}

CHRTImageDepthAndAngleData CHRTImageDepthAndAngleCalculator::getResults()
{
	CLog::Log(CLog::eDebug, L"CHRTImageDepthAndAngleCalculator", L"returning results");
	return m_ImageData;
}

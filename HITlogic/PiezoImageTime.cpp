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
#include "PiezoImageTime.h"



PiezoImageTime::PiezoImageTime()
{
}

PiezoImageTime::~PiezoImageTime()
{
}

int PiezoImageTime::readPiezo(const std::wstring& filepath)
{
	m_PiezoValues.clear();
	synced = false;

	m_minPiezoHeight = std::numeric_limits<int>::max();
	m_maxPiezoHeight = std::numeric_limits<int>::min();

	std::wifstream ifs{std::filesystem::path(filepath)};
	if (!ifs.is_open())
		return -1;

	int errorlines = 0;

	int year = 0;
	int month = 0;
	int day = 0;
	int hour = 0;
	int minute = 0;
	int second = 0;
	int millis = 0;
	double height = 0.0;
	std::wstring heightString;
	wchar_t delim;

	// example line:
	// 2021-01-22T13:52:49.907569;496,184825897
	while (!ifs.eof())
	{
		if (ifs >> year >> delim >> month >> delim >> day >> delim >> hour >> delim >> minute >> delim >> second >> delim >> millis >> delim >> heightString)
		{
			// potentially correct line
			std::replace(heightString.begin(), heightString.end(), L',', L'.');
			std::wistringstream iss{heightString};
			if ((iss >> height) && iss.eof())
			{
				// correct line
				Timestamp tmp;
				tmp.hms = hour * 3600 + minute * 60 + second;
				tmp.mili = static_cast<double>(millis);
				tmp.height = height;
				tmp.Inumber = 0;
				m_PiezoValues.push_back(tmp);
			}
			else
			{
				// incorrect line
				errorlines++;
				continue;
			}
		}
		else if (ifs.eof())
		{
			// empty last line
			continue;
		}
		else
		{
			// incorrect line
			errorlines++;
			continue;
		}

		if (height < m_minPiezoHeight)
		{
			m_minPiezoHeight = height;
		}
		if (height > m_maxPiezoHeight)
		{
			m_maxPiezoHeight = height;
		}
	}

	if (m_PiezoValues.size() == 0)
	{
		// not a single correct line
		return -1;
	}

	m_PiezoDif = m_maxPiezoHeight - m_minPiezoHeight;
	calcAvergaeHeight();
	m_firstRealPiezoVal = m_PiezoValues[0];

	recalcTime(&m_PiezoValues);
	return errorlines;
}

//berechnet Zeiten von erstem Timestamp als Nullpunkt aus
void PiezoImageTime::recalcTime(vector<Timestamp>* timeVect) {
	if (timeVect->size() > 0) {
		int startHms = timeVect->at(0).hms;
		double startMili = timeVect->at(0).mili;

		for (Timestamp& tmp : *timeVect)
		{
			//Timestamp tmp = timeVect->at(i);
			tmp.hms = tmp.hms - startHms;
			tmp.mili = tmp.mili - startMili;
			if (tmp.mili < 0) {
				tmp.hms--;
				tmp.mili += 1000 * 1000;
			}

			tmp.mili = tmp.mili;
		}
	}
}

int PiezoImageTime::readImageTime(const std::wstring& filepath)
{
	m_ImageValues.clear();
	m_missingImages.clear();
	m_allImages.clear();
	synced = false;

	std::wifstream ifs{std::filesystem::path(filepath)};
	if (!ifs.is_open())
		return -1;

	std::wstring line;

	// look for header line above tabular data (only line containing #)
	bool ok = false;
	while (!ifs.eof())
	{
		if (std::getline(ifs, line) && (line.find(L'#') != std::wstring::npos))
		//if ((ifs >> line) && (line.find(L'#') != std::wstring::npos))
		{
			ok = true;
			break;
		}
	}

	if (!ok)
	{
		return -1;
	}

	int errorlines = 0;

	int imageID = 0;
	int year = 0;
	int month = 0;
	int day = 0;
	int hour = 0;
	int minute = 0;
	int second = 0;
	int millis = 0;

	// example line:
	// 00001	2021	1	22	13	52	46	497	0.033000	0	85	6.643812	7.649688	-59	-32768	2	9.483622	0.000000	0.000000	-6	1.000000	0.000000	0.000000	0.000000	1.000000	0.000000
	while (!ifs.eof())
	{
		if (ifs >> imageID >> year >> month >> day >> hour >> minute >> second >> millis)
		{
			// correct line
			// The following handling of split-seconds is really weird, but the
			// split-seconds are actually saved as a double value representing
			// the decimal part AS IF IT WERE AN INTEGER. So for example the
			// decimal part of 3.14 would be saved as 14, and the decimal part
			// of 3.140 would be saved as 140, even though they should be the
			// same thing. For that reason, we multiply the three digit
			// split-seconds part by 1000 here, in order to have six decimal
			// digits, as in the piezo data part. This is really unintuitive and
			// error-prone, and if we ever actually actively use this again, we
			// should REALLY change this!
			Timestamp tmp;
			tmp.hms = hour * 3600 + minute * 60 + second;
			tmp.mili = static_cast<double>(millis * 1000);
			tmp.Inumber = imageID;
			tmp.height = 0.0;
			m_allImages.push_back(tmp);
		}
		else if (ifs.eof())
		{
			// empty last line
			continue;
		}
		else
		{
			// incorrect line
			errorlines++;
		}
		std::getline(ifs, line);
	}

	std::copy(m_allImages.begin(), m_allImages.end(), std::back_inserter(m_ImageValues));
	if (m_allImages.size() == 0) //wenn Daten aus irgendeinem Grund sinnvoll aber nicht lesbar
		return -1;
	m_firstRealImageVal = m_allImages.at(0);

	recalcTime(&m_ImageValues);
	recalcTime(&m_allImages);
	return errorlines;
}

void PiezoImageTime::giveImageFilterByNames(vector<wstring> tmp) {  //Gibt Dateinamen der verwendeten Bilder um Zeitdaten zu nicht verwendeten Bildern zu filtern
	m_iImageFilter.clear();
	m_ImageValues.clear();
	m_missingImages.clear();
	if (!tmp.empty()) {
		for (auto el : tmp) {
			wstring inum = el.substr(el.size() - 9, 5);
			//CLog::Log(CLog::eDebug,L"test: ",inum.c_str());
			m_iImageFilter.push_back(std::stoi(inum));
		}
	}
	for (Timestamp tmpEl : m_allImages) {
		if ((m_iImageFilter.empty()) || (std::find(m_iImageFilter.begin(), m_iImageFilter.end(), tmpEl.Inumber) != m_iImageFilter.end())) {  //nur speichern wenn auch dazu gehörendes Bild geladen oder alle wenn keine Bilder geladen
			m_ImageValues.push_back(tmpEl);
		}
		else { //alle im Mosaik fehlenden Bilder
			m_missingImages.push_back(tmpEl);
		}
	}

	m_missingImages.insert(m_missingImages.begin(), m_ImageValues.at(0));
}


int PiezoImageTime::ret_ValueCountTime(int time) {
	if (time < 1)
		time = 1;
	int count = 0;
	for (Timestamp tmp : m_PiezoValues) {
		count++;
		if (tmp.hms >= time)
			break;
	}
	return count;
}

void PiezoImageTime::syncManualOffset(double OffsetIn) {
	resetOffset();

	double OffsetJustMili = OffsetIn * 1000; //Offset in korrektes Format bringen

	//berechnet aus dem kompletten Offset die Hms und Mili-Werte
	double offsetHms;
	if (OffsetJustMili >= 0) {
		offsetHms = floor(OffsetJustMili / 1000000);
	}
	else {
		offsetHms = -1 * (floor(abs(OffsetJustMili) / 1000000));
	}
	double offsetMili = fmod(OffsetJustMili, 1000000);

	recalcTime(&m_ImageValues, int(offsetHms), offsetMili); //dann Berechnen mit Offset
	recalcTime(&m_missingImages, int(offsetHms), offsetMili);
	recalcTime(&m_allImages, int(offsetHms), offsetMili);

	int counter = 0;
	for (Timestamp& tmp : m_ImageValues) {
		while (tmp.calcJustMili() > m_PiezoValues.at(counter).calcJustMili()) { //suchen grobe Höhe zu Bild
			counter++;
			if (counter >= m_PiezoValues.size()) {  //wenn hinten raus (PiezoZeit viel größer als ImageZeit)
				counter--;
				break;
			}
		}
		if (counter == 0) {  //wenn vorne raus (PiezoZeit < ImageZeit)
			tmp.height = m_PiezoValues[counter].height;
		}
		else {
			Timestamp before = m_PiezoValues[counter - 1];  //Höhe vor Bild
			Timestamp after = m_PiezoValues[counter];   //Höhe nach Bild

			double timePiezoDif = double(after.calcJustMili() - before.calcJustMili());
			double timeImageDif = double(after.calcJustMili() - tmp.calcJustMili());
			double transFakt = 0;
			if (timeImageDif != 0) {
				transFakt = timeImageDif / timePiezoDif;
			}

			tmp.height = after.height - (after.height - before.height) * transFakt;
		}
		/*if (tmp.height < m_minPiezoHeight || tmp.height > m_maxPiezoHeight) {
		int stop = 0;
		}*/
	}

	counter = 0; //reset counter
	bool first = true;
	for (Timestamp& tmp : m_missingImages) { //siehe oben nur für nicht verwendete Bilder
		if (first) { //erstes Bild dient nur zur Synchronisation, führt aber sonst zu Fehlern
			first = false;
			continue;
		}
		while (tmp.calcJustMili() > m_PiezoValues.at(counter).calcJustMili()) {
			counter++;
			if (counter >= m_PiezoValues.size()) {
				counter--;
				break;
			}
		}
		if (counter == 0) {
			tmp.height = m_PiezoValues[counter].height;
		}
		else {
			Timestamp before = m_PiezoValues[counter - 1];
			Timestamp after = m_PiezoValues[counter];

			double timePiezoDif = double(after.calcJustMili() - before.calcJustMili());
			double timeImageDif = double(after.calcJustMili() - tmp.calcJustMili());
			double transFakt = 0;
			if (timeImageDif != 0) {
				transFakt = timeImageDif / timePiezoDif;
			}

			tmp.height = after.height - (after.height - before.height) * transFakt;
		}
		/*if (tmp.height < m_minPiezoHeight || tmp.height > m_maxPiezoHeight) {
		int stop = 0;
		}*/
	}

	counter = 0; //reset counter
	for (Timestamp& tmp : m_allImages) { //siehe oben nur für nicht verwendete Bilder
		while (tmp.calcJustMili() > m_PiezoValues.at(counter).calcJustMili()) {
			counter++;
			if (counter >= m_PiezoValues.size()) {
				counter--;
				break;
			}
		}
		if (counter == 0) {
			tmp.height = m_PiezoValues[counter].height;
		}
		else {
			Timestamp before = m_PiezoValues[counter - 1];
			Timestamp after = m_PiezoValues[counter];

			double timePiezoDif = double(after.calcJustMili() - before.calcJustMili());
			double timeImageDif = double(after.calcJustMili() - tmp.calcJustMili());
			double transFakt = 0;
			if (timeImageDif != 0) {
				transFakt = timeImageDif / timePiezoDif;
			}

			tmp.height = after.height - (after.height - before.height) * transFakt;
		}
		/*if (tmp.height < m_minPiezoHeight || tmp.height > m_maxPiezoHeight) {
		int stop = 0;
		}*/
	}
	synced = true;
}

void PiezoImageTime::resetOffset() {
	recalcTime(&m_ImageValues); //setzen auf 0
	recalcTime(&m_missingImages);
	recalcTime(&m_allImages);

	synced = false;
}

//gibt <offsetHms,offsetMili> des jeweiligen Paars zurück
double PiezoImageTime::getOffset(int Pstep, int Inum) {
	double retVal = double(m_PiezoValues.at(Pstep).calcJustMili() - m_allImages.at(Inum).calcJustMili());
	return retVal;
}
//ungenau wenn Bild an Minimum / Maximum
void PiezoImageTime::syncImagePiezoHeight(int Pstep, int Inum) {
	resetOffset();

	int offsetHms = m_PiezoValues.at(Pstep).hms - m_allImages.at(Inum).hms;
	double offsetMili = m_PiezoValues.at(Pstep).mili - m_allImages.at(Inum).mili;

	recalcTime(&m_ImageValues, offsetHms, offsetMili); //dann Berechnen mit Offset
	recalcTime(&m_missingImages, offsetHms, offsetMili);
	recalcTime(&m_allImages, offsetHms, offsetMili);

	int counter = 0;
	for (Timestamp& tmp : m_ImageValues) {
		while (tmp.calcJustMili() > m_PiezoValues.at(counter).calcJustMili()) { //suchen grobe Höhe zu Bild
			counter++;
			if (counter >= m_PiezoValues.size()) {  //wenn hinten raus (PiezoZeit viel größer als ImageZeit)
				counter--;
				break;
			}
		}
		if (counter == 0) {  //wenn vorne raus (PiezoZeit < ImageZeit)
			tmp.height = m_PiezoValues[counter].height;
		}
		else {
			Timestamp before = m_PiezoValues[counter - 1];  //Höhe vor Bild
			Timestamp after = m_PiezoValues[counter];   //Höhe nach Bild

			double timePiezoDif = double(after.calcJustMili() - before.calcJustMili());
			double timeImageDif = double(after.calcJustMili() - tmp.calcJustMili());
			double transFakt = 0;
			if (timeImageDif != 0) {
				transFakt = timeImageDif / timePiezoDif;
			}

			tmp.height = after.height - (after.height - before.height) * transFakt;
		}
		/*if (tmp.height < m_minPiezoHeight || tmp.height > m_maxPiezoHeight) {
			int stop = 0;
		}*/
	}

	counter = 0; //reset counter
	bool first = true;
	for (Timestamp& tmp : m_missingImages) { //siehe oben nur für nicht verwendete Bilder
		if (first) { //erstes Bild dient nur zur Synchronisation, führt aber sonst zu Fehlern
			first = false;
			continue;
		}
		while (tmp.calcJustMili() > m_PiezoValues.at(counter).calcJustMili()) {
			counter++;
			if (counter >= m_PiezoValues.size()) {
				counter--;
				break;
			}
		}
		if (counter == 0) {
			tmp.height = m_PiezoValues[counter].height;
		}
		else {
			Timestamp before = m_PiezoValues[counter - 1];
			Timestamp after = m_PiezoValues[counter];

			double timePiezoDif = double(after.calcJustMili() - before.calcJustMili());
			double timeImageDif = double(after.calcJustMili() - tmp.calcJustMili());
			double transFakt = 0;
			if (timeImageDif != 0) {
				transFakt = timeImageDif / timePiezoDif;
			}

			tmp.height = after.height - (after.height - before.height) * transFakt;
		}
		/*if (tmp.height < m_minPiezoHeight || tmp.height > m_maxPiezoHeight) {
			int stop = 0;
		}*/
	}

	counter = 0; //reset counter
	for (Timestamp& tmp : m_allImages) { //siehe oben nur für nicht verwendete Bilder
		while (tmp.calcJustMili() > m_PiezoValues.at(counter).calcJustMili()) {
			counter++;
			if (counter >= m_PiezoValues.size()) {
				counter--;
				break;
			}
		}
		if (counter == 0) {
			tmp.height = m_PiezoValues[counter].height;
		}
		else {
			Timestamp before = m_PiezoValues[counter - 1];
			Timestamp after = m_PiezoValues[counter];

			double timePiezoDif = double(after.calcJustMili() - before.calcJustMili());
			double timeImageDif = double(after.calcJustMili() - tmp.calcJustMili());
			double transFakt = 0;
			if (timeImageDif != 0) {
				transFakt = timeImageDif / timePiezoDif;
			}

			tmp.height = after.height - (after.height - before.height) * transFakt;
			if (timePiezoDif > 100000.0)
			{
				std::wstring formatString(L"timePiezoDif very high at: Inum: %d, Piezostep(counter) %d, Before: %f, After %f, timePiezoDif: %f, timeImageDif %f");
				CLog::Log(CLog::eWarning, L"MosaikbildDlg",
					boost::wformat(formatString) % tmp.Inumber % counter % before.calcJustMili() % after.calcJustMili() % timePiezoDif % timeImageDif);
			}
			//else
			//{
			//	std::wstring formatString(L"Inum: %d, Piezostep(counter) %d, Before: %f, After %f, timePiezoDif: %f, timeImageDif %f");
			//	CLog::Log(CLog::eWarning, L"MosaikbildDlg",
			//		boost::wformat(formatString) % tmp.Inumber % counter % before.calcJustMili() % after.calcJustMili() % timePiezoDif % timeImageDif);
			//}
		}
		/*if (tmp.height < m_minPiezoHeight || tmp.height > m_maxPiezoHeight) {
		int stop = 0;
		}*/

	}
	synced = true;
}

void PiezoImageTime::recalcTime(vector<Timestamp>* timeVect, int offsetHms, double OffsetMili) {
	if (timeVect->size() > 0) {
		int startHms = timeVect->at(0).hms - offsetHms;
		double startMili = timeVect->at(0).mili - OffsetMili;

		for (Timestamp& tmp : *timeVect)
		{
			//Timestamp tmp = timeVect->at(i);
			tmp.hms = tmp.hms - startHms;
			tmp.mili = tmp.mili - startMili;
			if (tmp.mili < 0) {
				tmp.hms--;
				tmp.mili += 1000 * 1000;
			}
			if (tmp.mili >= 1000000) {
				tmp.hms++;
				tmp.mili -= 1000 * 1000;
			}

			tmp.mili = tmp.mili;
		}
	}
}

void PiezoImageTime::calcAvergaeHeight() {
	double completeHeight = 0;
	int counter = 0;
	for (int i = 0; i < 20; i++) {
		counter++;
		completeHeight += m_PiezoValues.at(i).height;
	}
	/*for (Timestamp tmp : m_PiezoValues) {
		counter++;
		completeHeight += tmp.height;
	}
	*/
	m_avergaHeight = completeHeight / counter;
}

void PiezoImageTime::recalcHeights() {
	m_minPiezoHeight = std::numeric_limits<int>::max();
	m_maxPiezoHeight = std::numeric_limits<int>::min();

	for (Timestamp& tmp : m_PiezoValues) {
		tmp.height = tmp.height - m_avergaHeight;

		if (tmp.height < m_minPiezoHeight) {
			m_minPiezoHeight = tmp.height;
		}
		if (tmp.height > m_maxPiezoHeight) {
			m_maxPiezoHeight = tmp.height;
		}
	}
	/*for (Timestamp &tmp : m_ImageValues) {
		tmp.height = tmp.height - m_avergaHeight;
	}
	for (Timestamp &tmp : m_missingImages) {
		tmp.height = tmp.height - m_avergaHeight;
	}*/
}
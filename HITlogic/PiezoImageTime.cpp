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
#include "Log.h"


PiezoImageTime::PiezoImageTime()
{
}


PiezoImageTime::~PiezoImageTime()
{

}

int PiezoImageTime::readPiezo(CString file) {
	m_PiezoValues.clear();
	synced = FALSE;

	CStdioFile f(file, CStdioFile::modeRead + CStdioFile::typeText);
	CString line = _T("");

	m_minPiezoHeight = INT_MAX;
	m_maxPiezoHeight = INT_MIN;

	int errorlines = 0;
	BOOL errorfree = true;

	while (f.ReadString(line)) { //Zeilenweise lesen
		vector<int> cuts;
		cuts.push_back(0);
		cuts.push_back(line.Find(L'T', cuts.at(0) + 1));
		cuts.push_back(line.Find(L'.', cuts.at(0) + 1));
		cuts.push_back(line.Find(L';', cuts.at(0) + 1));

		errorfree = true;
		for (int tmp : cuts) { //wenn teilzeichen nicht gefunden ist Zeile fehlerhaft
			if (tmp == -1) {
				errorfree = false;
				break;
			}
		}
		if (errorfree) {

			//Datum wird weg gelassen
			/*CString date = line.Mid(cuts.at(0), cuts.at(1));
			date.Remove(L'-');*/
			CString timeTmp = line.Mid(cuts.at(1) + 1, cuts.at(2) - cuts.at(1) - 1);
			UINT time = 0;
			time += _ttoi(timeTmp.Mid(0, 2)) * 3600;
			time += _ttoi(timeTmp.Mid(3, 2)) * 60;
			time += _ttoi(timeTmp.Mid(6, 2));
			CString mili = line.Mid(cuts.at(2) + 1, cuts.at(3) - cuts.at(2) - 1);
			CString height = line.Right(line.GetLength() - cuts.at(3) - 1);
			height.Replace(L',', L'.');

			//CLog::Log(CLog::eDebug, L"Test", "%i - " + mili + " - " + height, time);
			Timestamp tmp;
			tmp.hms = time;
			tmp.mili = _ttoi(mili);
			tmp.height = _ttof(height);
			tmp.Inumber = 0;
			m_PiezoValues.push_back(tmp);

			if (_ttof(height) < m_minPiezoHeight) {
				m_minPiezoHeight = _ttof(height);
			}
			if (_ttof(height) > m_maxPiezoHeight) {
				m_maxPiezoHeight = _ttof(height);
			}
		}
		else {
			errorlines++;
		}
	}
	if (m_PiezoValues.size() == 0) //wenn keine Zeilen fehlerfrei waren
		return -1;
	m_PiezoDif = m_maxPiezoHeight - m_minPiezoHeight;
	calcAvergaeHeight();
	m_firstRealPiezoVal = m_PiezoValues.at(0);

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


int PiezoImageTime::readImageTime(CString file) {
	m_ImageValues.clear();
	m_missingImages.clear();
	m_allImages.clear();
	synced = FALSE;

	CStdioFile f(file, CStdioFile::modeRead + CStdioFile::typeText);
	CString line = _T("");
	BOOL DataOk = false;

	//Suche Anfang der Zeitdaten (einzige Zeile mit #)
	while (f.ReadString(line)) {
		if (line.Find(L'#') != -1) {
			DataOk = true;
			break;
		}
	}

	if (DataOk) { //wenn # gefunden
		int errorlines = 0;
		BOOL errorfree = true;

		while (f.ReadString(line)) { //Zeilenweise lesen
			vector<int> cuts;
			cuts.push_back(0);
			for (int i = 1; i < 9; i++) {
				cuts.push_back(line.Find(L'	', cuts.at(i - 1) + 1));
			}
			errorfree = true;
			for (int tmp : cuts) {  //wenn nicht ausreichend Tabs zum Teilen gefunden ist Zeile fehlerhaft
				if (tmp == -1) {
					errorfree = false;
					break;
				}
			}
			if (errorfree) {
				CString Number = line.Mid(cuts.at(0), cuts.at(1)); //Bildnummer
				//Datum wird weg gelassen
				/*
				CString date =  (line.Mid(cuts.at(1) + 1, cuts.at(2) - cuts.at(1) - 1)); //Jahr
				date += norm2	(line.Mid(cuts.at(2) + 1, cuts.at(3) - cuts.at(2) - 1)); //Monat
				date += norm2	(line.Mid(cuts.at(3) + 1, cuts.at(4) - cuts.at(3) - 1)); //Tag
				*/

				UINT time = 0; //Zeit wird in Sekunden umgerechnet
				time += _ttoi(line.Mid(cuts.at(4) + 1, cuts.at(5) - cuts.at(4) - 1)) * 3600; //Stunde
				time += _ttoi(line.Mid(cuts.at(5) + 1, cuts.at(6) - cuts.at(5) - 1)) * 60; //Minute
				time += _ttoi(line.Mid(cuts.at(6) + 1, cuts.at(7) - cuts.at(6) - 1)); //Sekunde

				CString mili = (line.Mid(cuts.at(7) + 1, cuts.at(8) - cuts.at(7) - 1)) + L"000"; //milisenkundenwert gleich lang wie in Piezo

				Timestamp tmp;
				tmp.hms = time;
				tmp.mili = _ttoi(mili);
				tmp.Inumber = _ttoi(Number);
				tmp.height = 0;
				m_allImages.push_back(tmp);
			}
			else {
				errorlines++;
			}
		}
		//m_missingImages.push_back(&m_allImages);
		std::copy(m_allImages.begin(), m_allImages.end(), std::back_inserter(m_ImageValues));
		if (m_allImages.size() == 0) //wenn Daten aus irgendeinem Grund sinnvoll aber nicht lesbar
			return -1;
		//m_missingImages.insert(m_missingImages.begin(), m_ImageValues.at(0));
		m_firstRealImageVal = m_allImages.at(0);

		recalcTime(&m_ImageValues);
		recalcTime(&m_allImages);
		return errorlines;
	}
	return -1;
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
	BOOL first = TRUE;
	for (Timestamp& tmp : m_missingImages) { //siehe oben nur für nicht verwendete Bilder
		if (first) { //erstes Bild dient nur zur Synchronisation, führt aber sonst zu Fehlern
			first = FALSE;
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
	synced = TRUE;
}

void PiezoImageTime::resetOffset() {
	recalcTime(&m_ImageValues); //setzen auf 0
	recalcTime(&m_missingImages);
	recalcTime(&m_allImages);

	synced = FALSE;
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
	BOOL first = TRUE;
	for (Timestamp& tmp : m_missingImages) { //siehe oben nur für nicht verwendete Bilder
		if (first) { //erstes Bild dient nur zur Synchronisation, führt aber sonst zu Fehlern
			first = FALSE;
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
			/*CString out;
			out.Format("%d", tmp.Inumber);*/
			if (timePiezoDif > 100000.0) {
				CLog::Log(CLog::eWarning, L"MosaikbildDlg", L"timePiezoDif very high at: Inum: %d, Piezostep(counter) %d, Before: %f, After %f, timePiezoDif: %f, timeImageDif %f", tmp.Inumber, counter, before.calcJustMili(), after.calcJustMili(), timePiezoDif, timeImageDif);
			}
			/*else {
				CLog::Log(CLog::eWarning, L"MosaikbildDlg", L"Inum: %d, Counter %d, Before: %f, After %f, timePiezoDif: %f, timeImageDif %f", tmp.Inumber, counter, before.calcJustMili(), after.calcJustMili(), timePiezoDif, timeImageDif);

			}*/
		}
		/*if (tmp.height < m_minPiezoHeight || tmp.height > m_maxPiezoHeight) {
		int stop = 0;
		}*/

	}
	synced = TRUE;
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
	m_minPiezoHeight = INT_MAX;
	m_maxPiezoHeight = INT_MIN;

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
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
class Timestamp {
public:
	int hms;
	double mili; //hohe Auflösung Milizeit für genaue Synchronisation
	double height; //wird bei Image erst bei Sync generiert
	int Inumber; //wird von Piezo ignoriert
	double calcJustMili() { return double(hms * 1000 * 1000) + mili; }
};


/*
wichtig: benötigte Merkmale der Dateien
Piezodaten: <Datum>T<Uhrzeit>.<Mili>;<Höhe>
Bildparameter: # in Zeile über Nutzdaten
	alle Daten durch Tab getrennt


*/

//bei missing Images +1 (da Element 0 nur zur Synchronisation)
class PiezoImageTime
{
public:
	PiezoImageTime();
	~PiezoImageTime();
	int readPiezo(CString file);
	int readImageTime(CString file);
	void giveImageFilterByNames(std::vector<std::wstring> tmp);
	void recalcTime(std::vector <Timestamp>* timeVect);
	void recalcTime(std::vector<Timestamp>* timeVect, int offsetHms, double OffsetMili);
	void recalcHeights();
	double getAvergaeHeight() { return m_avergaHeight; };
	void calcAvergaeHeight();
	UINT getPiezoValCount() { return UINT(m_PiezoValues.size()); };
	UINT getAllImageValCount() { return UINT(m_allImages.size()); }
	UINT getImageValCount() { return UINT(m_ImageValues.size()); };
	UINT getMissingImageValCount() { return UINT(m_missingImages.size() - 1); };
	double getImageHeightAt(int i) const { return m_ImageValues.at(i).height; };
	double getPiezoHeightAt(int i) const { return m_PiezoValues.at(i).height; };
	double getMissingImageHeightAt(int i) const { return m_missingImages.at(i + 1).height; };
	int getImageNumber(int i) const { return m_ImageValues.at(i).Inumber; }
	double getImageMiliAt(int i) { return m_ImageValues.at(i).mili; };
	int getImageHmsAt(int i) { return m_ImageValues.at(i).hms; }
	double getPiezoMiliAt(int i) { return m_PiezoValues.at(i).mili; };
	int getPiezoHmsAt(int i) { return m_PiezoValues.at(i).hms; };
	int getMissingImageNumber(int i) const { return m_missingImages.at(i + 1).Inumber; };
	int getMissingImageHmsAt(int i) { return m_missingImages.at(i + 1).hms; };
	double getMissingImageMiliAt(int i) { return m_missingImages.at(i + 1).mili; };
	BOOL getSynced() { return synced; };
	double getMaxPiezoHeight() const { return m_maxPiezoHeight; };
	double getMinPiezoHeight() const { return m_minPiezoHeight; };
	double getPiezoDif() const { return m_PiezoDif; };
	int ret_ValueCountTime(int time);
	void syncManualOffset(double OffsetJustMili);
	void resetOffset();
	double getOffset(int Pstep, int Inum);
	void syncImagePiezoHeight(int Pstep, int Inum);
	double getFirstRealPiezoValjMili() { return m_firstRealPiezoVal.calcJustMili(); };
	double getFirstRealImageValjMili() { return m_firstRealImageVal.calcJustMili(); };

private:
	std::vector <Timestamp> m_PiezoValues;
	std::vector <Timestamp> m_ImageValues;
	std::vector <Timestamp> m_missingImages;
	std::vector <Timestamp> m_allImages;
	std::vector <int> m_iImageFilter;
	Timestamp m_firstRealPiezoVal;
	Timestamp m_firstRealImageVal;
	double m_maxPiezoHeight;
	double m_minPiezoHeight;
	double m_PiezoDif;
	double m_avergaHeight;
	BOOL synced = FALSE;
protected:
};

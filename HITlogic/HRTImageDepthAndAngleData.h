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
class CHRTImageDepthAndAngleData
{
public:
	CHRTImageDepthAndAngleData();
	~CHRTImageDepthAndAngleData();
	double getImageDepthAt(int i) const;
	//double getImageAngleAt(int i);
	double getImageInclination(int i) const;
	void setCombined(int i, double depth);// , double angle);
	void setImageDepth(int i, double depth);
	//void setImageAngle(int i, double angle);
	void setCount(int i); //should be used for performance reasons (before setters)
	int getCount() const;
	bool isAvailable() const; //should be called before using getters
	bool hasInclination() const;	//Freigabe Wert für schräge Bilder bestimmen
	void setInclination(bool val);

	CHRTImageDepthAndAngleData filter(std::list<size_t>) const;

	void setPiezoFilePath(std::wstring path); //optional !!!
	void setSnpPath(std::wstring path);
	std::wstring getPiezoFilePath() const;
	std::wstring getPiezoSnpPath() const;

	double getMaxDepth() const;
	double getMinDepth() const;
	double getMinInclination() const;	//Vorsicht: wenig verwenden da schlechte Laufzeit

	void depthMinus(double num);
private:
	std::vector <double> m_ImageDepths;
	//std::vector <double> m_ImageAngles;
	bool m_hasInclination = false;
	int m_iValueCount;
	std::wstring m_filePath;
	std::wstring m_snpPath;
};


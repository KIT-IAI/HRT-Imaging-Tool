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
class C3DCloud
{
public:
	enum class EWriteMode {	//Schreibmodus: pcd unterstützt alle, ply kann nicht komprimieren
		eASCII = 0,
		eBinary = 1,
		eBinaryCompressed = 2
	};

	C3DCloud();
	//C3DCloud(C3DCloud && fromMove);
	~C3DCloud();
	void add(float x, float y, float z, float intensity);
	void reserve(size_t size);
	/*für spätere gerasterte Punktwolken: isOrganized dann true
	void setOrganizedHW(int height, int width = 1) --> verwenden Reserve Funktion!;
	*/
	void savePCD(const std::wstring_view file_name, EWriteMode mode = EWriteMode::eBinary) const;
	void savePLY(const std::wstring_view file_name, EWriteMode mode = EWriteMode::eBinary) const;
	void Clone(const C3DCloud& ref);

private:
	class Impl;
	std::shared_ptr<Impl> pimpl;
};


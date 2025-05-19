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

#include <string>
#include "IIO_Defines.h"


class CImageExporter
{
private:
	CImageExporter(void);
	~CImageExporter(void);
	static bool ExportImageTif(std::wstring filename, int bit_depth, size_t height, size_t width, const void* const data, IIO_DATA_TYPE in_format, IIO_TIF_COMPRESSION flags);
	static bool ExportImageSeriesTif(std::wstring filename, int* bit_depth, size_t* height, size_t* width, void** data, int images_count, IIO_DATA_TYPE* in_format, IIO_TIF_COMPRESSION flags);

public:
	static bool ExportImage(std::wstring filename, int bit_depth, size_t height, size_t width, const void* const data, IIO_DATA_TYPE in_format, IIO_FILE_TYPE file_format, IIO_TIF_COMPRESSION flags);
	static bool ExportImageSeries(std::wstring filename, int* bit_depth, size_t* height, size_t* width, void** data, int images_count, IIO_DATA_TYPE* in_format, IIO_FILE_TYPE file_format, IIO_TIF_COMPRESSION flags);

};
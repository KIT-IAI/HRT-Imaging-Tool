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

#include <tiffio.h>

#include "IIO_Defines.h"

class CImageImporter
{
private:
	CImageImporter(void) = delete;
	~CImageImporter(void) = delete;
	static bool ImportPng(std::wstring filename, IIO_DATA_TYPE& format, int& bit_depth, size_t& height, size_t& width, void*& data);
	static bool ImportTif(std::wstring filename, IIO_DATA_TYPE& format, int& bit_depth, size_t& height, size_t& width, void*& data);
	static bool ImportJpg(std::wstring filename, IIO_DATA_TYPE& format, int& bit_depth, size_t& height, size_t& width, void*& data);
	static bool ImportMultiPageTif(const std::wstring& filename, IIO_DATA_TYPE& format, int& bit_depth, size_t& height, size_t& width, size_t& images_count, void**& data);

	static bool RetrieveMultiPageTifProperties(TIFF* tif, uint32_t& imageLength, uint32_t& imageWidth, uint16_t& bitsPerSample, uint16_t& samplesPerPixel, uint16_t& sampleFormat, uint16_t& photometric, uint32_t& tileWidth, uint32_t& tileLength, bool& tiledImage, size_t& pageCount);
	static bool ReadMultiPageTifImageData(TIFF* tif, uint32_t imageLength, uint32_t imageWidth, uint16_t bitsPerSample, uint16_t samplesPerPixel, uint16_t sampleFormat, uint16_t photometric, uint32_t tileWidth, uint32_t tileLength, bool tiledImage, size_t pageCount, void**& data);

	static void HandleTiffError(const char* module, const char* fmt, va_list ap);
	static bool bIsTiffErrorHandlerSet;

public:
	static bool ImportImage(std::wstring filename, IIO_DATA_TYPE& format, int& bit_depth, size_t& height, size_t& width, void*& data);
	static bool ImportImageSeries(const std::wstring& filename, IIO_DATA_TYPE& format, int& bit_depth, size_t& height, size_t& width, size_t& images_count, void**& data);

};

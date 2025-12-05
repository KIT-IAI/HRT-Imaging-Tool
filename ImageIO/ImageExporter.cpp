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
#include "ImageExporter.h"

#include "IIO_Defines.h"



CImageExporter::CImageExporter(void)
{
}

CImageExporter::~CImageExporter(void)
{
}

bool CImageExporter::ExportImageTif(std::wstring filename, int bit_depth, size_t height, size_t width, const void* const data, IIO_DATA_TYPE in_format, IIO_TIF_COMPRESSION flags)
{
	uint16_t BitsPerSample = static_cast<uint16_t>(bit_depth), SamplesPerPixel, SampleFormat, photo, compression;
	switch (in_format)
	{
	case IIO_DATA_TYPE::RGB_UNSIGNED:
		SamplesPerPixel=3;
		SampleFormat=SAMPLEFORMAT_UINT;
		photo=PHOTOMETRIC_RGB;
		break;;
	case IIO_DATA_TYPE::RGBA:
		return false;
	case IIO_DATA_TYPE::GRAY_UNSIGNED:
		SamplesPerPixel=1;
		SampleFormat=SAMPLEFORMAT_UINT;
		photo=PHOTOMETRIC_MINISBLACK;
		break;;
	case IIO_DATA_TYPE::GRAY_SIGNED:
		SamplesPerPixel=1;
		SampleFormat=SAMPLEFORMAT_INT;
		photo=PHOTOMETRIC_MINISBLACK;
		break;;
	case IIO_DATA_TYPE::GRAY_FLOAT:
		SamplesPerPixel=1;
		SampleFormat=SAMPLEFORMAT_IEEEFP;
		photo=PHOTOMETRIC_MINISBLACK;
		break;;
	case IIO_DATA_TYPE::GRAYA:
		return false;
	case IIO_DATA_TYPE::RGB_SIGNED:
		SamplesPerPixel=3;
		SampleFormat=SAMPLEFORMAT_INT;
		photo=PHOTOMETRIC_RGB;
		break;;
	case IIO_DATA_TYPE::RGB_FLOAT:
		SamplesPerPixel=3;
		SampleFormat=SAMPLEFORMAT_IEEEFP;
		photo=PHOTOMETRIC_RGB;
		break;;
	case IIO_DATA_TYPE::COMPLEX_SIGNED:
		SamplesPerPixel = 1;
		SampleFormat = SAMPLEFORMAT_COMPLEXINT;
		photo = PHOTOMETRIC_MINISBLACK;
		break;;
	case IIO_DATA_TYPE::COMPLEX_FLOAT:
		SamplesPerPixel = 1;
		SampleFormat = SAMPLEFORMAT_COMPLEXIEEEFP;
		photo = PHOTOMETRIC_MINISBLACK;
		break;;
	default:
		return false;
	}
	switch(flags)
	{
	case IIO_TIF_COMPRESSION::NONE:
		compression = COMPRESSION_NONE;
		break;;
	case IIO_TIF_COMPRESSION::LZW:
		compression = COMPRESSION_LZW;
		break;;
	case IIO_TIF_COMPRESSION::JPEG:
		compression = COMPRESSION_JPEG;
		break;;
	case IIO_TIF_COMPRESSION::PACKBITS:
		compression = COMPRESSION_PACKBITS;
		break;;
	case IIO_TIF_COMPRESSION::DEFLATE:
		compression = COMPRESSION_DEFLATE;
		break;;
	default:
		return false;
	}

	char* ascii = new char[filename.size() + 1];
	size_t retval;
	wcstombs_s( &retval,ascii,filename.size() + 1, filename.c_str(), filename.size());
	TIFF *out=TIFFOpen(ascii, "w");
	delete[] ascii;
	if(!out)
	{
		return false;
	}
	if(TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width)==0){TIFFClose(out); return false;} 
	if(TIFFSetField(out, TIFFTAG_IMAGELENGTH, height)==0){TIFFClose(out); return false;} 
	if(TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, SamplesPerPixel)==0){TIFFClose(out); return false;} 
	if(TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, BitsPerSample)==0){TIFFClose(out); return false;} 
	if(TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT)==0){TIFFClose(out); return false;} 
	if(TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG)==0){TIFFClose(out); return false;} 
	if(TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SampleFormat)==0){TIFFClose(out); return false;} 
	if(TIFFSetField(out, TIFFTAG_PHOTOMETRIC, photo)==0){TIFFClose(out); return false;} 
	if(TIFFSetField(out, TIFFTAG_COMPRESSION, compression)==0){TIFFClose(out); return false;} 
	if(TIFFSetField(out, TIFFTAG_SOFTWARE, "DIPLOM")==0){TIFFClose(out); return false;} 
	unsigned char *buf = NULL;        // buffer used to store the row of pixel information for writing to file
	//    Allocating memory to store the pixels of current row
	if (TIFFScanlineSize(out))
	{
		buf =(unsigned char *)_TIFFmalloc(TIFFScanlineSize(out));
	}
	else
	{
		TIFFClose(out); 
		return false;
	}

	// We set the strip size of the file to be size of one row of pixels
	if (TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, static_cast<uint32_t>(TIFFScanlineSize(out)))) == 0)
	{
		_TIFFfree(buf);
		TIFFClose(out);
		return false;
	}

	// Now writing image to the file one strip at a time
	for (uint32_t row = 0; row < height; row++)
	{
		memcpy(buf, &((unsigned char*)data)[row*TIFFScanlineSize(out)], TIFFScanlineSize(out));   
		if (TIFFWriteScanline(out, buf, row, 0) < 0)
		{
			 _TIFFfree(buf);
			TIFFClose(out); 
			return false;
		}
	}

	_TIFFfree(buf);
	TIFFClose(out); 
	return true;
}

bool CImageExporter::ExportImage(std::wstring filename, int bit_depth, size_t height, size_t width,const void* const data, IIO_DATA_TYPE in_format, IIO_FILE_TYPE file_format, IIO_TIF_COMPRESSION flags)
{
	if(file_format== IIO_FILE_TYPE::TIF)
	{
		return ExportImageTif(filename,bit_depth,height,width,data,in_format,flags);
	}
	return false;
}
bool CImageExporter::ExportImageSeries(std::wstring filename, int* bit_depth, size_t* height, size_t* width, void** data, int images_count, IIO_DATA_TYPE* in_format, IIO_FILE_TYPE file_format, IIO_TIF_COMPRESSION flags)
{
	if(file_format== IIO_FILE_TYPE::TIF)
	{
		return ExportImageSeriesTif(filename,bit_depth,height,width,data, images_count,in_format,flags);
	}
	return false;
}
bool CImageExporter::ExportImageSeriesTif(std::wstring filename, int* bit_depth, size_t* height, size_t* width, void** data, int images_count, IIO_DATA_TYPE* in_format, IIO_TIF_COMPRESSION flags)
{

	char* ascii = new char[filename.size() + 1];
	size_t retval;
	wcstombs_s( &retval,ascii,filename.size() + 1, filename.c_str(), filename.size());
	TIFF *out=TIFFOpen(ascii, "w");
	delete[] ascii;
	if(!out)
	{
		return false;
	}
	for(int ii = 0;ii<images_count;ii++)
	{
		uint16_t BitsPerSample = static_cast<uint16_t>(bit_depth[ii]), SamplesPerPixel, SampleFormat, photo, compression;
		switch (in_format[ii])
		{
		case IIO_DATA_TYPE::RGB_UNSIGNED:
			SamplesPerPixel=3;
			SampleFormat=SAMPLEFORMAT_UINT;
			photo=PHOTOMETRIC_RGB;
			break;;
		case IIO_DATA_TYPE::RGBA:
			return false;
		case IIO_DATA_TYPE::GRAY_UNSIGNED:
			SamplesPerPixel=1;
			SampleFormat=SAMPLEFORMAT_UINT;
			photo=PHOTOMETRIC_MINISBLACK;
			break;;
		case IIO_DATA_TYPE::GRAY_SIGNED:
			SamplesPerPixel=1;
			SampleFormat=SAMPLEFORMAT_INT;
			photo=PHOTOMETRIC_MINISBLACK;
			break;;
		case IIO_DATA_TYPE::GRAY_FLOAT:
			SamplesPerPixel=1;
			SampleFormat=SAMPLEFORMAT_IEEEFP;
			photo=PHOTOMETRIC_MINISBLACK;
			break;;
		case IIO_DATA_TYPE::GRAYA:
			return false;
		case IIO_DATA_TYPE::RGB_SIGNED:
			SamplesPerPixel=3;
			SampleFormat=SAMPLEFORMAT_INT;
			photo=PHOTOMETRIC_RGB;
			break;;
		case IIO_DATA_TYPE::RGB_FLOAT:
			SamplesPerPixel=3;
			SampleFormat=SAMPLEFORMAT_IEEEFP;
			photo=PHOTOMETRIC_RGB;
			break;;
		default:
			return false;
		}
		switch(flags)
		{
		case IIO_TIF_COMPRESSION::NONE:
			compression = COMPRESSION_NONE;
			break;;
		case IIO_TIF_COMPRESSION::LZW:
			compression = COMPRESSION_LZW;
			break;;
		case IIO_TIF_COMPRESSION::JPEG:
			compression = COMPRESSION_JPEG;
			break;;
		case IIO_TIF_COMPRESSION::PACKBITS:
			compression = COMPRESSION_PACKBITS;
			break;;
		case IIO_TIF_COMPRESSION::DEFLATE:
			compression = COMPRESSION_DEFLATE;
			break;;
		default:
			return false;
		}
		if(TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width[ii])==0){TIFFClose(out); return false;} 
		if(TIFFSetField(out, TIFFTAG_IMAGELENGTH, height[ii])==0){TIFFClose(out); return false;} 
		if(TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, SamplesPerPixel)==0){TIFFClose(out); return false;} 
		if(TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, BitsPerSample)==0){TIFFClose(out); return false;} 
		if(TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT)==0){TIFFClose(out); return false;} 
		if(TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG)==0){TIFFClose(out); return false;} 
		if(TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SampleFormat)==0){TIFFClose(out); return false;} 
		if(TIFFSetField(out, TIFFTAG_PHOTOMETRIC, photo)==0){TIFFClose(out); return false;} 
		if(TIFFSetField(out, TIFFTAG_COMPRESSION, compression)==0){TIFFClose(out); return false;} 
		if(TIFFSetField(out, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE )==0){TIFFClose(out); return false;} 
		if(TIFFSetField(out, TIFFTAG_PAGENUMBER, ii+1)==0){TIFFClose(out); return false;} 
		char name[10];
		sprintf_s(name,"%d",ii+1);
		if(TIFFSetField(out, TIFFTAG_PAGENAME, name)==0){TIFFClose(out); return false;} 
		if(TIFFSetField(out, TIFFTAG_IMAGEDESCRIPTION, name)==0){TIFFClose(out); return false;} 
		if(TIFFSetField(out, TIFFTAG_SOFTWARE, "DIPLOM")==0){TIFFClose(out); return false;} 

		unsigned char *buf = NULL;        // buffer used to store the row of pixel information for writing to file
		//    Allocating memory to store the pixels of current row
		if (TIFFScanlineSize(out))
		{
			buf =(unsigned char *)_TIFFmalloc(TIFFScanlineSize(out));
		}
		else
		{
			TIFFClose(out); 
			return false;
		}

		// We set the strip size of the file to be size of one row of pixels
		if (TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, static_cast<uint32_t>(TIFFScanlineSize(out)))) == 0)
		{
			_TIFFfree(buf);
			TIFFClose(out);
			return false;
		}

		// Now writing image to the file one strip at a time
		for (uint32_t row = 0; row < height[ii]; row++)
		{
			memcpy(buf, &((unsigned char*)data[ii])[row*TIFFScanlineSize(out)], TIFFScanlineSize(out));   
			if (TIFFWriteScanline(out, buf, row, 0) < 0)
			{
				 _TIFFfree(buf);
				TIFFClose(out); 
				return false;
			}
		}

		_TIFFfree(buf);
		TIFFWriteDirectory(out);
	}
	TIFFClose(out); 
	return true;
}
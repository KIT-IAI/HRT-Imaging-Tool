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



#include "targetver.h"

//libpng
#include "png.h"

//libtiff
#include <tiffio.h>
#include <jpeglib.h>

#include <algorithm>
#include <cassert>
#include <exception>

#include "ImageImporter.h"
#include "IIO_Defines.h"
#include "ImageIOException.h"

bool CImageImporter::bIsTiffErrorHandlerSet = false;

bool CImageImporter::ImportImage(std::wstring filename, IIO_DATA_TYPE& format, int& bit_depth, size_t& height, size_t& width, void*& data)
{
	static const std::wstring png = L"png";
	static const std::wstring tif = L"tif";
	static const std::wstring jpg = L"jpg";
	static const std::wstring tiff = L"tiff";

	if(filename.size()>4 && (std::equal(tif.rbegin(), tif.rend(), filename.rbegin()) || std::equal(tiff.rbegin(), tiff.rend(), filename.rbegin())))
	{
		return ImportTif(filename,format,bit_depth,height,width,data);
	}
	else if(filename.size()>4 && std::equal(png.rbegin(), png.rend(), filename.rbegin()))
	{
		return ImportPng(filename,format,bit_depth,height,width,data);
	}
	else if(filename.size()>4 && std::equal(jpg.rbegin(), jpg.rend(), filename.rbegin()))
	{
		return ImportJpg(filename,format,bit_depth,height,width,data);
	}
	throw CImageIOException(std::wstring(filename), CImageIOException::eUnsupportedFormatException);
}

bool CImageImporter::ImportTif(std::wstring filename, IIO_DATA_TYPE& format, int& bit_depth, size_t& height, size_t& width, void*& data)
{
	if (!bIsTiffErrorHandlerSet)
	{
		TIFFSetErrorHandler(HandleTiffError);
		bIsTiffErrorHandlerSet = true;
	}

	char* ascii = new char[filename.size() + 1];
	size_t retval;
	wcstombs_s( &retval,ascii,filename.size() + 1, filename.c_str(), filename.size());
	TIFF *tif=TIFFOpen(ascii, "r");
	delete[] ascii;
	if(!tif)
	{
		return false;
	}
	uint32_t w, h, tileWidth, tileLength;
	uint16_t BitsPerSample, SamplesPerPixel, SampleFormat, photo;
	bool tiledImage = true;
	if (TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w) == 0) { TIFFClose(tif); return false; }
	if (TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h) == 0) { TIFFClose(tif); return false; }
	if (TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &BitsPerSample) == 0) { BitsPerSample = 1; }
	if (TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &SamplesPerPixel) == 0) { SamplesPerPixel = 1; }
	if (TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &SampleFormat) == 0) { SampleFormat = SAMPLEFORMAT_UINT; }
	if (TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photo) == 0) { TIFFClose(tif); return false; }
	if (TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tileWidth) == 0) { tiledImage = false; }
	if (TIFFGetField(tif, TIFFTAG_TILELENGTH, &tileLength) == 0) { tiledImage = false; }

	if(((SamplesPerPixel == 1 && photo != PHOTOMETRIC_MINISBLACK) && (SamplesPerPixel == 1 && photo != PHOTOMETRIC_MINISWHITE)) || (SamplesPerPixel==3 && photo != PHOTOMETRIC_RGB))
	{
		TIFFClose(tif);
		return false;
	}


	if(!(SamplesPerPixel==1 || SamplesPerPixel==3))
	{
		TIFFClose(tif);
		return false;
	}

	switch(SampleFormat)
	{
	case SAMPLEFORMAT_UINT:
	case SAMPLEFORMAT_VOID:
		format= SamplesPerPixel==1?IIO_DATA_TYPE::GRAY_UNSIGNED:IIO_DATA_TYPE::RGB_UNSIGNED;
		break;;
	case SAMPLEFORMAT_INT:
		format= SamplesPerPixel==1?IIO_DATA_TYPE::GRAY_SIGNED:IIO_DATA_TYPE::RGB_SIGNED;
		break;;
	case SAMPLEFORMAT_IEEEFP:
		format= SamplesPerPixel==1?IIO_DATA_TYPE::GRAY_FLOAT:IIO_DATA_TYPE::RGB_FLOAT;
		break;;
	case SAMPLEFORMAT_COMPLEXINT:
		format = IIO_DATA_TYPE::COMPLEX_SIGNED;
		break;;
	case SAMPLEFORMAT_COMPLEXIEEEFP:
		format = IIO_DATA_TYPE::COMPLEX_FLOAT;
		break;;
	default:
		TIFFClose(tif);
		return false;
	}

	width=w; height=h;

	tdata_t buf;
	
	/*The following fails for one, two and four bit tiffs, so what do? We need to get the packed data and unpack it, or pass it up as packed data and leave the consumer deal with this*/
	if(BitsPerSample<8)
	{
		TIFFClose(tif);
		return false;
	}
	data = new unsigned char[static_cast<size_t>((BitsPerSample/8))*SamplesPerPixel*w*h];
	bit_depth=BitsPerSample;

	if(tiledImage)
	{
		tmsize_t tilesize=TIFFTileSize(tif);
		buf = _TIFFmalloc(tilesize);
		for (uint32_t y = 0; y < h; y += tileLength)
		{
			for (uint32_t x = 0; x < w; x += tileWidth)
			{
				if (TIFFReadTile(tif, buf, x, y ,0, 0)==-1)
				{
					_TIFFfree(buf);	
					TIFFClose(tif);
					return false;
				}
				for (uint32_t z = 0; z < tileLength; z++)
				{
					if(z+y<h)
					{
						memcpy(&((unsigned char*)data)[(y+z)*w*(BitsPerSample/8)*SamplesPerPixel+x*(BitsPerSample/8)*SamplesPerPixel],&((unsigned char*)buf)[z*(tilesize/tileLength)],std::min(static_cast<unsigned int>(tilesize/tileWidth),(w-x)*(BitsPerSample/8)*SamplesPerPixel));
					}
				}
			}
		}
		_TIFFfree(buf);
	}
	else
	{
		tmsize_t scanlinesize=TIFFScanlineSize(tif);
		buf = _TIFFmalloc(scanlinesize);
		for (uint32_t row = 0; row < h; row++)
		{
			if (TIFFReadScanline(tif, buf, row)==-1)
			{
				_TIFFfree(buf);	
				TIFFClose(tif);
				return false;
			}
			memcpy(&(((unsigned char*)data)[static_cast<tmsize_t>(row)*scanlinesize]),(unsigned char*)buf,scanlinesize);
		}
		_TIFFfree(buf);
	}
	TIFFClose(tif);

	if(SamplesPerPixel == 1 && photo == PHOTOMETRIC_MINISWHITE)
	{
		for (uint16_t count = 0; count < (BitsPerSample/8)*SamplesPerPixel*w*h; count++)
		{
			((char*)data)[count] = ~((char*)data)[count];
		}
	}

	return true;
}

bool CImageImporter::ImportJpg(std::wstring filename, IIO_DATA_TYPE& format, int& bit_depth, size_t& height, size_t& width, void*& data)
{
		//doc: https://github.com/libjpeg-turbo/libjpeg-turbo/blob/main/libjpeg.txt

		struct jpeg_decompress_struct cinfo;		//info Object
		struct jpeg_error_mgr err;          //the error handler

		/* More stuff */
		FILE* infile;      /* source file */
		JSAMPARRAY buffer;      /* Output row buffer */
		int row_stride;     /* physical row width in output buffer */

		_wfopen_s(&infile, filename.c_str(), L"rb");
		if (!infile)
		{
			throw CImageIOException(std::wstring(filename), CImageIOException::ePngOpeningError, L"Could not open file.");
		}

		/* Step 1: allocate and initialize JPEG decompression object */

		/* We set up the normal JPEG error routines, then override error_exit. */
		cinfo.err = jpeg_std_error(&err);
		
		/* Now we can initialize the JPEG decompression object. */
		jpeg_create_decompress(&cinfo);

		/* Step 2: specify data source (eg, a file) */
		jpeg_stdio_src(&cinfo, infile);

		/* Step 3: read file parameters with jpeg_read_header() */
		(void)jpeg_read_header(& cinfo, true);


		/* Step 4: set parameters for decompression */
		/*set wanted color space -> only grayscale*/
		cinfo.out_color_space = J_COLOR_SPACE::JCS_GRAYSCALE;
		format = IIO_DATA_TYPE::GRAY_UNSIGNED;

		 /* Step 5: Start decompressor */
		(void)jpeg_start_decompress(&cinfo);

		//read parameters
		width = cinfo.output_width;
		height = cinfo.output_height;
		bit_depth = 8; //always

		row_stride = cinfo.output_width * cinfo.output_components;
		/* Make a one-row-high sample array that will go away when done with image */
		buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
		
		data = new unsigned char[cinfo.output_width * cinfo.output_height * cinfo.output_components];
		long counter = 0;

		//step 6, read the image line by line
		while (cinfo.output_scanline < cinfo.output_height) {
			jpeg_read_scanlines(&cinfo, buffer, 1);
			memcpy((unsigned char*)data + counter, buffer[0], row_stride);
			counter += row_stride;
		}

		/* Step 7: Finish decompression */
		(void)jpeg_finish_decompress(&cinfo);
		
		/* Step 8: Release JPEG decompression object */
		/* This is an important step since it will release a good deal of memory. */
		jpeg_destroy_decompress(&cinfo);

		fclose(infile);
		/* And we're done! */
		return true;
}

void CImageImporter::HandleTiffError(const char * module, const char * fmt, va_list ap)
{
	const size_t nBufferSize = 100000;
	char buffer[nBufferSize];
	vsprintf_s(buffer, fmt, ap);
	
	std::string Module(module);
	std::string Message(buffer);
	std::string sErrorMessage = ("Module: " + Module + ": " + Message);
	std::wstring sWErrorMessage(sErrorMessage.begin(), sErrorMessage.end());

	throw CImageIOException(L"",CImageIOException::eTiffError, sWErrorMessage);
}

bool CImageImporter::ImportPng(std::wstring filename, IIO_DATA_TYPE& format, int& bit_depth, size_t& height, size_t& width, void*& data)
{
	png_byte header[8];
	png_structp png_ptr;
	png_infop info_ptr;
	png_byte color_type;
	//png_byte bit_depth;
	int number_of_passes;
	size_t y;
	png_bytep * row_pointers;

	FILE *fp;
	_wfopen_s(&fp,filename.c_str(), L"rb");
	if (!fp)
	{
		throw CImageIOException(std::wstring(filename), CImageIOException::ePngOpeningError, L"Could not open file.");
	}
	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8))
	{
		fclose(fp); 
		throw CImageIOException(std::wstring(filename), CImageIOException::ePngFileError, L"Not a valid PNG-File.");
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		fclose(fp);
		throw CImageIOException(std::wstring(filename), CImageIOException::ePngError, L"png_create_read_struct failed");
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		fclose(fp); 
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		throw CImageIOException(std::wstring(filename), CImageIOException::ePngError, L"png_create_info_struct failed.");
	}

//this works like a charm, but the compiler complains about it. There is no easy way around it, because libpng is a c libary (and not c++)
#pragma warning(push)
#pragma warning(disable: 4611)
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		fclose(fp); 
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		throw CImageIOException(std::wstring(filename), CImageIOException::ePngError, L"Error during init_io.");
	}
#pragma warning(pop)

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	number_of_passes = png_set_interlace_handling(png_ptr);

//this works like a charm, but the compiler complains about it. There is no easy way around it, because libpng is a c libary (and not c++)
#pragma warning(push)
#pragma warning(disable: 4611)
	/* read file */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		fclose(fp);
		throw CImageIOException(std::wstring(filename), CImageIOException::ePngError, L"Error during read_image.");
	}
#pragma warning(pop)

    if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
	{
		format=IIO_DATA_TYPE::RGB_UNSIGNED;
	}
    else if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGBA)
	{
		format=IIO_DATA_TYPE::RGBA;
	}
    else if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_GRAY)
	{
		format=IIO_DATA_TYPE::GRAY_UNSIGNED;
		if(bit_depth < 8)
		{
			png_set_expand_gray_1_2_4_to_8(png_ptr);
			bit_depth=8;
		}
	}
    else if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		format=IIO_DATA_TYPE::GRAYA;
		if(bit_depth < 8)
		{
			png_set_expand_gray_1_2_4_to_8(png_ptr);
			bit_depth=8;
		}
	}
    else if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_PALETTE)
	{
		if (png_get_bit_depth(png_ptr, info_ptr) == 8) 
		{
			bit_depth = 8;
			format = IIO_DATA_TYPE::GRAY_UNSIGNED;
		}
		else 
		{
			png_set_palette_to_rgb(png_ptr);
			format = IIO_DATA_TYPE::RGB_UNSIGNED;
		}
	}
	else 
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		fclose(fp);
		throw CImageIOException(std::wstring(filename), CImageIOException::ePngError, L"Unsupported Image format(" + std::to_wstring((int)png_get_color_type(png_ptr, info_ptr)) + L").");
	}


	png_read_update_info(png_ptr, info_ptr);


//this works like a charm, but the compiler complains about it. There is no easy way around it, because libpng is a c libary (and not c++)
#pragma warning(push)
#pragma warning(disable: 4611)
	/* read file */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		fclose(fp);
		throw CImageIOException(std::wstring(filename), CImageIOException::ePngError, L"Error during read_image.");
	}
#pragma warning(pop)




	size_t rowbytes=png_get_rowbytes(png_ptr,info_ptr);
	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	for (y=0; y<height; y++)
		row_pointers[y] = (png_byte*) malloc(rowbytes);

	png_read_image(png_ptr, row_pointers);

	data=new unsigned char[rowbytes*height];
	for (y=0; y<height; y++)
	{
		memcpy(&((unsigned char*)data)[rowbytes*y],row_pointers[y],rowbytes);
		free(row_pointers[y]);
	}

	free(row_pointers);
	png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
	fclose(fp);

	return true;
}

bool CImageImporter::ImportImageSeries(const std::wstring& filename, IIO_DATA_TYPE& format, int& bit_depth, size_t& height, size_t& width, size_t& images_count, void**& data)
{
	static const std::wstring tif = L"tif";
	static const std::wstring tiff = L"tiff";

	if (filename.size() > 4 && (std::equal(tif.rbegin(), tif.rend(), filename.rbegin()) || std::equal(tiff.rbegin(), tiff.rend(), filename.rbegin())))
	{
		return ImportMultiPageTif(filename, format, bit_depth, height, width, images_count, data);
	}
	throw CImageIOException(std::wstring(filename), CImageIOException::eUnsupportedFormatException);
}

bool CImageImporter::ImportMultiPageTif(const std::wstring& filename, IIO_DATA_TYPE& format, int& bit_depth, size_t& height, size_t& width, size_t& images_count, void**& data)
{
	if (!bIsTiffErrorHandlerSet)
	{
		TIFFSetErrorHandler(HandleTiffError);
		bIsTiffErrorHandlerSet = true;
	}

	TIFF* tif = TIFFOpenW(filename.c_str(), "r");
	if (!tif)
	{
		return false;
	}

	// in order to make memory handling (a little) easier, we first retrieve all
	// image properties before we start actually reading the image data ...

	uint32_t imageWidth, imageLength, tileWidth, tileLength;
	uint16_t bitsPerSample, samplesPerPixel, sampleFormat, photometric;
	bool tiledImage = true;
	size_t imageCount = 0;
	if (!RetrieveMultiPageTifProperties(tif, imageWidth, imageLength, bitsPerSample, samplesPerPixel, sampleFormat, photometric, tileWidth, tileLength, tiledImage, imageCount))
	{
		TIFFClose(tif);
		return false;
	}

	IIO_DATA_TYPE dataType;
	switch (sampleFormat)
	{
	case SAMPLEFORMAT_UINT:
	case SAMPLEFORMAT_VOID:
		dataType = samplesPerPixel == 1 ? IIO_DATA_TYPE::GRAY_UNSIGNED : IIO_DATA_TYPE::RGB_UNSIGNED;
		break;
	case SAMPLEFORMAT_INT:
		dataType = samplesPerPixel == 1 ? IIO_DATA_TYPE::GRAY_SIGNED : IIO_DATA_TYPE::RGB_SIGNED;
		break;
	case SAMPLEFORMAT_IEEEFP:
		dataType = samplesPerPixel == 1 ? IIO_DATA_TYPE::GRAY_FLOAT : IIO_DATA_TYPE::RGB_FLOAT;
		break;
	case SAMPLEFORMAT_COMPLEXINT:
		dataType = IIO_DATA_TYPE::COMPLEX_SIGNED;
		break;
	case SAMPLEFORMAT_COMPLEXIEEEFP:
		dataType = IIO_DATA_TYPE::COMPLEX_FLOAT;
		break;
	default:
		TIFFClose(tif);
		return false;
	}

	// ... and now we actually read the image data

	void** destBuffer = nullptr;
	if (!ReadMultiPageTifImageData(tif, imageLength, imageWidth, bitsPerSample, samplesPerPixel, sampleFormat, photometric, tileWidth, tileLength, tiledImage, imageCount, destBuffer))
	{
		TIFFClose(tif);
		return false;
	}

	TIFFClose(tif);

	format = dataType;
	width = imageWidth;
	height = imageLength;
	bit_depth = bitsPerSample;
	images_count = imageCount;
	data = destBuffer;

	return true;
}

bool CImageImporter::RetrieveMultiPageTifProperties(TIFF* tif, uint32_t& imageWidth, uint32_t& imageLength, uint16_t& bitsPerSample, uint16_t& samplesPerPixel, uint16_t& sampleFormat, uint16_t& photometric, uint32_t& tileWidth, uint32_t& tileLength, bool& tiledImage, size_t& pageCount)
{
	auto ret = TIFFSetDirectory(tif, 0);
	assert(ret == 1);

	tiledImage = true;
	pageCount = 0;
	do {
		if (pageCount == 0)
		{
			// retrieve and check properties for first image
			if (TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &imageWidth) == 0) { return false; }
			if (TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &imageLength) == 0) { return false; }
			if (TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample) == 0) { bitsPerSample = 1; }
			if (TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel) == 0) { samplesPerPixel = 1; }
			if (TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleFormat) == 0) { sampleFormat = SAMPLEFORMAT_UINT; }
			if (TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric) == 0) { return false; }
			if (TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tileWidth) == 0) { tiledImage = false; }
			if (TIFFGetField(tif, TIFFTAG_TILELENGTH, &tileLength) == 0) { tiledImage = false; }

			if (samplesPerPixel != 1 && samplesPerPixel != 3)
			{
				return false;
			}

			// ***** IMPORTANT NOTE *****
			// We allow a photometric interpretation value of PHOTOMETRIC_PALETTE here,
			// because we have cases where this happens (e.g. it is the default when
			// exporting TIFF files in IrfanView), but we don't actually import the
			// palette information! In effect, the palette index is taken as the pixel
			// intensity!
			if ((samplesPerPixel == 1 && photometric != PHOTOMETRIC_MINISBLACK && photometric != PHOTOMETRIC_MINISWHITE && photometric != PHOTOMETRIC_PALETTE) || (samplesPerPixel == 3 && photometric != PHOTOMETRIC_RGB))
			{
				return false;
			}

			if (sampleFormat != SAMPLEFORMAT_VOID && sampleFormat != SAMPLEFORMAT_UINT && sampleFormat != SAMPLEFORMAT_INT && sampleFormat != SAMPLEFORMAT_IEEEFP && sampleFormat != SAMPLEFORMAT_COMPLEXINT && sampleFormat != SAMPLEFORMAT_COMPLEXIEEEFP)
			{
				return false;
			}

			// the memory allocation code currently can't handle 1-, 2- and 4-bit images;
			// see comments in ImportTif() for more info
			if (bitsPerSample < 8)
			{
				return false;
			}
		}
		else
		{
			// retrieve and check properties for subsequent images;
			// for the time being, we want all images to have identical properties
			uint32_t imageWidth2, imageLength2, tileWidth2, tileLength2;
			uint16_t bitsPerSample2, samplesPerPixel2, sampleFormat2, photometric2;
			bool tiledImage2 = true;
			if (TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &imageWidth2) == 0) { return false; }
			if (TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &imageLength2) == 0) { return false; }
			if (TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample2) == 0) { bitsPerSample2 = 1; }
			if (TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel2) == 0) { samplesPerPixel2 = 1; }
			if (TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleFormat2) == 0) { sampleFormat2 = SAMPLEFORMAT_UINT; }
			if (TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric2) == 0) { return false; }
			if (TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tileWidth2) == 0) { tiledImage2 = false; }
			if (TIFFGetField(tif, TIFFTAG_TILELENGTH, &tileLength2) == 0) { tiledImage2 = false; }

			if ((imageWidth2 != imageWidth) || (imageLength2 != imageLength) || (bitsPerSample2 != bitsPerSample) || (samplesPerPixel2 != samplesPerPixel) || (sampleFormat2 != sampleFormat) || (photometric2 != photometric))
			{
				return false;
			}

			if ((tiledImage2 != tiledImage) || (tiledImage && (tileWidth2 != tileWidth)) || (tiledImage && (tileLength2 != tileLength)))
			{
				return false;
			}
		}
		pageCount++;
	} while (TIFFReadDirectory(tif));
	return true;
}

bool CImageImporter::ReadMultiPageTifImageData(TIFF* tif, uint32_t imageLength, uint32_t imageWidth, uint16_t bitsPerSample, uint16_t samplesPerPixel, uint16_t sampleFormat, uint16_t photometric, uint32_t tileWidth, uint32_t tileLength, bool tiledImage, size_t pageCount, void**& data)
{
	size_t bytesPerSample = bitsPerSample / 8;
	size_t bytesPerPixel = bytesPerSample * samplesPerPixel;
	size_t pixelsPerImage = imageWidth * imageLength;
	size_t bytesPerImage = bytesPerPixel * pixelsPerImage;

	data = new void*[pageCount];
	for (size_t i = 0; i < pageCount; i++)
	{
		data[i] = nullptr;
	}

	for (size_t i = 0; i < pageCount; i++)
	{
		auto ret = TIFFSetDirectory(tif, static_cast<tdir_t>(i));
		assert(ret == 1);

		data[i] = new unsigned char[bytesPerImage];
		if (tiledImage)
		{
			tmsize_t tileSize = TIFFTileSize(tif);
			tdata_t tileBuffer = _TIFFmalloc(tileSize);
			for (uint32_t y = 0; y < imageLength; y += tileLength)
			{
				for (uint32_t x = 0; x < imageWidth; x += tileWidth)
				{
					if (TIFFReadTile(tif, tileBuffer, x, y, 0, 0) == -1)
					{
						_TIFFfree(tileBuffer);
						TIFFClose(tif);
						for (size_t j = 0; j <= i; j++) delete[] data[j];
						delete[] data;
						data = nullptr;
						return false;
					}
					for (size_t z = 0; z < tileLength; z++)
					{
						if (z + y < imageLength)
						{
							memcpy(&(reinterpret_cast<unsigned char*>(data[i])[(y + z) * imageWidth * bytesPerPixel + x * bytesPerPixel]), &(reinterpret_cast<unsigned char*>(tileBuffer)[z * (tileSize / tileLength)]), std::min(static_cast<size_t>(tileSize / tileWidth), (imageWidth - x) * bytesPerPixel));
						}
					}
				}
			}
			_TIFFfree(tileBuffer);
		}
		else
		{
			tmsize_t scanlineSize = TIFFScanlineSize(tif);
			tdata_t scanlineBuffer = _TIFFmalloc(scanlineSize);
			for (uint32_t row = 0; row < imageLength; row++)
			{
				if (TIFFReadScanline(tif, scanlineBuffer, row) == -1)
				{
					_TIFFfree(scanlineBuffer);
					TIFFClose(tif);
					for (size_t j = 0; j <= i; j++) delete[] data[j];
					delete[] data;
					data = nullptr;
					return false;
				}
				memcpy(&(reinterpret_cast<unsigned char*>(data[i])[row * scanlineSize]), reinterpret_cast<unsigned char*>(scanlineBuffer), scanlineSize);
			}
			_TIFFfree(scanlineBuffer);
		}
	}

	return true;
}

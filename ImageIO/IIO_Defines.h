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

enum class IIO_DATA_TYPE
{
	RGB_UNSIGNED = 0,
	RGBA,
	GRAY_UNSIGNED,
	GRAY_SIGNED,
	GRAY_FLOAT,
	GRAYA,
	RGB_SIGNED,
	RGB_FLOAT,
	COMPLEX_SIGNED,
	COMPLEX_FLOAT
};

enum class IIO_FILE_TYPE
{
	TIF = 0,
	PNG
};

enum class IIO_TIF_COMPRESSION
{
	NONE = 0,
	LZW,
	JPEG,
	PACKBITS,
	DEFLATE,
	MASK = 0xf
};

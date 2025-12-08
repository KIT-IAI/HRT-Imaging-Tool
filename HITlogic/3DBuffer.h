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

#include "StlImage.h"

//#ifdef HIT_STLIMAGE_USE_CUSTOM_HEAP_MANAGEMENT
#ifdef _WIN32
#include "StlImageAlignedAllocator.h"
#endif

struct C3DBufferSize
{
	long long x, y, z;
	bool operator==(const C3DBufferSize& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
	bool operator!=(const C3DBufferSize& rhs) const { return !(*this == rhs); }
	C3DBufferSize()
	{
		x = 0;
		y = 0;
		z = 0;
	}
	C3DBufferSize(long long x_, long long y_, long long z_)
	{
		x = x_;
		y = y_;
		z = z_;
	}

	C3DBufferSize& operator+=(const C3DBufferSize& rhs) {
		x += rhs.x;		//same behavior like in CSize
		y += rhs.y;
		z += rhs.z;
		return *this;
	};
};
typedef C3DBufferSize C3DBufferPoint;
struct C3DBufferRect
{
	long long x, y, z;
	long long sx, sy, sz;
	C3DBufferRect() = default;
	C3DBufferRect(long long x_start, long long y_start, long long z_start, long long x_end, long long y_end, long long z_end)
	{
		x = x_start;
		y = y_start;
		z = z_start;
		sx = x_end - x_start;
		sy = y_end - y_start;
		sz = z_end - z_start;
	}
	C3DBufferRect(C3DBufferPoint topleft, C3DBufferSize size)
	{
		x = topleft.x;
		y = topleft.y;
		z = topleft.z;
		sx = size.x;
		sy = size.y;
		sz = size.z;
	}
	C3DBufferSize Size() const
	{
		return { sx, sy, sz };
	}
	bool IsRectEmpty() const
	{
		return sx == 0 || sy == 0 || sz == 0;
	}
	void MoveToXYZ(long long x_, long long y_, long long z_)
	{
		x = x_;
		y = y_;
		z = z_;
	}
};

template <typename T> class C3DBuffer
{
	template<typename> friend class C3DBuffer;
//#ifdef HIT_STLIMAGE_USE_CUSTOM_HEAP_MANAGEMENT
#ifdef _WIN32
	template<typename, size_t> friend struct StlImageAlignedAllocator;
#endif

protected:
//#ifdef HIT_STLIMAGE_USE_CUSTOM_HEAP_MANAGEMENT
#ifdef _WIN32
	std::vector<T, StlImageAlignedAllocator<T, 512>> m_data;
#else
	std::vector<T> m_data;
#endif
	C3DBufferSize m_size;

public:
	C3DBuffer();
	C3DBuffer(C3DBuffer<T>&& from) noexcept;
	explicit C3DBuffer(const C3DBuffer<T>& from);

	template<typename T2> explicit C3DBuffer(const C3DBuffer<T2>& from);
	//todo stlImage to 3dBuffer construktor

	~C3DBuffer();

	void Alloc(C3DBufferSize size);
	void Free();

	void Clear(T value = T());
	void Clear(size_t _x, size_t _y, size_t _z, size_t sx, size_t sy, size_t sz, T value = T());

	//Put3D => CopyAtOffset
	void CopyAtOffset(C3DBufferPoint p, const C3DBuffer<T>& buf);
	void CopyAtOffset(C3DBufferPoint p, const C3DBuffer<T>& buf, C3DBufferRect r);
	void CopyAtOffset(C3DBufferPoint p, const StlImage<T>& img);
	void AddAtOffset(C3DBufferPoint p, C3DBuffer<T>& buf);
	void AddLineAtOffset(C3DBufferPoint p, std::vector<T>& buf);
	void AddAtOffset(C3DBufferPoint p, C3DBuffer<T>& buf, C3DBufferRect r);
	void PutInStlImage(StlImage<T>& img);

	C3DBuffer<T> operator+(const C3DBuffer<T>& rhs);
	C3DBuffer<T> operator-(const C3DBuffer<T>& rhs);
	C3DBuffer<T> operator*(const C3DBuffer<T>& rhs);
	C3DBuffer<T> operator/(const C3DBuffer<T>& rhs);

	C3DBuffer<T>& operator+=(const C3DBuffer<T>& rhs);
	C3DBuffer<T>& operator-=(const C3DBuffer<T>& rhs);
	C3DBuffer<T>& operator*=(const C3DBuffer<T>& rhs);
	C3DBuffer<T>& operator/=(const C3DBuffer<T>& rhs);

	C3DBuffer<T> operator+(const T rhs) const;
	C3DBuffer<T> operator-(const T rhs) const;
	C3DBuffer<T> operator*(const T rhs) const;
	C3DBuffer<T> operator/(const T rhs) const;

	C3DBuffer<T>& operator+=(const T rhs);
	C3DBuffer<T>& operator-=(const T rhs);
	C3DBuffer<T>& operator*=(const T rhs);
	C3DBuffer<T>& operator/=(const T rhs);

	C3DBuffer<T>& operator=(const C3DBuffer<T>& rhs);
	//Clone => operator =

	bool operator==(const C3DBuffer<T>& rhs) const;
	bool operator!=(const C3DBuffer<T>& rhs) const;

	template< typename T2 = T>
	typename std::enable_if<std::is_same<T2, std::complex<float>>::value, C3DBuffer<T>&>::type operator/=(const C3DBuffer<float>& rhs);

	T& operator[](C3DBufferSize idx);
	T& at(C3DBufferSize idx);
	const T& operator[](C3DBufferSize idx) const;
	const T& at(C3DBufferSize idx) const;

	//replaceBigger => Clamp
	template< typename T2 = T>
	typename std::enable_if<std::is_arithmetic<T2>::value, void>::type   Clamp(const T lower, const T higher);

	//replaceEqual => passt?
	void Replace(const T from, const T to);
	void Replace(const std::function<bool(T)>& replace, const T to);

	template< typename T2 = T>
	typename std::enable_if<std::is_arithmetic<T2>::value, T>::type   FindMax() const;

	int getFullSize() const
	{
		return m_size.x * m_size.y * m_size.z;
	}

	C3DBufferSize GetSize() const
	{
		return m_size;
	}


	bool IsAllocated() const;
	std::vector<T> AsArray() const;

	void saveAsImageSeries(const std::wstring_view folderName) const;


	void loadFromImageSeries(const std::wstring_view folderName);


};


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

#include <vector>
#include <tuple>
#include <boost/rational.hpp>
#include <complex>
#include <functional>
#include <IIO_Defines.h>

#include "StlImageAlignedAllocator.h"
#include "Point.h"
#include <Win32HeapRAII.h>



#define STLIMAGE_HEAP_MINSIZE (102ull * 1024ull * 1024ull)

struct StlImageSize
{
	long long x; long long y;
	bool operator==(const StlImageSize& rhs) const { return x == rhs.x && y == rhs.y; }
	bool operator!=(const StlImageSize& rhs) const { return !(*this == rhs); }
	StlImageSize()
	{
		x = 0;
		y = 0;
	}
	StlImageSize(long long x_, long long y_)
	{
		x = x_;
		y = y_;
	}

	StlImageSize& operator+=(const StlImageSize& rhs) {
		x += rhs.x;		//same behavior like in CSize
		y += rhs.y;
		return *this;
	};
};
typedef StlImageSize StlImagePoint;
struct StlImageRect
{
	StlImageRect() = default;
	StlImageRect(long long x_start, long long y_start, long long x_end, long long y_end)
	{
		x = x_start;
		y = y_start;
		sx = x_end - x_start;
		sy = y_end - y_start;
	}
	StlImageRect(StlImagePoint topleft, StlImageSize size)
	{
		x = topleft.x;
		y = topleft.y;
		sx = size.x;
		sy = size.y;
	}
	long long x; long long y; long long sx; long long sy;
	StlImageSize Size() const
	{
		return { sx,sy };
	}
	bool IsRectEmpty() const
	{
		return sx == 0 || sy == 0;
	}
	void MoveToXY(long long x_, long long y_)
	{
		x = x_;
		y = y_;
	}
	bool Intersection(const StlImageRect& rect1, const StlImageRect& rect2)
	{
		auto intersection_x1 = std::max(rect1.x, rect2.x);
		auto intersection_y1 = std::max(rect1.y, rect2.y);

		auto intersection_x2 = std::min(rect1.x + rect1.sx, rect2.x + rect2.sx);
		auto intersection_y2 = std::min(rect1.y + rect1.sy, rect2.y + rect2.sy);

		auto intersection_sx = intersection_x2 - intersection_x1;
		auto intersection_sy = intersection_y2 - intersection_y1;

		if ((intersection_sx > 0) && (intersection_sy > 0))
		{
			x = intersection_x1;
			y = intersection_y1;
			sx = intersection_sx;
			sy = intersection_sy;
			return true;
		}
		else
		{
			x = 0;
			y = 0;
			sx = 0;
			sy = 0;
			return false;
		}
	}
};




template<typename T> class StlImage
{
	template<typename> friend class StlImage;
	template<typename> friend class C3DBuffer;
	template<typename, size_t> friend struct StlImageAlignedAllocator;

protected:
	std::vector<T, StlImageAlignedAllocator<T, 512>> m_data;
	StlImageSize m_size;

	static Win32HeapRAII<STLIMAGE_HEAP_MINSIZE> OurHackyMemoryStore[24];

public:
	StlImage();
	StlImage(StlImage<T>&& from) noexcept;
	explicit StlImage(const StlImage<T>& from);

	template<typename T2> explicit StlImage(const StlImage<T2>& from);
	~StlImage();

	void Alloc(StlImageSize size);
	void Free();

	void Clear(T value = T());
	void Clear(size_t _x, size_t _y, size_t sx, size_t sy, T value = T());

	void AddAtOffset(StlImagePoint p, StlImage<T>& img);
	void SubstractAtOffset(StlImagePoint p, StlImage<T>& img);
	void CopyAtOffset(StlImagePoint p, StlImage<T>& img);
	void AddAtOffset(StlImagePoint p, StlImage<T>& img, StlImageRect r);
	void AddAtOffset(StlImagePoint p, T value, StlImageRect r);
	void SubstractAtOffset(StlImagePoint p, StlImage<T>& img, StlImageRect r);
	void SubstractAtOffset(StlImagePoint p, T value, StlImageRect r);
	void CopyAtOffset(StlImagePoint p, StlImage<T>& img, StlImageRect r);

	StlImage<T> operator+(const StlImage<T>& rhs);
	StlImage<T> operator-(const StlImage<T>& rhs);
	StlImage<T> operator*(const StlImage<T>& rhs);
	StlImage<T> operator/(const StlImage<T>& rhs);

	StlImage<T>& operator+=(const StlImage<T>& rhs);
	StlImage<T>& operator-=(const StlImage<T>& rhs);
	StlImage<T>& operator*=(const StlImage<T>& rhs);
	StlImage<T>& operator/=(const StlImage<T>& rhs);

	StlImage<T> operator+(const T rhs) const;
	StlImage<T> operator-(const T rhs) const;
	StlImage<T> operator*(const T rhs) const;
	StlImage<T> operator/(const T rhs) const;

	StlImage<T>& operator+=(const T rhs);
	StlImage<T>& operator-=(const T rhs);
	StlImage<T>& operator*=(const T rhs);
	StlImage<T>& operator/=(const T rhs);

	StlImage<T>& operator=(const StlImage<T>& rhs);



	bool operator==(const StlImage<T>& rhs) const;
	bool operator!=(const StlImage<T>& rhs) const;

	template< typename T2 = T>
	typename std::enable_if<std::is_same<T2, std::complex<float>>::value, StlImage<T>&>::type operator/=(const StlImage<float>& rhs);

	T& operator[](StlImageSize idx);
	T& at(StlImageSize idx);
	const T& operator[](StlImageSize idx) const;
	const T& at(StlImageSize idx) const;


	void Abs();
	void Abs(const StlImage<T>& src);
	template< typename T2 = T>
	typename std::enable_if<std::is_same<T2, float>::value, void>::type Abs(const StlImage<std::complex<T>>& src);

	StlImageSize GetSize() const;

	template< typename T2 = T>
	typename std::enable_if<std::is_same<T2, std::complex<float>>::value, void>::type Conjugate();
	template< typename T2 = T>
	typename std::enable_if<std::is_arithmetic<T2>::value, double>::type Mean() const;
	template< typename T2 = T>
	typename std::enable_if<std::is_arithmetic<T2>::value, double>::type Mean(const StlImage<T>& mask) const;

	size_t NumberOfNonzeroPixels() const;

	static StlImageSize GetPossibleFFTSize(StlImageSize im1Size, StlImageSize im2Size);

	template< typename T2 = T>
	typename std::enable_if<std::is_same<T2, float>::value, void>::type Correlation_phase(const StlImage<T>& img1, const StlImage<T>& img2, int nLowFrequencyWidth, int nHighFrequencyWidth, StlImageSize FFTSize = StlImageSize());

	void LowFrequencySuppression(int nBandWidth);
	void HighFrequencySuppression(int nBandWidth);

	void MaskedCopy(const StlImage<T>& from);

	template< typename T2 = T>
	typename std::enable_if<std::is_same<T2, std::complex<float>>::value, void>::type Transform_Forward(const StlImage<float>& source);
	template< typename T2 = T>
	typename std::enable_if<std::is_same<T2, float>::value, void>::type Transform_Backward(const StlImage<std::complex<float>>& source);

	bool Export(std::wstring filename) const;
	bool LoadStrict(std::wstring filename);
	bool LoadConvert(std::wstring filename);

	static bool ImportFromMultiPageTiffFile(const std::wstring& filepath, std::vector<StlImage<T>>& imageList);

	template<typename Tsrc = T>
	typename std::enable_if<!std::is_same<Tsrc, std::complex<float>>::value, bool>::type ExportAs8bitUnsigned(std::wstring filename) const;

	template<typename Tsrc>
	typename std::enable_if<!std::is_same<Tsrc, std::complex<float>>::value, void>::type CopyConvert(const StlImage<Tsrc>& src)
	{
		if (!IsAllocated() || GetSize() != src.GetSize())	//verhindert Clear in Alloc-Funktion
			Alloc(src.GetSize());
		std::transform(src.m_data.cbegin(), src.m_data.cend(), m_data.begin(), [](Tsrc elem) -> T {return static_cast<T>(elem);});
	}
	void Copy2d(StlImageRect ourRoi, const StlImage<T>& src, StlImagePoint srcPoint = { 0,0 });
	void Copy2d(StlImageRect ourRoi, const StlImage<T>& src, StlImageRect srcROI);

	void Child2dIndep(StlImage<T>& src, StlImagePoint start, StlImageSize size);
	void Child2dIndep(StlImage<T>& src, StlImageRect rect);

	template< typename T2 = T>
	typename std::enable_if<std::is_arithmetic<T2>::value, void>::type Smooth(const StlImage<T>& src);

	std::vector<T> AsArray() const;

	bool IsAllocated() const;
	void Put(const T* src);
	void AddBorders(const StlImage<T>& source, long nLeft, long nTop, long nRight, long nBottom, T Color = T());
	bool CropBorders(const StlImage<T>& src, T transparentColor = T());
	void PrepareCropBorders(T transparentColor, StlImagePoint& Offset, StlImageSize& NewSize) const;

	template< typename T2 = T>
	typename std::enable_if<std::is_arithmetic<T2>::value, double>::type MeanSquare() const;	//only simple Implementation
	template< typename T2 = T>
	typename std::enable_if<std::is_arithmetic<T2>::value, void>::type TranslateBilinear(StlImage<T>& src, const float x_shift, const float y_shift);

	template< typename T2 = T>
	typename std::enable_if<std::is_arithmetic<T2>::value, void>::type   Clamp(const T lower, const T higher);

	void Replace(const T from, const T to);
	void Replace(const std::function<bool(T)>& replace, const T to);

	void Resize(StlImage<T>& src, boost::rational<int> ratio);
	template< typename T2 = T>
	typename std::enable_if<std::is_arithmetic<T2>::value, void>::type CopyClamp(const StlImage<T>& src, const T lower, const T higher);

	template< typename T2 = T>
	typename std::enable_if<std::is_arithmetic<T2>::value, std::tuple<T, DPoint>>::type  FindMax() const;
	template< typename T2 = T>
	typename std::enable_if<std::is_arithmetic<T2>::value, std::tuple<T, DPoint>>::type  FindMin() const;
	template< typename T2 = T>
	typename std::enable_if<std::is_arithmetic<T2>::value, DPoint>::type FindMaxSubPixelPosition() const;
	template< typename T2 = T>
	typename std::enable_if<std::is_arithmetic<T2>::value, DPoint>::type FindMaxSubPixelPosition(const DPoint maximumPixelPosition) const;
	template< typename T2 = T>
	typename std::enable_if<std::is_arithmetic<T2>::value, double>::type FindMaxSubPixelPositionCoordinate(const DPoint maximumPixelPosition, bool bX) const;
	void ClearPoint(DPoint point, int nPaddingAroundPoint, T fColor = T());
	template< typename T2 = T>
	typename std::enable_if<std::is_arithmetic<T2>::value, double>::type StandardDeviation() const;

	template< typename T2 = T>
	typename std::enable_if<std::is_arithmetic<T2>::value, void>::type DistanceChamfer();
private:
	template< typename T2 = T>
	typename inline std::enable_if<std::is_arithmetic<T2>::value, T>::type calcPixelDistChamfer(int x, int y);
};

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
#include "StlImage.h"

#pragma comment(lib, "mkl_core.lib")
#pragma comment(lib, "mkl_intel_ilp64.lib")
#pragma comment(lib, "mkl_sequential.lib")

#pragma comment(lib,"ImageIO.lib")



template<typename T>
StlImage<T>::StlImage()
{
	m_size = { 0,0 };
}

template<typename T>
StlImage<T>::StlImage(StlImage<T>&& from) noexcept
{
	m_data = std::move(from.m_data);
	m_size = from.m_size;
	from.m_size = { 0,0 };
	from.m_data.clear();
}

template<typename T>
StlImage<T>::StlImage(const StlImage<T>& from)
{
	m_data = from.m_data;
	m_size = from.m_size;
}

template<typename T>
template<typename T2>
typename std::enable_if<std::is_same<T2, std::complex<float>>::value, void>::type StlImage<T>::Conjugate()
{
	for (auto& elem : m_data) elem = std::conj(elem);
}

template<typename T>
StlImage<T>::~StlImage() = default;

template<typename T>
void StlImage<T>::Alloc(StlImageSize size)
{
	if (size.x < 1 || size.y < 1) throw std::bad_alloc();
	if (m_size == size)
	{
		Clear();
		return;
	}

	Free();
	m_data.resize(size.x * size.y);
	m_size = size;
}

template<typename T>
void StlImage<T>::Free()
{
	m_data.clear();
	m_size = { 0,0 };
}

template<typename T>
void StlImage<T>::Clear(T value)
{
	std::fill(m_data.begin(), m_data.end(), value);
}

template<typename T>
void StlImage<T>::Clear(size_t _x, size_t _y, size_t sx, size_t sy, T value)
{
	for (int y = _y;y < std::min(_y + sy, (size_t)m_size.y);y++)
	{
		for (int x = _x;x < std::min(_x + sx, (size_t)m_size.x);x++)
		{
			m_data[x + y * m_size.x] = value;
		}
	}
}

template<typename T>
void StlImage<T>::AddAtOffset(StlImagePoint p, StlImage<T>& img)
{
	for (int y = 0;y < std::min(img.GetSize().y, GetSize().y - p.y);y++)
	{
		for (int x = 0;x < std::min(img.GetSize().x, GetSize().x - p.x);x++)
		{
			(*this)[{p.x + x, p.y + y}] += img[{x, y}];
		}
	}
}

template<typename T>
void StlImage<T>::SubstractAtOffset(StlImagePoint p, StlImage<T>& img)
{
	for (int y = 0;y < std::min(img.GetSize().y, GetSize().y - p.y);y++)
	{
		for (int x = 0;x < std::min(img.GetSize().x, GetSize().x - p.x);x++)
		{
			(*this)[{p.x + x, p.y + y}] -= img[{x, y}];
		}
	}
}

template<typename T>
void StlImage<T>::CopyAtOffset(StlImagePoint p, StlImage<T>& img)
{
	for (int y = 0; y < std::min(img.GetSize().y, GetSize().y - p.y); y++)
	{
		for (int x = 0; x < std::min(img.GetSize().x, GetSize().x - p.x); x++)
		{
			(*this)[{p.x + x, p.y + y}] = img[{x, y}];
		}
	}
}

template<typename T>
void StlImage<T>::AddAtOffset(StlImagePoint p, StlImage<T>& img, StlImageRect r)
{
	for (int y = 0; y < std::min(r.sy, GetSize().y - p.y); y++)
	{
		for (int x = 0; x < std::min(r.sx, GetSize().x - p.x); x++)
		{
			(*this)[{p.x + x, p.y + y}] += img[{x + r.x, y + r.y}];
		}
	}
}

template<typename T>
void StlImage<T>::AddAtOffset(StlImagePoint p, T value, StlImageRect r)
{
	for (int y = 0; y < std::min(r.sy, GetSize().y - p.y); y++)
	{
		for (int x = 0; x < std::min(r.sx, GetSize().x - p.x); x++)
		{
			(*this)[{p.x + x, p.y + y}] += value;
		}
	}
}

template<typename T>
void StlImage<T>::SubstractAtOffset(StlImagePoint p, StlImage<T>& img, StlImageRect r)
{
	for (int y = 0; y < std::min(r.sy, GetSize().y - p.y); y++)
	{
		for (int x = 0; x < std::min(r.sx, GetSize().x - p.x); x++)
		{
			(*this)[{p.x + x, p.y + y}] -= img[{x + r.x, y + r.y}];
		}
	}
}

template<typename T>
void StlImage<T>::SubstractAtOffset(StlImagePoint p, T value, StlImageRect r)
{
	for (int y = 0; y < std::min(r.sy, GetSize().y - p.y); y++)
	{
		for (int x = 0; x < std::min(r.sx, GetSize().x - p.x); x++)
		{
			(*this)[{p.x + x, p.y + y}] -= value;
		}
	}
}

template<typename T>
void StlImage<T>::CopyAtOffset(StlImagePoint p, StlImage<T>& img, StlImageRect r)
{
	for (int y = 0; y < std::min(r.sy, GetSize().y - p.y); y++)
	{
		for (int x = 0; x < std::min(r.sx, GetSize().x - p.x); x++)
		{
			(*this)[{p.x + x, p.y + y}] = img[{x + r.x, y + r.y}];
		}
	}
}

template<typename T>
StlImage<T> StlImage<T>::operator+(const StlImage<T>& rhs)
{
	assert(m_size == rhs.m_size);
	StlImage<T> ret;
	ret.Alloc(m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), ret.m_data.begin(), std::plus<T>());
	return ret;
}

template<typename T>
StlImage<T> StlImage<T>::operator-(const StlImage<T>& rhs)
{
	assert(m_size == rhs.m_size);
	StlImage<T> ret;
	ret.Alloc(m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), ret.m_data.begin(), std::minus<T>());
	return ret;
}
template<typename T>
StlImage<T> StlImage<T>::operator*(const StlImage<T>& rhs)
{
	assert(m_size == rhs.m_size);
	StlImage<T> ret;
	ret.Alloc(m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), ret.m_data.begin(), std::multiplies<T>());
	return ret;
}
template<typename T>
StlImage<T> StlImage<T>::operator/(const StlImage<T>& rhs)
{
	assert(m_size == rhs.m_size);
	StlImage<T> ret;
	ret.Alloc(m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), ret.m_data.begin(), std::divides<T>());
	return ret;
}
template<>
StlImage<float> StlImage<float>::operator/(const StlImage<float>& rhs)
{
	assert(m_size == rhs.m_size);
	StlImage<float> ret;
	ret.Alloc(m_size);
	vsDiv(m_size.x * m_size.y, m_data.data(), rhs.m_data.data(), ret.m_data.data());
	return ret;
}
template<typename T>
StlImage<T>& StlImage<T>::operator+=(const StlImage<T>& rhs)
{
	assert(m_size == rhs.m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), m_data.begin(), std::plus<T>());
	return *this;
}
template<typename T>
StlImage<T>& StlImage<T>::operator-=(const StlImage<T>& rhs)
{
	assert(m_size == rhs.m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), m_data.begin(), std::minus<T>());
	return *this;
}
template<typename T>
StlImage<T>& StlImage<T>::operator*=(const StlImage<T>& rhs)
{
	assert(m_size == rhs.m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), m_data.begin(), std::multiplies<T>());
	return *this;
}
template<typename T>
StlImage<T>& StlImage<T>::operator/=(const StlImage<T>& rhs)
{
	assert(m_size == rhs.m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), m_data.begin(), std::divides<T>());
	return *this;
}
template<>
StlImage<float>& StlImage<float>::operator/=(const StlImage<float>& rhs)
{
	assert(m_size == rhs.m_size);
	vsDiv(m_size.x * m_size.y, m_data.data(), rhs.m_data.data(), m_data.data());
	return *this;
}
template<typename T>
StlImage<T> StlImage<T>::operator+(const T rhs) const
{
	StlImage<T> ret;
	ret.Alloc(m_size);
	std::transform(m_data.cbegin(), m_data.cend(), ret.m_data.begin(), [rhs](T elem)-> T {return elem + rhs;});
	return ret;
}
template<typename T>
StlImage<T> StlImage<T>::operator-(const T rhs) const
{
	StlImage<T> ret;
	ret.Alloc(m_size);
	std::transform(m_data.cbegin(), m_data.cend(), ret.m_data.begin(), [rhs](T elem)-> T {return elem - rhs;});
	return ret;
}
template<typename T>
StlImage<T> StlImage<T>::operator*(const T rhs) const
{
	StlImage<T> ret;
	ret.Alloc(m_size);
	std::transform(m_data.cbegin(), m_data.cend(), ret.m_data.begin(), [rhs](T elem)-> T {return elem * rhs;});
	return ret;
}
template<typename T>
StlImage<T> StlImage<T>::operator/(const T rhs) const
{
	StlImage<T> ret;
	ret.Alloc(m_size);
	std::transform(m_data.cbegin(), m_data.cend(), ret.m_data.begin(), [rhs](T elem)-> T {return elem / rhs;});
	return ret;
}
template<typename T>
StlImage<T>& StlImage<T>::operator+=(const T rhs)
{
	for (auto& elem : m_data)
		elem += rhs;
	return *this;
}
template<typename T>
StlImage<T>& StlImage<T>::operator-=(const T rhs)
{
	for (auto& elem : m_data)
		elem -= rhs;
	return *this;
}
template<typename T>
StlImage<T>& StlImage<T>::operator*=(const T rhs)
{
	for (auto& elem : m_data)
		elem *= rhs;
	return *this;
}
template<typename T>
StlImage<T>& StlImage<T>::operator/=(const T rhs)
{
	for (auto& elem : m_data)
		elem /= rhs;
	return *this;
}

template<typename T>
StlImage<T>& StlImage<T>::operator=(const StlImage<T>& rhs)
{
	if (this != &rhs)
	{ // self-assignment check expected
		m_data = rhs.m_data;
		m_size = rhs.m_size;
	}
	return *this;
}

template<typename T>
bool StlImage<T>::operator==(const StlImage<T>& rhs) const
{
	return m_size == rhs.m_size && std::equal(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), rhs.m_data.cend());
}

template<typename T>
bool StlImage<T>::operator!=(const StlImage<T>& rhs) const
{
	return !(*this == rhs);
}

template<typename T>
template<typename T2>
typename std::enable_if<std::is_same<T2, std::complex<float>>::value, StlImage<T>&>::type StlImage<T>::operator/=(const StlImage<float>& rhs)
{
	assert(m_size == rhs.m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), m_data.begin(), [](T lhs_elem, float rhs_elem) -> T {return T(lhs_elem.real() / rhs_elem, lhs_elem.imag() / rhs_elem);});
	return *this;
}
template<typename T>
T& StlImage<T>::operator[](StlImageSize idx)
{
	return m_data[idx.x + idx.y * m_size.x];
}
template<typename T>
T& StlImage<T>::at(StlImageSize idx)
{
	return m_data.at(idx.x + idx.y * m_size.x);
}
template<typename T>
const T& StlImage<T>::operator[](StlImageSize idx) const
{
	return m_data[idx.x + idx.y * m_size.x];
}
template<typename T>
const T& StlImage<T>::at(StlImageSize idx) const
{
	return m_data.at(idx.x + idx.y * m_size.x);
}
template<typename T>
void StlImage<T>::Abs()
{
	for (auto& elem : m_data)
		elem = std::abs(elem);
}


template<typename T>
void StlImage<T>::Abs(const StlImage<T>& src)
{
	Alloc(src.m_size);
	std::transform(src.m_data.cbegin(), src.m_data.cend(), m_data.begin(), [](T elem)-> T {return abs(elem);});
}
template<typename T>
template< typename T2>
typename std::enable_if<std::is_same<T2, float>::value, void>::type StlImage<T>::Abs(const StlImage<std::complex<T>>& src)
{
	Alloc(src.GetSize());
	MKL_INT size = static_cast<MKL_INT>(m_data.size());
	vcAbs(size, (MKL_Complex8*)src.m_data.data(), m_data.data());
	//std::transform(src.m_data.cbegin(), src.m_data.cend(), m_data.begin(), [](std::complex<T> elem)-> T {return abs(elem);});
}

template<typename T>
template< typename T2>
typename std::enable_if<std::is_arithmetic<T2>::value, void>::type StlImage<T>::DistanceChamfer()
{
	std::replace_if(m_data.begin(), m_data.end(), [](T elem) -> bool {return (elem != 0.0); }, std::numeric_limits<T>::max());
	bool someChange = true;
//#ifdef HIT_STLIMAGE_USE_CUSTOM_HEAP_MANAGEMENT
#ifdef _WIN32
	std::vector<T, StlImageAlignedAllocator<T, 512>> writeBackCopy;
#else
	std::vector<T> writeBackCopy;
#endif
	writeBackCopy.resize(m_data.size());
	int counter = 0;
	while (someChange)
	{
		someChange = false;
		T fillVal = std::numeric_limits<T>::max();
		//std::fill(writeBackCopy.begin(), writeBackCopy.end(), fillVal);

		for (int y = 0; y < m_size.y; y++)
		{
			for (int x = 0; x < m_size.x; x++)
			{
				if (m_data[x + y * m_size.x] != 0)
				{
					T calcResult = calcPixelDistChamfer(x, y);
					if (m_data[x + y * m_size.x] > calcResult) {
						writeBackCopy[x + y * m_size.x] = calcResult;
						someChange = true;
					}
					else
					{
						writeBackCopy[x + y * m_size.x] = m_data[x + y * m_size.x];
					}
				}
				else {
					m_data[x + y * m_size.x] = 0;
				}
			}
		}
		m_data.swap(writeBackCopy);
		counter++;

		/*std::wstringstream wss;
		wss << L"E:\\tmp\\tst\\i_";
		wss << counter;
		wss << L".tiff";
		Export( wss.str() );*/
	}
	*this /= static_cast<T>(3);
}

template<typename T>
template< typename T2>
typename inline std::enable_if<std::is_arithmetic<T2>::value, T>::type StlImage<T>::calcPixelDistChamfer(int x, int y)
{
	/*
		1 2 3		4 3 4
		4   5		3   3
		6 7 8		4 3 4

	*/

	T minVal;
	if (m_data[x + y * m_size.x] == 0)
	{
		minVal = std::numeric_limits<T>::max();
	}
	else
	{
		minVal = m_data[x + y * m_size.x];
	}
	T fillVal = std::numeric_limits<T>::max();
	if (y > 0) {
		//1
		if (x > 0) {
			if (m_data[(x - 1) + (y - 1) * m_size.x] != fillVal) {
				T val = m_data[(x - 1) + (y - 1) * m_size.x] + 4; // static_cast<T>(4.0);
				minVal = std::min(minVal, val);;//std::min(minVal, val);
			}
		}
		//2
		if (m_data[x + (y - 1) * m_size.x] != fillVal) {
			T val = m_data[x + (y - 1) * m_size.x] + 3; // static_cast<T>(3.0);
			minVal = std::min(minVal, val);
		}
		//3
		if (x + 1 < m_size.x) {
			if (m_data[(x + 1) + (y - 1) * m_size.x] != fillVal) {
				T val = m_data[(x + 1) + (y - 1) * m_size.x] + 4; // static_cast<T>(4.0);
				minVal = std::min(minVal, val);
			}
		}
	}

	//4
	if (x > 0) {
		if (m_data[(x - 1) + y * m_size.x] != fillVal) {
			T val = m_data[(x - 1) + y * m_size.x] + 3; // static_cast<T>(3.0);
			minVal = std::min(minVal, val);
		}
	}
	//5
	if (x + 1 < m_size.x) {
		if (m_data[(x + 1) + y * m_size.x] != fillVal) {
			T val = m_data[(x + 1) + y * m_size.x] + 3; // static_cast<T>(3.0);
			minVal = std::min(minVal, val);
		}
	}

	if (y + 1 < m_size.y) {
		//6
		if (x > 0) {
			if (m_data[(x - 1) + (y + 1) * m_size.x] != fillVal) {
				T val = m_data[(x - 1) + (y + 1) * m_size.x] + 4; // static_cast<T>(4.0);
				minVal = std::min(minVal, val);
			}
		}
		//7
		if (m_data[x + (y + 1) * m_size.x] != fillVal) {
			T val = m_data[x + (y + 1) * m_size.x] + 3; // static_cast<T>(3.0);
			minVal = std::min(minVal, val);
		}
		//8
		if (x + 1 < m_size.x) {
			if (m_data[(x + 1) + (y + 1) * m_size.x] != fillVal) {
				T val = m_data[(x + 1) + (y + 1) * m_size.x] + 4; // static_cast<T>(4.0);
				minVal = std::min(minVal, val);
			}
		}
	}

	return minVal;
}

template<typename T>
StlImageSize StlImage<T>::GetSize() const
{
	return m_size;
}

template<typename T>
template< typename T2>
typename std::enable_if<std::is_arithmetic<T2>::value, double>::type StlImage<T>::Mean() const
{
	return std::transform_reduce(m_data.cbegin(), m_data.cend(), 0.0, std::plus<double>(), [](T elem) -> double {return elem;}) / (double)m_data.size();
}

template<typename T>
template< typename T2>
typename std::enable_if<std::is_arithmetic<T2>::value, double>::type StlImage<T>::Mean(const StlImage<T>& mask) const
{
	return std::accumulate(m_data.cbegin(), m_data.cend(), T()) / (double)mask.NumberOfNonzeroPixels();
}


template<typename T>
size_t StlImage<T>::NumberOfNonzeroPixels() const
{
	return m_data.size() - std::count(m_data.cbegin(), m_data.cend(), T());
	//return std::transform_reduce(m_data.cbegin(), m_data.cend(), 0.0f, std::plus<>(), [=](auto const elem) { return elem == T() ? 0 : 1;});
}

template<typename T>
StlImageSize StlImage<T>::GetPossibleFFTSize(StlImageSize im1Size, StlImageSize im2Size)
{
	StlImageSize maxSize{ std::max(im1Size.x, im2Size.x), std::max(im1Size.y, im2Size.y) };
	return { CMathTools::NextPowOf2(maxSize.x), CMathTools::NextPowOf2(maxSize.y) };
}

template<typename T>
template<typename T2>
typename std::enable_if<std::is_same<T2, float>::value, void>::type StlImage<T>::Correlation_phase(const StlImage<T>& img1, const StlImage<T>& img2, int nLowFrequencyWidth, int nHighFrequencyWidth, StlImageSize FFTSize)
{
	if (FFTSize == StlImageSize())
	{
		FFTSize = GetPossibleFFTSize(img1.GetSize(), img2.GetSize());
	}

	double emp_wert = ((static_cast<double>(FFTSize.x * FFTSize.y) * static_cast<double>(FFTSize.x * FFTSize.y)) * 0.0025);

	StlImage<T> imgPadded1;
	StlImage<T> imgPadded2;

	auto fMean1 = img1.Mean(img1);
	imgPadded1.Alloc(FFTSize);
	imgPadded1.Clear(fMean1);
	imgPadded1.MaskedCopy(img1);
	imgPadded1 -= fMean1;

	auto fMean2 = img2.Mean(img2);
	imgPadded2.Alloc(FFTSize);
	imgPadded2.Clear(fMean2);
	imgPadded2.MaskedCopy(img2);
	imgPadded2 -= fMean2;



	StlImage<std::complex<T>> imgFourier;
	StlImage<std::complex<T>> imgFourierTemp;

	imgFourierTemp.Transform_Forward(imgPadded1);

	imgFourier.Transform_Forward(imgPadded2);
	imgFourier.Conjugate();


	imgFourier *= imgFourierTemp;
	imgFourier[{0, 0}] = 0;

	imgFourier.LowFrequencySuppression(nLowFrequencyWidth);
	imgFourier.HighFrequencySuppression(nHighFrequencyWidth);

	imgPadded1.Abs(imgFourier);
	imgPadded1 += emp_wert;

	imgFourier /= imgPadded1;

	imgFourierTemp.Free();

	this->Alloc(FFTSize);
	this->Transform_Backward(imgFourier);
}
#define DFTI_CHECK(call) status=call; assert(status==DFTI_NO_ERROR)
template<typename T>
template<typename T2>
typename std::enable_if<std::is_same<T2, std::complex<float>>::value, void>::type StlImage<T>::Transform_Forward(const StlImage<float>& source)
{

	Alloc({ source.m_size.x / 2 + 1, source.m_size.y });

	DFTI_DESCRIPTOR_HANDLE handle_forward = NULL;
	int64_t status;

	int64_t dim_sizes[2] = { source.m_size.y, source.m_size.x };

	int64_t rs[3] = { 0, source.m_size.x, 1 };
	int64_t cs[3] = { 0, source.m_size.x / 2 + 1, 1 };

	DFTI_CHECK(DftiCreateDescriptor(&handle_forward, DFTI_SINGLE, DFTI_REAL, 2, dim_sizes));
	DFTI_CHECK(DftiSetValue(handle_forward, DFTI_PLACEMENT, DFTI_NOT_INPLACE));
	DFTI_CHECK(DftiSetValue(handle_forward, DFTI_CONJUGATE_EVEN_STORAGE, DFTI_COMPLEX_COMPLEX));
	DFTI_CHECK(DftiSetValue(handle_forward, DFTI_PACKED_FORMAT, DFTI_CCE_FORMAT));
	DFTI_CHECK(DftiSetValue(handle_forward, DFTI_INPUT_STRIDES, rs));
	DFTI_CHECK(DftiSetValue(handle_forward, DFTI_OUTPUT_STRIDES, cs));
	DFTI_CHECK(DftiSetValue(handle_forward, DFTI_THREAD_LIMIT, 1));
	DFTI_CHECK(DftiCommitDescriptor(handle_forward));

	DFTI_CHECK(DftiComputeForward(handle_forward, const_cast<float*>(source.m_data.data()), m_data.data()));
	DFTI_CHECK(DftiFreeDescriptor(&handle_forward));
}

template<typename T>
template<typename T2>
typename std::enable_if<std::is_same<T2, float>::value, void>::type StlImage<T>::Transform_Backward(const StlImage<std::complex<float>>& source)
{

	Alloc({ (source.m_size.x - 1) * 2, source.m_size.y });
	int64_t status;
	int64_t dim_sizes[2] = { m_size.y, m_size.x };

	int64_t rs[3] = { 0, m_size.x, 1 };
	int64_t cs[3] = { 0, m_size.x / 2 + 1, 1 };
	DFTI_DESCRIPTOR_HANDLE handle_backward = NULL;

	DFTI_CHECK(DftiCreateDescriptor(&handle_backward, DFTI_SINGLE, DFTI_REAL, 2, dim_sizes));
	DFTI_CHECK(DftiSetValue(handle_backward, DFTI_PLACEMENT, DFTI_NOT_INPLACE));
	DFTI_CHECK(DftiSetValue(handle_backward, DFTI_CONJUGATE_EVEN_STORAGE, DFTI_COMPLEX_COMPLEX));
	DFTI_CHECK(DftiSetValue(handle_backward, DFTI_PACKED_FORMAT, DFTI_CCE_FORMAT));
	DFTI_CHECK(DftiSetValue(handle_backward, DFTI_INPUT_STRIDES, cs));
	DFTI_CHECK(DftiSetValue(handle_backward, DFTI_OUTPUT_STRIDES, rs));
	DFTI_CHECK(DftiSetValue(handle_backward, DFTI_THREAD_LIMIT, 1));
	DFTI_CHECK(DftiCommitDescriptor(handle_backward));

	DFTI_CHECK(DftiComputeBackward(handle_backward, const_cast<std::complex<T>*>(source.m_data.data()), m_data.data()));
	DFTI_CHECK(DftiFreeDescriptor(&handle_backward));
}

/**	\brief Unterdrückt die niederen Bildfrequenzen im Spektralbild \a imFreq.
*
*	Die niederen Bildfrequenzen befinden sich im (periodisch zu
*	interpretierenden) Spektralbild \a imFreq rund um den Ursprung. Zu deren
*	Unterdrückung werden daher die Ecken des Spektralbilds auf den Wert 0
*	gesetzt.
*
*	Der Bandbreite der Frequenzunterdrückung, also die Anzahl der Pixel, die
*	auf den Wert 0 gesetzt werden, wird durch \a nBandWidth definiert.
*
*	\param[in,out] imFreq Das komplexe Spektralbild.
*	\param[in] nBandWidth Die Bandbreite der Frequenzunterdrückung.
*
*	\see HighFrequencySuppression()
*/
template<typename T>
void StlImage<T>::LowFrequencySuppression(int nBandWidth)
{
	assert(nBandWidth >= 0);

	if (nBandWidth <= 0)
	{
		return;
	}

	if (m_size.x > 2 * nBandWidth)
	{
		if (m_size.y > 2 * nBandWidth)
		{
			Clear(0, 0, nBandWidth, nBandWidth);
			Clear(m_size.x - nBandWidth, 0, nBandWidth, nBandWidth);
			Clear(0, m_size.y - nBandWidth, nBandWidth, nBandWidth);
			Clear(m_size.x - nBandWidth, m_size.y - nBandWidth, nBandWidth, nBandWidth);
		}
		else
		{
			Clear(0, 0, nBandWidth, m_size.y);
			Clear(m_size.x - nBandWidth, 0, nBandWidth, m_size.y);
		}
	}
	else
	{
		if (m_size.y > 2 * nBandWidth)
		{
			Clear(0, 0, m_size.x, nBandWidth);
			Clear(0, m_size.y - nBandWidth, m_size.x, nBandWidth);
		}
		else
		{
			Clear();
		}
	}
}

/**	\brief Unterdrückt die hohen Bildfrequenzen im Bereich der
*	Nyquist-Frequenz im Spektralbild \a imFreq.
*
*	Die Bildfrequenzen im Bereich der Nyquist-Frequenz befinden sich im
*	(periodisch zu interpretierenden) Spektralbild \a imFreq auf den
*	Bildzeilen bzw. -spalten auf halber Bildhöhe bzw. -breite. Zu deren
*	Unterdrückung werden daher ein waagrechter und ein senkrechter Streifen
*	des Spektralbilds auf den Wert 0 gesetzt.
*
*	Der Bandbreite der Frequenzunterdrückung, also die Anzahl der Pixel, die
*	auf den Wert 0 gesetzt werden, wird durch \a nBandWidth definiert.
*
*	\param[in,out] imFreq Das komplexe Spektralbild.
*	\param[in] nBandWidth Die Bandbreite der Frequenzunterdrückung.
*
*	\see LowFrequencySuppression
*/
template<typename T>
void StlImage<T>::HighFrequencySuppression(int nBandWidth)
{
	assert(nBandWidth >= 0);

	if (nBandWidth <= 0)
	{
		return;
	}

	StlImageSize center{ m_size.x / 2, m_size.y / 2 };
	int nWidth = 2 * nBandWidth + 1;

	if ((m_size.x > nWidth) && (m_size.y > nWidth))
	{
		Clear(0, center.y - nBandWidth, m_size.x, nWidth);
		Clear(center.x - nBandWidth, 0, nWidth, m_size.y);
	}
	else
	{
		Clear();
	}
}

template<typename T>
void StlImage<T>::MaskedCopy(const StlImage<T>& from)
{
	size_t min_x = std::min(m_size.x, from.m_size.x);
	size_t min_y = std::min(m_size.y, from.m_size.y);

	for (int y = 0;y < min_y;y++)
	{
		for (int x = 0;x < min_x;x++)
		{
			if (auto val = from.m_data[x + y * from.m_size.x]; val != T())
			{
				m_data[x + y * m_size.x] = val;
			}
		}
	}
}

template<typename T>
bool StlImage<T>::Export(std::wstring filename) const
{
	size_t h = GetSize().y, w = GetSize().x;
	int bit_depth = sizeof(T) * 8;
	int channels = 1;
	IIO_DATA_TYPE iio_type;

	if (std::is_floating_point<T>::value)
	{
		iio_type = IIO_DATA_TYPE::GRAY_FLOAT;
	}
	else if (std::is_unsigned<T>::value)
	{
		iio_type = IIO_DATA_TYPE::GRAY_UNSIGNED;
	}
	else if (std::is_signed<T>::value)
	{
		iio_type = IIO_DATA_TYPE::GRAY_SIGNED;
	}
	else if (std::is_same<T, std::complex<float>>::value || std::is_same<T, std::complex<double>>::value)
	{
		iio_type = IIO_DATA_TYPE::COMPLEX_FLOAT;
	}
	else
	{
		assert(false);
		return false;
	}
	bool b = CImageExporter::ExportImage(filename.c_str(), bit_depth, h, w, m_data.data(), iio_type, IIO_FILE_TYPE::TIF, IIO_TIF_COMPRESSION::NONE);
	return b;
}


template<typename T>
template< typename Tsrc>
typename std::enable_if<!std::is_same<Tsrc, std::complex<float>>::value, bool>::type StlImage<T>::ExportAs8bitUnsigned(std::wstring filename) const
{
	StlImage<uint8_t> tmp;
	//const StlImage<T> obj = *this;
	tmp.CopyConvert(*this);
	return tmp.Export(filename);
}

template<typename T>
bool StlImage<T>::LoadStrict(std::wstring filename)
{
	IIO_DATA_TYPE format;
	int bits;
	void* data = nullptr;
	size_t h, w;

	if (!CFileUtilities::FileExists(std::wstring(filename)))
		throw CImageIOException(filename, CImageIOException::eFileNotFound);

	try
	{
		if (!CImageImporter::ImportImage(filename, format, bits, h, w, data))
		{
			// regardless of the image pixel type, this raw data buffer is
			// always created with type unsigned char inside
			// CImageImporter::ImportImage().
			delete[] static_cast<unsigned char *>(data);
			return false;
		}
	}
	catch (CImageIOException ex)
	{
		// regardless of the image pixel type, this raw data buffer is always
		// created with type unsigned char inside CImageImporter::ImportImage().
		delete[] static_cast<unsigned char *>(data);
		return false;
	}
	if (format == IIO_DATA_TYPE::GRAY_UNSIGNED)
	{
		if (std::is_same<T, unsigned char>::value)
		{
			Alloc(StlImageSize(w, h));
			memcpy(m_data.data(), data, sizeof(T) * w * h);
		}
		else
		{
			// regardless of the image pixel type, this raw data buffer is
			// always created with type unsigned char inside
			// CImageImporter::ImportImage().
			delete[] static_cast<unsigned char *>(data);
			return false;
		}
	}
	else if (format == IIO_DATA_TYPE::GRAY_FLOAT)
	{
		if (std::is_same<T, float>::value)
		{
			Alloc(StlImageSize(w, h));
			memcpy(m_data.data(), data, sizeof(T) * w * h);
		}
		else
		{
			// regardless of the image pixel type, this raw data buffer is
			// always created with type unsigned char inside
			// CImageImporter::ImportImage().
			delete[] static_cast<unsigned char *>(data);
			return false;
		}
	}
	else if (format == IIO_DATA_TYPE::COMPLEX_FLOAT)
	{
		if (std::is_same<T, std::complex<float>>::value)
		{
			Alloc(StlImageSize(w, h));
			memcpy(m_data.data(), data, sizeof(T) * w * h);
		}
		else
		{
			// regardless of the image pixel type, this raw data buffer is
			// always created with type unsigned char inside
			// CImageImporter::ImportImage().
			delete[] static_cast<unsigned char *>(data);
			return false;
		}
	}
	else
	{
		// regardless of the image pixel type, this raw data buffer is always
		// created with type unsigned char inside CImageImporter::ImportImage().
		delete[] static_cast<unsigned char *>(data);
		return false;
	}

	// regardless of the image pixel type, this raw data buffer is always
	// created with type unsigned char inside CImageImporter::ImportImage().
	delete[] static_cast<unsigned char *>(data);
	return true;
}

template<typename T>
bool StlImage<T>::LoadConvert(std::wstring filename)
{
	IIO_DATA_TYPE format;
	int bits;
	void* data = nullptr;
	size_t h, w;

	if (!CFileUtilities::FileExists(std::wstring(filename)))
		throw CImageIOException(filename, CImageIOException::eFileNotFound);

	try
	{
		CImageImporter::ImportImage(filename, format, bits, h, w, data);
	}
	catch (CImageIOException ex)
	{
		// regardless of the image pixel type, this raw data buffer is always
		// created with type unsigned char inside CImageImporter::ImportImage().
		delete[] static_cast<unsigned char *>(data);
		throw;
	}
	if (format == IIO_DATA_TYPE::GRAY_UNSIGNED && bits == 8)
	{
		if (std::is_same<T, unsigned char>::value)
		{
			Alloc(StlImageSize(w, h));
			memcpy(m_data.data(), data, bits / 8 * w * h);
		}
		else
		{
			StlImage<unsigned char> temp;;
			temp.Alloc(StlImageSize(w, h));
			memcpy(temp.m_data.data(), data, bits / 8 * w * h);
			CopyConvert(temp);
		}
	}
	else if (format == IIO_DATA_TYPE::GRAY_FLOAT && bits == 32)
	{
		if (std::is_same<T, float>::value)
		{
			Alloc(StlImageSize(w, h));
			memcpy(m_data.data(), data, bits / 8 * w * h);
		}
		else
		{
			StlImage<float> temp;;
			temp.Alloc(StlImageSize(w, h));
			memcpy(temp.m_data.data(), data, bits / 8 * w * h);
			CopyConvert(temp);
		}
	}
	else if (format == IIO_DATA_TYPE::COMPLEX_FLOAT && bits == 64)
	{
		if (std::is_same<T, std::complex<float>>::value)
		{
			Alloc(StlImageSize(w, h));
			memcpy(m_data.data(), data, bits / 8 * w * h);
		}
		else
		{
			// regardless of the image pixel type, this raw data buffer is
			// always created with type unsigned char inside
			// CImageImporter::ImportImage().
			delete[] static_cast<unsigned char *>(data);
			return false;
		}
	}
	else
	{
		// regardless of the image pixel type, this raw data buffer is always
		// created with type unsigned char inside CImageImporter::ImportImage().
		delete[] static_cast<unsigned char *>(data);
		return false;
	}
	// regardless of the image pixel type, this raw data buffer is always
	// created with type unsigned char inside CImageImporter::ImportImage().
	delete[] static_cast<unsigned char *>(data);
	return true;
}

template<typename T>
bool StlImage<T>::ImportFromMultiPageTiffFile(const std::wstring& filepath, std::vector<StlImage<T>>& imageList)
{
	IIO_DATA_TYPE format;
	int bits;
	void** data = nullptr;
	size_t h, w, n;

	if (!CFileUtilities::FileExists(filepath))
		throw CImageIOException(filepath, CImageIOException::eFileNotFound);

	try
	{
		if (!CImageImporter::ImportImageSeries(filepath, format, bits, h, w, n, data))
		{
			assert(data == nullptr);
			return false;
		}
	}
	catch (CImageIOException ex)
	{
		if (data != nullptr)
		{
			for (size_t i = 0; i < n; i++)
			{
				if (data[i] != nullptr)
				{
					// regardless of the image pixel type, this raw data buffer
					// is always created with type unsigned char inside
					// CImageImporter::ImportImage().
					delete[] static_cast<unsigned char *>(data[i]);
				}
			}
			delete[] data;
		}
		throw;
	}
	assert(data != nullptr);

	imageList.resize(n);
	for (size_t i = 0; i < n; i++)
	{
		assert(data[i] != nullptr);

		StlImage<T>& img = imageList[i];

		if (format == IIO_DATA_TYPE::GRAY_UNSIGNED && bits == 8)
		{
			if (std::is_same<T, unsigned char>::value)
			{
				img.Alloc(StlImageSize(w, h));
				memcpy(img.m_data.data(), data[i], bits / 8 * w * h);
			}
			else
			{
				StlImage<unsigned char> temp;
				temp.Alloc(StlImageSize(w, h));
				memcpy(temp.m_data.data(), data[i], bits / 8 * w * h);
				img.CopyConvert(temp);
			}
		}
		else if (format == IIO_DATA_TYPE::GRAY_FLOAT && bits == 32)
		{
			if (std::is_same<T, float>::value)
			{
				img.Alloc(StlImageSize(w, h));
				memcpy(img.m_data.data(), data[i], bits / 8 * w * h);
			}
			else
			{
				StlImage<float> temp;;
				temp.Alloc(StlImageSize(w, h));
				memcpy(temp.m_data.data(), data[i], bits / 8 * w * h);
				img.CopyConvert(temp);
			}
		}
		else
		{
			for (size_t j = i; j < n; j++)
			{
				// regardless of the image pixel type, this raw data buffer is
				// always created with type unsigned char inside
				// CImageImporter::ImportImage().
				delete[] static_cast<unsigned char *>(data[j]);
			}
			delete[] data;
			return false;
		}

		// regardless of the image pixel type, this raw data buffer is always
		// created with type unsigned char inside CImageImporter::ImportImage().
		delete[] static_cast<unsigned char *>(data[i]);
		data[i] = nullptr;
	}
	delete[] data;
	return true;
}

template<typename T>
void StlImage<T>::Child2dIndep(StlImage<T>& src, StlImageRect rect)
{
	Child2dIndep(src, { rect.x, rect.y }, { rect.sx, rect.sy });
}

template<typename T>
std::vector<T> StlImage<T>::AsArray() const
{
	return std::vector<T>(m_data.cbegin(), m_data.cend());
}

template<typename T>
template<typename T2>
typename std::enable_if<std::is_arithmetic<T2>::value, double>::type StlImage<T>::StandardDeviation() const
{
	auto m = Mean();
	auto sumsq = std::transform_reduce(m_data.cbegin(), m_data.cend(), 0.0, std::plus<double>(), [m](T val) -> double {return (val - m) * (val - m);});
	return sqrt(sumsq / (m_data.size() - 1.0));
}

template<typename T>
template<typename T2>
typename std::enable_if<std::is_arithmetic<T2>::value, void>::type StlImage<T>::Smooth(const StlImage<T>& src)
{
	std::vector<float> kernel{ 1 / 4.0,2 / 4.0,1 / 4.0 };

	//Do not! use auto here, this needs to be a signed type and auto would be unsigned
	int ksize = kernel.size() >> 1;

	StlImage<T> tmp;
	if (this != &src) Alloc(src.GetSize());
	tmp.Alloc(src.GetSize());

	for (auto y = 0;y < m_size.y;y++)
	{
		for (auto x = 0;x < m_size.x;x++)
		{
			T sum = T();
			for (auto k = -ksize; k <= ksize;k++)
			{
				sum += src[{ static_cast<long long>(x), static_cast<long long>(std::clamp(y + k, 0, static_cast<int>(m_size.y) - 1)) }] * kernel[k + ksize];
			}
			tmp[{ static_cast<long long>(x), static_cast<long long>(y) }] = sum;
		}
	}
	for (auto y = 0;y < m_size.y;y++)
	{
		for (auto x = 0;x < m_size.x;x++)
		{
			T sum = T();
			for (auto k = -ksize; k <= ksize;k++)
			{
				sum += tmp[{ static_cast<long long>(std::clamp(x + k, 0, static_cast<int>(m_size.x) - 1)), static_cast<long long>(y) }] * kernel[k + ksize];
			}
			(*this)[{ static_cast<long long>(x), static_cast<long long>(y) }] = sum;
		}
	}
}

template<typename T>
void StlImage<T>::Copy2d(StlImageRect ourRoi, const StlImage<T>& src, StlImagePoint srcPoint)
{
	if (ourRoi.sy + ourRoi.y > m_size.y || ourRoi.sx + ourRoi.x > m_size.x)
	{
		throw std::out_of_range("ROI out of Bounds");
	}
	for (auto y = ourRoi.y; y < ourRoi.sy + ourRoi.y; y++)
	{
		for (auto x = ourRoi.x; x < ourRoi.sx + ourRoi.x; x++)
		{
			at({ x, y }) = src.at({ x - ourRoi.x + srcPoint.x, y - ourRoi.y + srcPoint.y });
		}
	}
}
template<typename T>
void StlImage<T>::Copy2d(StlImageRect ourRoi, const StlImage<T>& src, StlImageRect srcROI)
{
	if (ourRoi.sy + ourRoi.y > m_size.y || ourRoi.sx + ourRoi.x > m_size.x || ourRoi.x < 0 || ourRoi.y < 0)
	{
		throw std::out_of_range("ROI out of Bounds");
	}
	auto maxy = std::min(ourRoi.sy, srcROI.sy);
	auto maxx = std::min(ourRoi.sx, srcROI.sx);
	for (auto y = ourRoi.y;y < maxy + ourRoi.y;y++)
	{
		for (auto x = ourRoi.x;x < maxx + ourRoi.x;x++)
		{
			at({ x, y }) = src.at({ x - ourRoi.x + srcROI.x, y - ourRoi.y + srcROI.y });
		}
	}
}

template<typename T>
void StlImage<T>::Child2dIndep(StlImage<T>& src, StlImagePoint start, StlImageSize size)
{
	Alloc(size);
	for (auto y = 0u; y < size.y;y++)
	{
		for (auto x = 0u; x < size.x;x++)
		{
			(*this)[{x, y}] = src[{start.x + x, start.y + y}];
		}
	}
}

template<typename T>
template< typename T2>
typename std::enable_if<std::is_arithmetic<T2>::value, std::tuple<T, DPoint>>::type StlImage<T>::FindMax() const
{
	auto max = std::max_element(m_data.cbegin(), m_data.cend());
	size_t pos = std::distance(m_data.cbegin(), max);

	return { *max, DPoint(pos % m_size.x, (pos - pos % m_size.x) / (m_size.x)) };
}
template<typename T>
template<typename T2>
typename std::enable_if<std::is_arithmetic<T2>::value, std::tuple<T, DPoint>>::type StlImage<T>::FindMin() const
{
	auto min = std::min_element(m_data.cbegin(), m_data.cend());
	size_t pos = std::distance(m_data.cbegin(), min);

	return { *min, DPoint(pos % m_size.x, (pos - pos % m_size.x) / (m_size.x)) };
}
template<typename T>
template< typename T2>
typename std::enable_if<std::is_arithmetic<T2>::value, DPoint>::type StlImage<T>::FindMaxSubPixelPosition() const
{
	auto [max, maxPos] = FindMax();
	return FindMaxSubPixelPosition(maxPos);
}
template<typename T>
template< typename T2>
typename std::enable_if<std::is_arithmetic<T2>::value, DPoint>::type  StlImage<T>::FindMaxSubPixelPosition(const DPoint maximumPixelPosition) const
{

	double fSubPixX = FindMaxSubPixelPositionCoordinate(maximumPixelPosition, true);
	double fSubPixY = FindMaxSubPixelPositionCoordinate(maximumPixelPosition, false);


	DPoint subPixelPosition(
		maximumPixelPosition.m_x + fSubPixX,
		maximumPixelPosition.m_y + fSubPixY);

	return subPixelPosition;

}
/*
bX Says whether its the X coordinate (true) or the y-coordinate (false). This is not a nice way of doing this,
but it has always been like this and frankly I don't know enough abotu this function to change it.

Actualy before it was an int which could be 1 or 0.
*/
template<typename T>
template< typename T2>
typename std::enable_if<std::is_arithmetic<T2>::value, double>::type StlImage<T>::FindMaxSubPixelPositionCoordinate(const DPoint maximumPixelPosition, bool bX) const
{
	long m = 5; // Grauwertverlauf über 11 Pixel (+-5)
	float fMinDecrease = 0.8F; // minimaler Grauwertabfall;

	int nS = bX ? m_size.x : m_size.y;

	int nXRounded = CMathTools::RoundValue(maximumPixelPosition.m_x);
	int nYRounded = CMathTools::RoundValue(maximumPixelPosition.m_y);

	float maxP, meanNP;
	int dk = 1;

	if (bX) // x
	{
	}
	else // y
	{
	}

	T* pPixel = new T[2 * m + 1];

	//Daten auslesen
	for (int j = 0; j <= 2 * m; j++)
	{
		if (bX) // x
		{
			pPixel[j] = (*this)[{static_cast<long long>((nXRounded + j - m + nS) % nS), static_cast<long long>(maximumPixelPosition.m_y)}];

		}
		else  // y
		{

			pPixel[j] = (*this)[{static_cast<long long>(maximumPixelPosition.m_x), static_cast<long long>((nYRounded + j - m + nS) % nS)}];
		}
	}

	//Subpixelgenaue Bestimmung der Lage des Maximums

	maxP = pPixel[m];
	meanNP = (pPixel[m - dk] + pPixel[m + dk]) / 2.0F;

	if (meanNP > fMinDecrease * maxP)
	{
		//Distanz für Interpolation verdoppeln
		dk = dk * 2;
		meanNP = (pPixel[m - dk] + pPixel[m + dk]) / 2.0F;
		if (meanNP > fMinDecrease * maxP)
		{
			//Distanz für Interpolation verdoppeln
			dk = dk * 2;
			/*meanNP = (pPixel[m-dk] + pPixel[m+dk])/2.0;*/
		}
	}

	//Koeefifizienten berechnen
	float b, c, returnValSub_u = 0;
	b = pPixel[m + dk] - pPixel[m - dk];
	c = pPixel[m] * 2 - pPixel[m - dk] - pPixel[m + dk];

	if (c > 0)
	{
		returnValSub_u = b * c * dk / (2 * c * c + 0.0001F * maxP);
	}

	delete[] pPixel;

	return returnValSub_u;
}
/*
Clears a point in the image with a Padding around it. If the Padding overlaps outisde of the image, it is wrapped around.
*/
template<typename T>
void StlImage<T>::ClearPoint(DPoint point, int nPaddingAroundPoint, T fColor)
{
	int nXPos = CMathTools::RoundValue(point.m_x);
	int nYPos = CMathTools::RoundValue(point.m_y);

	for (int y = nYPos - nPaddingAroundPoint + m_size.y;y < nYPos + nPaddingAroundPoint + m_size.y;y++)
	{
		for (int x = nXPos - nPaddingAroundPoint + m_size.x;x < nXPos + nPaddingAroundPoint + m_size.x;x++)
		{
			(*this)[{x% m_size.x, y% m_size.y}] = fColor;
		}
	}
}

template<typename T>
template<typename T2>
typename std::enable_if<std::is_arithmetic<T2>::value, double>::type StlImage<T>::MeanSquare() const
{
	double fSquareSum = std::transform_reduce(m_data.cbegin(), m_data.cend(), 0.0, std::plus<double>(), [](T val) -> double {return (static_cast<double>(val) * val); });
	long long  nNumber = m_size.x * m_size.y;
	return (fSquareSum / nNumber);
}

template<typename T>
template<typename T2>
typename std::enable_if<std::is_arithmetic<T2>::value, void>::type StlImage<T>::TranslateBilinear(StlImage<T>& src, const float x_shift, const float y_shift)
{
	StlImage<T> dest;
	dest.Alloc(src.GetSize());
	dest.Clear(static_cast<T>(0.0f));

	long long lower_x_bound = static_cast<long long>(std::max(ceil(x_shift), 0.0f));
	long long lower_y_bound = static_cast<long long>(std::max(ceil(y_shift), 0.0f));
	long long upper_x_bound = std::min(dest.GetSize().x, static_cast<long long>(dest.GetSize().x + floor(x_shift)));
	long long upper_y_bound = std::min(dest.GetSize().y, static_cast<long long>(dest.GetSize().y + floor(y_shift)));

	long long x_shift_int = ceil(x_shift);
	long long y_shift_int = ceil(y_shift);

	float abs_shift_tmp_x = abs(1.0 - x_shift);
	float abs_shift_tmp_y = abs(1.0 - y_shift);

	float sx = abs_shift_tmp_x - floor(abs_shift_tmp_x);
	float sy = abs_shift_tmp_y - floor(abs_shift_tmp_y);

	float f_tl = (1.0f - sx) * (1.0f - sy);
	float f_tr = (sx) * (1.0f - sy);
	float f_bl = (1.0f - sx) * (sy);
	float f_br = (sx) * (sy);

	//https://gist.github.com/lebedov/12e9dbdf86d34b966cd197ea46dd958e
	for (long long y = lower_y_bound; y < upper_y_bound; y++) {

		//float src_y = y - y_shift;
		//long long src_y0 = static_cast<long long>(floor(src_y));
		//long long src_y1 = min(src_y0 + 1, src.GetSize().y - 1);

		long long src_y_int = y - y_shift_int;
		long long src_y0 = src_y_int;
		long long src_y1 = std::min(src_y_int + 1, src.GetSize().y - 1);

		//float sy = src_y - src_y0;

		for (long long x = lower_x_bound; x < upper_x_bound; x++) {

			//float src_x = x - x_shift;

			//long long src_x0 = static_cast<long long>(floor(src_x));
			//long long src_x1 = min(src_x0 + 1, src.GetSize().x - 1);

			long long src_x_int = x - x_shift_int;
			long long src_x0 = src_x_int;
			long long src_x1 = std::min(src_x_int + 1, src.GetSize().y - 1);

			//float sx = src_x - src_x0;

			auto val = f_tl * src.m_data[src_x0 + src_y0 * m_size.x];
			val += f_tr * src.m_data[src_x1 + src_y0 * m_size.x];
			val += f_bl * src.m_data[src_x0 + src_y1 * m_size.x];
			val += f_br * src.m_data[src_x1 + src_y1 * m_size.x];
			if constexpr (std::is_same_v<T, decltype(val)>)
			{
				dest.m_data[x + y * m_size.x] = val;
			}
			else
			{
				dest.m_data[x + y * m_size.x] = static_cast<T>(val);
			}

		}
	}
	*this = std::move(dest);
}

template<typename T>
bool StlImage<T>::IsAllocated() const
{
	return m_size.x > 0 && m_size.y > 0;
}
template<typename T>
void StlImage<T>::Put(const T* src)
{
	std::memcpy(m_data.data(), src, m_data.size() * sizeof(T));
}
/**	\brief Fügt dem Bild an allen Seiten Ränder in der Farbe Color hinzu.
*
*	\param[in] source Das Quellbild
*	\param[in] nLeft Breite des linken Randes.
*	\param[in] nTop Breite des oberen Randes.
*	\param[in] nRight Breite des rechten Randes.
*	\param[in] nBottom Breite des unteren Randes.
*	\param[in] Color Farbwert.
*/
template<typename T>
void StlImage<T>::AddBorders(const StlImage<T>& source, long nLeft, long nTop, long nRight, long nBottom, T Color)
{
	auto NewSize = GetSize();
	NewSize.x += nLeft + nRight;
	NewSize.y += nTop + nBottom;

	StlImage<T> trg;
	trg.Alloc(NewSize);
	// Bild mit der Bordercolor füllen
	trg.Clear(Color);

	// altes Bild an die entsprechende Stelle kopieren
	trg.Copy2d(StlImageRect(nLeft, nTop, source.GetSize().x, source.GetSize().y), source, StlImagePoint(0, 0));
	*this = std::move(trg);
}
/**	\brief Schneidet Rand ab.
*
*	Schneidet Bereich ohne Bildinformation ab soweit wie möglich ab.
*	Resultat ist ein kleineres Bild, welches keinen Rand der gewählten
*	Farbe enthält.
*	Ist die gewählte Farbe 0 (Schwarz), so wird ein schnelleres Verfahren
*	angewandt.
*
*	\param[in] sourceImage		Buffer of the Source Image.
*	\param[in] transparentColor Farbe des undefinierten Bereiches.
*/
template<typename T>
bool StlImage<T>::CropBorders(const StlImage<T>& src, T transparentColor)
{
	auto OldSize = src.GetSize();
	StlImagePoint Offset;
	StlImageSize NewSize;

	src.PrepareCropBorders(transparentColor, Offset, NewSize);


	assert((Offset.x >= 0) && (NewSize.x >= 0) && (Offset.x + NewSize.x <= OldSize.x));
	assert((Offset.y >= 0) && (NewSize.y >= 0) && (Offset.y + NewSize.y <= OldSize.y));


	if ((NewSize.x == 0) || (NewSize.y == 0))
	{
		return false;
	}
	else if ((NewSize.x < OldSize.x) || (NewSize.y < OldSize.y))
	{
		if (&src != this)
		{
			Alloc(NewSize);
			Copy2d(StlImageRect(0, 0, NewSize.x, NewSize.y), src, Offset);
		}
		else
		{
			StlImage<T> tempCopy;
			tempCopy.Alloc(NewSize);
			tempCopy.Copy2d(StlImageRect(0, 0, NewSize.x, NewSize.y), src, Offset);
			*this = std::move(tempCopy);
		}
	}
	else // EQUAL SIZE
	{
		if (&src != this)
		{
			*this = src;
		}
	}

	return true;
}

template<typename T>
void StlImage<T>::PrepareCropBorders(T transparentColor, StlImagePoint& Offset, StlImageSize& NewSize) const
{
	auto OldSize = GetSize();


	StlImagePoint topLeftCorner(0, 0);
	StlImagePoint bottomRightCorner(0, 0);

	//
	//	Y-Koordinate des obersten Punktes des Bildes
	//	
	bool abort = false;


	for (long long dy = 0; dy < OldSize.y; dy++)
	{
		for (long long dx = 0; dx < OldSize.x; dx++)
		{
			if ((*this)[{dx, dy}] != transparentColor)
			{
				topLeftCorner.y = dy;
				abort = true;
				break;
			}
		}
		if (abort)	break;
	}
	abort = false;

	//
	//	Y-Koordinate des untersten Punktes des Bildes
	//	

	for (long long dy = OldSize.y - 1; dy >= 0; dy--)
	{
		for (long long dx = OldSize.x - 1; dx >= 0; dx--)
		{
			if ((*this)[{dx, dy}] != transparentColor)
			{
				bottomRightCorner.y = dy + 1;
				abort = true;
				break;
			}
		}
		if (abort)	break;
	}
	abort = false;

	//
	//	X-Koordinate des linksten Punktes des Bildes
	//

	for (long long dx = 0; dx < OldSize.x; dx++)
	{
		for (long long dy = 0; dy < OldSize.y; dy++)
		{
			if ((*this)[{dx, dy}] != transparentColor)
			{
				topLeftCorner.x = dx;
				abort = true;
				break;
			}
		}
		if (abort)	break;
	}
	abort = false;

	//
	//	X-Koordinate des rechtesten Punktes des Bildes
	//	
	for (long long dx = OldSize.x - 1; dx >= 0; dx--)
	{
		for (long long dy = OldSize.y - 1; dy >= 0; dy--)
		{
			if ((*this)[{dx, dy}] != transparentColor)
			{
				bottomRightCorner.x = dx + 1;
				abort = true;
				break;
			}
		}
		if (abort)	break;
	}

	//
	// Anpassung auf Standard
	NewSize.x = bottomRightCorner.x - topLeftCorner.x;
	NewSize.y = bottomRightCorner.y - topLeftCorner.y;
	Offset = topLeftCorner;
}

template<typename T>
template< typename T2>
typename std::enable_if<std::is_arithmetic<T2>::value, void>::type StlImage<T>::Clamp(const T lower, const T higher)
{
	std::transform(m_data.begin(), m_data.end(), m_data.begin(), [&lower, &higher](T val) -> T { return std::clamp(val, lower, higher); });
}

template<typename T>
void StlImage<T>::Replace(const T from, const T to)
{
	std::transform(m_data.begin(), m_data.end(), m_data.begin(), [&from, &to](T val) -> T { return val == from ? to : val; });
}
template<typename T>
void StlImage<T>::Replace(const std::function<bool(T)>& replace, const T to)
{
	std::transform(m_data.begin(), m_data.end(), m_data.begin(), [&replace, &to](T val) -> T { return replace(val) ? to : val; });
}
template<typename T>
void StlImage<T>::Resize(StlImage<T>& src, boost::rational<int> ratio)
{
	if (ratio.numerator() != 1)
	{
		throw std::runtime_error("not done yet");
	}
	Alloc(StlImageSize(src.GetSize().x / ratio.denominator(), src.GetSize().y / ratio.denominator()));
	for (auto y = 0;y < m_size.y;y++)
	{
		for (auto x = 0;x < m_size.x;x++)
		{
			(*this)[{x, y}] = 0;
			for (auto y2 = 0;y2 < ratio.denominator();y2++)
			{
				for (auto x2 = 0;x2 < ratio.denominator();x2++)
				{
					(*this)[{x, y}] += src[{x* ratio.denominator() + x2, y* ratio.denominator() + y2}];
				}
			}
			(*this)[{x, y}] /= ratio.denominator() * ratio.denominator();
		}
	}
}

template<typename T>
template< typename T2>
typename std::enable_if<std::is_arithmetic<T2>::value, void>::type StlImage<T>::CopyClamp(const StlImage<T>& src, const T lower, const T higher)
{
	std::transform(src.m_data.begin(), src.m_data.end(), m_data.begin(), [&lower, &higher](T val) -> T { return std::clamp(val, lower, higher); });
}

//Instantiate the data types we want to support
template class StlImage<float>;
template class StlImage<int>;
template class StlImage<unsigned char>;
template class StlImage<std::complex<float>>;

template void StlImage<float>::Correlation_phase<float>(const StlImage<float>&, const StlImage<float>&, int, int, StlImageSize);
template bool StlImage<float>::ExportAs8bitUnsigned<float>(std::wstring filename) const;
template double StlImage<float>::Mean<float>() const;
template void StlImage<float>::Smooth<float>(const StlImage<float>&);
template double StlImage<float>::StandardDeviation<float>() const;
template std::tuple<float, DPoint> StlImage<float>::FindMax<float>() const;
template std::tuple<float, DPoint> StlImage<float>::FindMin<float>() const;
template DPoint StlImage<float>::FindMaxSubPixelPosition<float>() const;
template double StlImage<float>::MeanSquare<float>() const;
template void StlImage<float>::TranslateBilinear<float>(StlImage<float>& src, const float x_shift, const float y_shift);
template void StlImage<float>::Clamp<float>(float, float);
template void StlImage<float>::CopyClamp<float>(const StlImage<float>&, float, float);
template void StlImage<float>::DistanceChamfer<float>();
template float StlImage<float>::calcPixelDistChamfer<float>(int, int);

template std::tuple<int, DPoint> StlImage<int>::FindMax<int>() const;
template std::tuple<int, DPoint> StlImage<int>::FindMin<int>() const;

template double StlImage<unsigned char>::Mean<unsigned char>() const;
template void StlImage<unsigned char>::Smooth<unsigned char>(const StlImage<unsigned char>&);
template double StlImage<unsigned char>::StandardDeviation<unsigned char>() const;
template std::tuple<unsigned char, DPoint> StlImage<unsigned char>::FindMax<unsigned char>() const;
template std::tuple<unsigned char, DPoint> StlImage<unsigned char>::FindMin<unsigned char>() const;
template DPoint StlImage<unsigned char>::FindMaxSubPixelPosition<unsigned char>() const;
template double StlImage<unsigned char>::MeanSquare<unsigned char>() const;
template void StlImage<unsigned char>::TranslateBilinear<unsigned char>(StlImage<unsigned char>& src, const float x_shift, const float y_shift);
template void StlImage<unsigned char>::Clamp<unsigned char>(unsigned char, unsigned char);
template void StlImage<unsigned char>::CopyClamp<unsigned char>(const StlImage<unsigned char>&, unsigned char, unsigned char);

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
#include "3DBuffer.h"

#ifdef _WIN32
#pragma comment(lib, "mkl_core.lib")
#pragma comment(lib, "mkl_intel_ilp64.lib")
#pragma comment(lib, "mkl_sequential.lib")
#endif



template<typename T>
C3DBuffer<T>::C3DBuffer()
{
}

template<typename T>
C3DBuffer<T>::C3DBuffer(C3DBuffer<T>&& from) noexcept
{
	m_data = std::move(from.m_data);
	m_size = from.m_size;
	from.m_size = { 0,0,0 };
	from.m_data.clear();
}

template<typename T>
C3DBuffer<T>::C3DBuffer(const C3DBuffer<T>& from)
{
	m_data = from.m_data;
	m_size = from.m_size;
}

template<typename T>
C3DBuffer<T>::~C3DBuffer() = default;

template<typename T>
void C3DBuffer<T>::Alloc(C3DBufferSize size)
{
	if (size.x < 1 || size.y < 1 || size.z < 1) throw std::bad_alloc();
	if (m_size == size)
	{
		Clear();
		return;
	}

	Free();
	m_data.resize(size.x * size.y * size.z);
	m_size = size;
}

template<typename T>
void C3DBuffer<T>::Free()
{
	m_data.clear();
	m_size = { 0,0,0 };
}

template<typename T>
void C3DBuffer<T>::Clear(T value)
{
	std::fill(m_data.begin(), m_data.end(), value);
}

template<typename T>
void C3DBuffer<T>::Clear(size_t _x, size_t _y, size_t _z, size_t sx, size_t sy, size_t sz, T value)
{
	for (int z = _y; z < std::min(_z + sz, (size_t)m_size.z); z++)
	{
		for (int y = _y; y < std::min(_y + sy, (size_t)m_size.y); y++)
		{
			for (int x = _x; x < std::min(_x + sx, (size_t)m_size.x); x++)
			{
				m_data[x + y * m_size.x + z * m_size.y * m_size.x] = value;
			}
		}
	}
}

template<typename T>
void C3DBuffer<T>::CopyAtOffset(C3DBufferPoint p, const C3DBuffer<T>& buf)
{
	for (int z = 0; z < std::min(buf.GetSize().z, GetSize().z - p.z); z++)
	{
		for (int y = 0; y < std::min(buf.GetSize().y, GetSize().y - p.y); y++)
		{
			for (int x = 0; x < std::min(buf.GetSize().x, GetSize().x - p.x); x++)
			{
				(*this)[{p.x + x, p.y + y, p.z + z}] = buf[{x, y, z}];
			}
		}
	}
}
template<typename T>
void C3DBuffer<T>::CopyAtOffset(C3DBufferPoint p, const C3DBuffer<T>& buf, C3DBufferRect r)
{
	for (int z = 0; z < std::min(r.sz, GetSize().z - p.z); z++)
	{
		for (int y = 0; y < std::min(r.sy, GetSize().y - p.y); y++)
		{
			for (int x = 0; x < std::min(r.sx, GetSize().x - p.x); x++)
			{
				(*this)[{p.x + x, p.y + y, p.z + z}] = buf[{x + r.x, y + r.y, z + r.z}];
			}
		}
	}
}

template<typename T>
void C3DBuffer<T>::CopyAtOffset(C3DBufferPoint p, const StlImage<T>& img)
{
	//no for loop for z (z-thickness always == 1)
	for (int y = 0; y < std::min(img.GetSize().y, GetSize().y - p.y); y++)
	{
		for (int x = 0; x < std::min(img.GetSize().x, GetSize().x - p.x); x++)
		{
			(*this)[{p.x + x, p.y + y, p.z}] = img[{x, y}];
		}
	}
}

template<typename T>
void C3DBuffer<T>::AddAtOffset(C3DBufferPoint p, C3DBuffer<T>& buf)
{
	for (int z = 0; z < std::min(buf.GetSize().z, GetSize().z - p.z); z++)
	{
		for (int y = 0; y < std::min(buf.GetSize().y, GetSize().y - p.y); y++)
		{
			for (int x = 0; x < std::min(buf.GetSize().x, GetSize().x - p.x); x++)
			{
				(*this)[{p.x + x, p.y + y, p.z + z}] += buf[{x, y, z}];
			}
		}
	}
}

template<typename T>
void C3DBuffer<T>::AddLineAtOffset(C3DBufferPoint p, std::vector<T>& buf)
{
	for (int x = 0; x < std::min((long long)buf.size(), GetSize().x - p.x); x++)
	{
		(*this)[{p.x + x, p.y, p.z}] += buf[x];
	}
}

template<typename T>
void C3DBuffer<T>::AddAtOffset(C3DBufferPoint p, C3DBuffer<T>& buf, C3DBufferRect r)
{
	for (int z = 0; z < std::min(r.sz, GetSize().z - p.z); z++)
	{
		for (int y = 0; y < std::min(r.sy, GetSize().y - p.y); y++)
		{
			for (int x = 0; x < std::min(r.sx, GetSize().x - p.x); x++)
			{
				(*this)[{p.x + x, p.y + y, p.z + z}] += buf[{x + r.x, y + r.y, z + r.z}];
			}
		}
	}
}

template<typename T>
void C3DBuffer<T>::PutInStlImage(StlImage<T>& img)
{
	std::memcpy(img.m_data.data(), m_data.data(), m_data.size() * sizeof(T));
}



template<typename T>
C3DBuffer<T> C3DBuffer<T>::operator+(const C3DBuffer<T>& rhs)
{
	assert(m_size == rhs.m_size);
	C3DBuffer<T> ret;
	ret.Alloc(m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), ret.m_data.begin(), std::plus<T>());
	return ret;
}

template<typename T>
C3DBuffer<T> C3DBuffer<T>::operator-(const C3DBuffer<T>& rhs)
{
	assert(m_size == rhs.m_size);
	C3DBuffer<T> ret;
	ret.Alloc(m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), ret.m_data.begin(), std::minus<T>());
	return ret;
}
template<typename T>
C3DBuffer<T> C3DBuffer<T>::operator*(const C3DBuffer<T>& rhs)
{
	assert(m_size == rhs.m_size);
	C3DBuffer<T> ret;
	ret.Alloc(m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), ret.m_data.begin(), std::multiplies<T>());
	return ret;
}
template<typename T>
C3DBuffer<T> C3DBuffer<T>::operator/(const C3DBuffer<T>& rhs)
{
	assert(m_size == rhs.m_size);
	C3DBuffer<T> ret;
	ret.Alloc(m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), ret.m_data.begin(), std::divides<T>());
	return ret;
}
template<>
C3DBuffer<float> C3DBuffer<float>::operator/(const C3DBuffer<float>& rhs)
{
	assert(m_size == rhs.m_size);
	C3DBuffer<float> ret;
	ret.Alloc(m_size);
	vsDiv(m_size.x * m_size.y * m_size.z, m_data.data(), rhs.m_data.data(), ret.m_data.data());
	return ret;
}
template<typename T>
C3DBuffer<T>& C3DBuffer<T>::operator+=(const C3DBuffer<T>& rhs)
{
	assert(m_size == rhs.m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), m_data.begin(), std::plus<T>());
	return *this;
}
template<typename T>
C3DBuffer<T>& C3DBuffer<T>::operator-=(const C3DBuffer<T>& rhs)
{
	assert(m_size == rhs.m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), m_data.begin(), std::minus<T>());
	return *this;
}
template<typename T>
C3DBuffer<T>& C3DBuffer<T>::operator*=(const C3DBuffer<T>& rhs)
{
	assert(m_size == rhs.m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), m_data.begin(), std::multiplies<T>());
	return *this;
}
template<typename T>
C3DBuffer<T>& C3DBuffer<T>::operator/=(const C3DBuffer<T>& rhs)
{
	assert(m_size == rhs.m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), m_data.begin(), std::divides<T>());
	return *this;
}
template<>
C3DBuffer<float>& C3DBuffer<float>::operator/=(const C3DBuffer<float>& rhs)
{
	assert(m_size == rhs.m_size);
	vsDiv(m_size.x * m_size.y * m_size.z, m_data.data(), rhs.m_data.data(), m_data.data());
	return *this;
}
template<typename T>
C3DBuffer<T> C3DBuffer<T>::operator+(const T rhs) const
{
	C3DBuffer<T> ret;
	ret.Alloc(m_size);
	std::transform(m_data.cbegin(), m_data.cend(), ret.m_data.begin(), [rhs](T elem)-> T {return elem + rhs; });
	return ret;
}
template<typename T>
C3DBuffer<T> C3DBuffer<T>::operator-(const T rhs) const
{
	C3DBuffer<T> ret;
	ret.Alloc(m_size);
	std::transform(m_data.cbegin(), m_data.cend(), ret.m_data.begin(), [rhs](T elem)-> T {return elem - rhs; });
	return ret;
}
template<typename T>
C3DBuffer<T> C3DBuffer<T>::operator*(const T rhs) const
{
	C3DBuffer<T> ret;
	ret.Alloc(m_size);
	std::transform(m_data.cbegin(), m_data.cend(), ret.m_data.begin(), [rhs](T elem)-> T {return elem * rhs; });
	return ret;
}
template<typename T>
C3DBuffer<T> C3DBuffer<T>::operator/(const T rhs) const
{
	C3DBuffer<T> ret;
	ret.Alloc(m_size);
	std::transform(m_data.cbegin(), m_data.cend(), ret.m_data.begin(), [rhs](T elem)-> T {return elem / rhs; });
	return ret;
}
template<typename T>
C3DBuffer<T>& C3DBuffer<T>::operator+=(const T rhs)
{
	for (auto& elem : m_data)
		elem += rhs;
	return *this;
}
template<typename T>
C3DBuffer<T>& C3DBuffer<T>::operator-=(const T rhs)
{
	for (auto& elem : m_data)
		elem -= rhs;
	return *this;
}
template<typename T>
C3DBuffer<T>& C3DBuffer<T>::operator*=(const T rhs)
{
	for (auto& elem : m_data)
		elem *= rhs;
	return *this;
}
template<typename T>
C3DBuffer<T>& C3DBuffer<T>::operator/=(const T rhs)
{
	for (auto& elem : m_data)
		elem /= rhs;
	return *this;
}

template<typename T>
C3DBuffer<T>& C3DBuffer<T>::operator=(const C3DBuffer<T>& rhs)
{
	if (this != &rhs)
	{ // self-assignment check expected
		m_data = rhs.m_data;
		m_size = rhs.m_size;
	}
	return *this;
}

template<typename T>
bool C3DBuffer<T>::operator==(const C3DBuffer<T>& rhs) const
{
	return m_size == rhs.m_size && std::equal(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), rhs.m_data.cend());
}

template<typename T>
bool C3DBuffer<T>::operator!=(const C3DBuffer<T>& rhs) const
{
	return !(*this == rhs);
}

template<typename T>
template<typename T2>
typename std::enable_if<std::is_same<T2, std::complex<float>>::value, C3DBuffer<T>&>::type C3DBuffer<T>::operator/=(const C3DBuffer<float>& rhs)
{
	assert(m_size == rhs.m_size);
	std::transform(m_data.cbegin(), m_data.cend(), rhs.m_data.cbegin(), m_data.begin(), [](T lhs_elem, float rhs_elem) -> T {return T(lhs_elem.real() / rhs_elem, lhs_elem.imag() / rhs_elem); });
	return *this;
}
template<typename T>
T& C3DBuffer<T>::operator[](C3DBufferSize idx)
{
	return m_data[idx.x + idx.y * m_size.x + idx.z * m_size.y * m_size.x];
}
template<typename T>
T& C3DBuffer<T>::at(C3DBufferSize idx)
{
	return m_data.at(idx.x + idx.y * m_size.x + idx.z * m_size.y * m_size.x);
}
template<typename T>
const T& C3DBuffer<T>::operator[](C3DBufferSize idx) const
{
	return m_data[idx.x + idx.y * m_size.x + idx.z * m_size.y * m_size.x];
}
template<typename T>
const T& C3DBuffer<T>::at(C3DBufferSize idx) const
{
	return m_data.at(idx.x + idx.y * m_size.x + idx.z * m_size.y * m_size.x);
}

template<typename T>
template< typename T2>
typename std::enable_if<std::is_arithmetic<T2>::value, void>::type C3DBuffer<T>::Clamp(const T lower, const T higher)
{
	std::transform(m_data.begin(), m_data.end(), m_data.begin(), [&lower, &higher](T val) -> T { return std::clamp(val, lower, higher); });
}

template<typename T>
template<typename T2>
typename std::enable_if<std::is_arithmetic<T2>::value, T>::type C3DBuffer<T>::FindMax() const
{
	auto max = std::max_element(m_data.cbegin(), m_data.cend());

	return *max;
}

template<typename T>
void C3DBuffer<T>::Replace(const T from, const T to)
{
	std::transform(m_data.begin(), m_data.end(), m_data.begin(), [&from, &to](T val) -> T { return val == from ? to : val; });
}
template<typename T>
void C3DBuffer<T>::Replace(const std::function<bool(T)>& replace, const T to)
{
	std::transform(m_data.begin(), m_data.end(), m_data.begin(), [&replace, &to](T val) -> T { return replace(val) ? to : val; });
}

template<typename T>
bool C3DBuffer<T>::IsAllocated() const
{
	return this->getFullSize() > 0;
}

template<typename T>
std::vector<T> C3DBuffer<T>::AsArray() const
{
	return std::vector<T>(m_data.cbegin(), m_data.cend());
}

template<typename T>
void C3DBuffer<T>::saveAsImageSeries(const std::wstring_view folderName) const
{
	CFileUtilities::MakeDirectory(wstring(folderName));
	StlImage<T> tmpImage;

	tmpImage.Alloc({ m_size.x, m_size.y });

	for (int zCounter = 0; zCounter < m_size.z; zCounter++) {
		tmpImage.Put(m_data.data() + (m_size.x * m_size.y * zCounter));
		auto fileName = CFileUtilities::FullFile({ wstring(folderName), std::to_wstring(zCounter) + L".tif" });
		tmpImage.Export(fileName);
	}
	tmpImage.Free();
}

template<typename T>
void C3DBuffer<T>::loadFromImageSeries(const std::wstring_view folderName)
{
	auto files = CFileUtilities::GetFilesInDirectory(std::wstring(folderName), CRegex(LR"(.*\.tif)"));
	assert(files.size() > 0);

	int maxZ = -1;
	for (auto file : files) // Suche höchste Z-Koordinate
	{
		auto fileName = file.substr(file.find_last_of('\\') + 1);
		int imgNum = std::stoi(fileName.substr(0, fileName.size() - 4));
		assert(imgNum != maxZ);
		if (imgNum > maxZ)
			maxZ = imgNum;
	}
	maxZ += 1; //Volumen eins größer als höchste Ebene

	StlImage<T> tmpSizeImage;

	tmpSizeImage.LoadConvert(files.at(0));	//Suche höchste XY-Koordinate
	int maxX = tmpSizeImage.GetSize().x;
	int maxY = tmpSizeImage.GetSize().y;
	tmpSizeImage.Free();

	Alloc({ maxX, maxY, maxZ });

	std::for_each(std::execution::par_unseq, files.begin(), files.end(), [&](auto&& imgName)	//load one image per layer -> parallel loading possible
		{
			StlImage<T> openImage;

			openImage.LoadConvert(imgName);	//Lade Bild

			assert(maxX == openImage.GetSize().x);
			assert(maxY == openImage.GetSize().y);

			auto fileName = imgName.substr(imgName.find_last_of('\\') + 1);
			CopyAtOffset({ 0, 0, std::stoi(fileName.substr(0, fileName.size() - 4)) }, openImage);

			openImage.Free();
		});
}



//Instantiate the data types we want to support
template class C3DBuffer<float>;
template class C3DBuffer<int>;
template class C3DBuffer<unsigned char>;
template class C3DBuffer<std::complex<float>>;

//float
template void C3DBuffer<float>::Clamp<float>(float, float);
template float C3DBuffer<float>::FindMax<float>() const;

//unsigned char
template void C3DBuffer<unsigned char>::Clamp<unsigned char>(unsigned char, unsigned char);
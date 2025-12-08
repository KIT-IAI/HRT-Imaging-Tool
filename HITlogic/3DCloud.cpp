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
#include "3DCloud.h"



/*#if __has_include(<pcl/pcl_base.h>)
#include <pcl/common/projection_matrix.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/io/ply_io.h>
#define HAS_PCL
#else
#pragma warning("PCL not installed.")
#endif*/



class C3DCloud::Impl {
public:
#ifdef HAS_PCL
	pcl::PointCloud <pcl::PointXYZRGB> m_cloud;	//rgb und rgba in pcl genau gleich definiert, Konstruktor von rgb aber besser
#endif
	Impl()
	{
	}
};

C3DCloud::C3DCloud()
#ifdef HAS_PCL
	: pimpl(new C3DCloud::Impl())
#endif
{
#ifdef HAS_PCL
	pimpl->m_cloud.is_dense = true;			//true, da alle Punktkoordinaten endliche Zahlen sind
#endif
}



C3DCloud::~C3DCloud()
{

}

void C3DCloud::add(float x, float y, float z, float intensity)
{
#ifdef HAS_PCL
	pcl::PointXYZRGB point(intensity, intensity, intensity);	//r,g,b
	point.x = x;
	point.y = y;
	point.z = z;
	pimpl->m_cloud.points.push_back(point);
#else
	assert(false);
#endif
}

void C3DCloud::reserve(size_t size)
{
#ifdef HAS_PCL
	pimpl->m_cloud.reserve(size);
#else
	assert(false);
#endif
}

void C3DCloud::savePCD(const std::wstring_view file_name, EWriteMode mode) const
{
#ifdef HAS_PCL
	auto name = CStringUtilities::ConvertToStdString(file_name);
	switch (mode)
	{
	case EWriteMode::eASCII:
		pcl::io::savePCDFileASCII(name, pimpl->m_cloud);
		break;
	case EWriteMode::eBinary:
		pcl::io::savePCDFileBinary(name, pimpl->m_cloud);
		break;
	case EWriteMode::eBinaryCompressed:
		pcl::io::savePCDFileBinaryCompressed(name, pimpl->m_cloud);
		break;
	default:
		assert(false);		//unbekannter Typ
		break;
	}
#else
	assert(false);
#endif
}

void C3DCloud::savePLY(const std::wstring_view file_name, EWriteMode mode) const
{
#ifdef HAS_PCL
	auto name = CStringUtilities::ConvertToStdString(file_name);
	switch (mode)
	{
	case EWriteMode::eASCII:
		pcl::io::savePLYFileASCII(name, pimpl->m_cloud);
		break;
	case EWriteMode::eBinary:
	case EWriteMode::eBinaryCompressed:
		pcl::io::savePLYFileBinary(name, pimpl->m_cloud);
		break;
	default:
		assert(false);		//unbekannter Typ
		break;
	}
#else
	assert(false);
#endif
}

void C3DCloud::Clone(const C3DCloud& ref)
{
#ifdef HAS_PCL
	pimpl->m_cloud.clear();		//zurücksetzen interne Cloud
	pimpl->m_cloud.points = ref.pimpl->m_cloud.points;	//alle wichtigen Inhalte kopieren
	pimpl->m_cloud.width = ref.pimpl->m_cloud.width;
	pimpl->m_cloud.height = ref.pimpl->m_cloud.height;
	pimpl->m_cloud.is_dense = ref.pimpl->m_cloud.is_dense;
	pimpl->m_cloud.sensor_origin_ = ref.pimpl->m_cloud.sensor_origin_;
	pimpl->m_cloud.sensor_orientation_ = ref.pimpl->m_cloud.sensor_orientation_;
#else
	assert(false);
#endif
}

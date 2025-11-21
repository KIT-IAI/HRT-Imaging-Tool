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
#include "DateTimeUtilities.h"

#include <boost\date_time.hpp>



std::wstring CDateTimeUtilities::CDateTime::ToString() const
{
	auto formatted = boost::wformat(L"%04d-%02d-%02d %02d:%02d:%02d") % year % month % day % hour % minute % second;
	return formatted.str();
}

CDateTimeUtilities::CDateTime CDateTimeUtilities::Now()
{
	auto time = boost::posix_time::second_clock::local_time();
	CDateTime date_time;
	date_time.hour = (int)time.time_of_day().hours();
	date_time.minute = (int)time.time_of_day().minutes();
	date_time.second = (int)time.time_of_day().seconds();
	date_time.day = time.date().day();
	date_time.month = time.date().month();
	date_time.year = time.date().year();

	return date_time;
}

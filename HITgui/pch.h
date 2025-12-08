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

#define _CRT_SECURE_NO_WARNINGS 1

#ifdef _WIN32
#include <SDKDDKVer.h>
#include <afxwin.h>
#endif

// standard libraries
#include <filesystem>
#include <string>
#include <thread>
using std::string;
using std::wstring;

// Boost libraries
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

using boost::posix_time::ptime;
using boost::posix_time::time_duration;
using boost::posix_time::microsec_clock;
using boost::posix_time::time_from_string;
using boost::posix_time::time_from_string;
using boost::posix_time::not_a_date_time;
using boost::filesystem::path;
using boost::lexical_cast;
using boost::property_tree::ptree;
using boost::property_tree::wptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;
using boost::property_tree::read_xml;
using boost::property_tree::write_xml;

// wxWidgets libraries
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif
#include <wx/clipbrd.h>
#include <wx/filepicker.h>
#include <wx/spinctrl.h>
#include <wx/stdpaths.h>
#include <wx/xrc/xmlres.h>

// Utilities project
#include "FileUtilities.h"
#if defined(__AVX__) || defined(__AVX2__) || defined (__AVX512F__)
#include "InstructionSet.h" 
#endif
#include "Log.h"
#include "SettingsStore.h"
#include "Utilities.h"

// HITlogic project
#include "SNPDatasetLoader.h"
#include "SNPDatasetParameterExporter.h"
#include "SNPDatasetParameterLoader.h"

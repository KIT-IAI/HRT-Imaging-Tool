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



#include "StdAfx.h"

#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "psapi.lib")

#include "Log.h"
#include "StringUtilities.h"
//Be carefull this supresses a lot of noise and the Unreferenced Parameter warning for this file.
#pragma warning(disable: 4503 4714 4510 4610 4100)

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/event_log_backend.hpp>
#include <boost/log/sinks/debug_output_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/support/date_time.hpp>

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace src = boost::log::sources;
namespace trv = boost::log::trivial;
typedef sinks::synchronous_sink< sinks::debug_output_backend > sink_t;
typedef sinks::synchronous_sink< sinks::simple_event_log_backend > sink2_t;
typedef sinks::synchronous_sink< sinks::text_file_backend > sink3_t;

boost::shared_ptr<sink3_t> CLogFileLogger;

trv::severity_level TranslateSeverity(CLog::ELogLevel level)
{
	switch (level)
	{
	case CLog::eEmergency:
		return trv::fatal;
		break;
	case CLog::eAlert:
		return trv::fatal;
		break;
	case CLog::eCritical:
		return trv::error;
		break;
	case CLog::eError:
		return trv::error;
		break;
	case CLog::eWarning:
		return trv::warning;
		break;
	case CLog::eNotice:
		return trv::info;
		break;
	case CLog::eInformational:
		return trv::info;
		break;
	case CLog::eDebug:
		return trv::debug;
		break;
	default:
		return trv::fatal;
		break;
	}
}

/**	\brief Setzt den Pfad für die Dateiausgabe des Protokolls.
 *
 *	\param[in] sFilePath Der Pfad der Protokolldatei.
 */
void CLog::InitLogging(const std::wstring_view sFilePath)
{
	if (sFilePath.size() == 0)
	{
		// Create an event log sink
		boost::shared_ptr< sink2_t > sink
		(
			new sink2_t
			(
				(
					keywords::format =
					(
						expr::stream
						<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
						<< " [" << expr::attr<logging::thread_id>("ThreadID") << "]"
						<< " <" << logging::trivial::severity
						<< ">: " << expr::smessage
						)
					),
				keywords::log_name = "GraphicsOverlay",
				keywords::log_source = "GraphicsOverlay"
			)
		);


		// Create and fill in another level translator for "MyLevel" attribute of type string
		sinks::event_log::custom_event_type_mapping< trv::severity_level >  mapping("Severity");
		mapping[trv::fatal] = sinks::event_log::error;
		mapping[trv::error] = sinks::event_log::error;
		mapping[trv::warning] = sinks::event_log::warning;
		mapping[trv::info] = sinks::event_log::info;
		mapping[trv::debug] = sinks::event_log::info;
		mapping[trv::trace] = sinks::event_log::info;
		sink->locked_backend()->set_event_type_mapper(mapping);

		// Add the sink to the core
		logging::core::get()->add_sink(sink);
		return;
	}
	std::wstring filePath(sFilePath);
	filePath.append(L"_%N.log");
	auto filebackend = logging::add_file_log(
		keywords::file_name = filePath,                 /*< file name pattern >*/
		keywords::rotation_size = 1024 * 1024 * 1024,   /*< rotate files every 1 GiB... >*/
		keywords::format =
		(
			expr::stream
			<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
			<< " [" << expr::attr<logging::thread_id>("ThreadID") << "]"
			<< " <" << logging::trivial::severity
			<< ">: " << expr::smessage
			)
	);
	// Create a new backend
	boost::shared_ptr< sink_t > debugbackend(new sink_t());
	debugbackend->set_filter(expr::is_debugger_present());
	debugbackend->set_formatter(
		expr::format("%1% [%2%] <%3%>: %4% \n")
		% expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
		% expr::attr<logging::thread_id>("ThreadID")
		% logging::trivial::severity
		% expr::smessage
	);

	// Wrap it into the frontend and register in the core.
	logging::core::get()->add_sink(debugbackend);

	logging::add_common_attributes();

	CLogFileLogger = filebackend;
#ifdef _DEBUG
	SetSeverityCeiling(eDebug);
#else
	SetSeverityCeiling(eInformational);
#endif
}

/**	\brief Fügt dem Protokoll einen Eintrag hinzu.
 *
 *	\param[in] nLogLevel Die Wichtigkeitsgrad des Eintrags. Anhand des
 *		Wichtigkeitsgrads kann das Protokoll gefiltert werden.
 *	\param[in] sModule Der Name des Moduls, welches den Protokolleintrag
 *		verursacht. Der Modulname kann frei definiert werden.
 *	\param[in] sEvent Der Protokolleintrag. Kann FormatStrings enthalten.
 * \param[in, out] ... Die Parameter für den Formatstring in sEvent
 */
void CLog::Log(ELogLevel nLogLevel, const LPCTSTR sModule, const LPCTSTR sMessage, ...)
{
	std::wstring msg(sModule);
	msg.append(L": ");

	va_list v;
	va_start(v, sMessage);
	msg.append(CStringUtilities::Format(sMessage, v));
	va_end(v);

	using namespace logging::trivial;
	src::severity_logger< severity_level > lg;
	BOOST_LOG_SEV(lg, TranslateSeverity(nLogLevel)) << CStringUtilities::ConvertToStdString(msg);
	if (CLogFileLogger.get())
	{
		CLogFileLogger->flush();
	}
}

void CLog::SetSeverityCeiling(ELogLevel ceiling)
{
	logging::core::get()->set_filter
	(
		logging::trivial::severity >= TranslateSeverity(ceiling)
	);
}

std::wstring CLog::SeverityToString(ELogLevel nLogLevel)
{
	switch (nLogLevel)
	{
	case eDebug:
		return L"Debug";
	case eInformational:
		return L"Info";
	case eNotice:
		return L"Notice";
	case eWarning:
		return L"Warn";
	case eError:
		return L"Error";
	case eCritical:
		return L"Critical";
	case eAlert:
		return L"Alert";
	case eEmergency:
		return L"Emergency";
	default:
		return L"Unknown";
	}
}

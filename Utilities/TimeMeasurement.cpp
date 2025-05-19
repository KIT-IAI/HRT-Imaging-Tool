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

#include "TimeMeasurement.h"
#include "StringUtilities.h"
#include <string>

/**	\brief Der Standardkonstruktor.
 */
CTimeMeasurement::CTimeMeasurement()
{
	m_nDuration = 0;
	m_nOldDuration = 0;
	m_start = 0;
	m_bRunning = false;
}

/**	\brief Der Standarddestruktor.
 */
CTimeMeasurement::~CTimeMeasurement()
{
}

/**	\brief Startet eine Zeitmessung.
 *
 *	Das in vorhergegangenen Start()-Stop()-Zyklen erfasste Zeitkonto wird
 *	dabei nicht gelöscht.
 *
 *	Wenn die Zeitmessung bereits läuft wird der Aufruf von Start() ignoriert.
 *
 *	\see Stop()
 *	\see Reset()
 */
void CTimeMeasurement::Start()
{
	if (!m_bRunning)
	{
		m_start = clock();
		m_bRunning = true;
	}
}

/**	\brief Liest die aktuell gemessene Zeit aus, ohne die Zeiterfassung
 *	anzuhalten.
 *
 *	Die mit dieser Methode gemessene Zeit ergibt sich als Summe der seit dem
 *	letzten gültigen Aufruf von Start() vergangenen Dauer und der aus
 *	vorherigen Start()-Stop()-Zyklen gespeicherten Dauer.
 *
 *	Wenn die Zeitmessung angehalten wurde wird der Aufruf von TakeTime()
 *	ignoriert.
 *
 *	\see Stop()
 *	\see Start()
 */
void CTimeMeasurement::TakeTime()
{
	if (m_bRunning)
	{
		m_nDuration = m_nOldDuration + (clock() - m_start);
	}
}

/**	\brief Liest die aktuell gemessene Zeit aus und hält die Zeiterfassung
 *	an.
 *
 *	Die mit dieser Methode gemessene Zeit ergibt sich als Summe der seit dem
 *	letzten gültigen Aufruf von Start() vergangenen Dauer und der aus
 *	vorherigen Start()-Stop()-Zyklen gespeicherten Dauer.
 *
 *	Wenn die Zeitmessung bereits angehalten wurde wird der Aufruf von Stop()
 *	ignoriert.
 *
 *	\see TakeTime()
 *	\see Start()
 */
void CTimeMeasurement::Stop()
{
	if (m_bRunning)
	{
		m_nDuration = m_nOldDuration + (clock() - m_start);
		m_nOldDuration = m_nDuration;
		m_bRunning = false;
	}
}

/**	\brief Setzt die insgesamt gemessene Dauer sowie die in vorherigen
 *	Start()-Stop()-Zyklen gespeicherte Dauer auf 0 zurück.
 *
 *	Wenn die Zeiterfassung zum Zeitpunkt des Aufrufs läuft wird sie
 *	angehalten.
 *
 *	\see Start()
 *	\see Stop()
 */
void CTimeMeasurement::Reset()
{
	m_nDuration = 0;
	m_nOldDuration = 0;
	m_start = clock();
	m_bRunning = false;
}

/**	\brief Liefert die beim letzten gültigen Aufruf von Stop() oder
 *	TakeTime() gemessene Zeit in ganzen Millisekunden.
 *
 *	\return Die beim letzten gültigen Aufruf von Stop() oder TakeTime()
 *		gemessene Zeit in ganzen Millisekunden. Bruchteile werden immer
 *		abgerundet.
 */
long CTimeMeasurement::GetMilliseconds()
{
	return static_cast<long>((static_cast<double>(m_nDuration) / CLOCKS_PER_SEC) * 1000);
}

/**	\brief Liefert die beim letzten gültigen Aufruf von Stop() oder
 *	TakeTime() gemessene Zeit in ganzen Sekunden.
 *
 *	\return Die beim letzten gültigen Aufruf von Stop() oder TakeTime()
 *		gemessene Zeit in ganzen Sekunden. Bruchteile werden immer
 *		abgerundet.
 */
long CTimeMeasurement::GetSeconds()
{
	return m_nDuration / CLOCKS_PER_SEC;
}

/**	\brief Liefert die beim letzten gültigen Aufruf von Stop() oder
 *	TakeTime() gemessene Zeit in ganzen Minuten.
 *
 *	\return Die beim letzten gültigen Aufruf von Stop() oder TakeTime()
 *		gemessene Zeit in ganzen Minuten. Bruchteile werden immer abgerundet.
 */
long CTimeMeasurement::GetMinutes()
{
	return (m_nDuration / CLOCKS_PER_SEC) / 60;
}

/**	\brief Liefert die beim letzten gültigen Aufruf von Stop() oder
 *	TakeTime() gemessene Zeit in ganzen Stunden.
 *
 *	\return Die beim letzten gültigen Aufruf von Stop() oder TakeTime()
 *		gemessene Zeit in ganzen Stunden. Bruchteile werden immer abgerundet.
 */
long CTimeMeasurement::GetHours()
{
	return (m_nDuration / CLOCKS_PER_SEC) / 3600;
}

/**	\brief Liefert die beim letzten gültigen Aufruf von Stop() oder
 *	TakeTime() gemessene Zeit in ganzen Millisekunden als Zeichenkette.
 *
 *	\return Die beim letzten gültigen Aufruf von Stop() oder TakeTime()
 *		gemessene Zeit in ganzen Millisekunden als Zeichenkette. Bruchteile
 *		werden immer abgerundet.
 */
std::wstring CTimeMeasurement::GetMillisecondsStr()
{
	return std::to_wstring(GetMilliseconds());
}

/**	\brief Liefert die beim letzten gültigen Aufruf von Stop() oder
 *	TakeTime() gemessene Zeit in ganzen Sekunden als Zeichenkette.
 *
 *	\return Die beim letzten gültigen Aufruf von Stop() oder TakeTime()
 *		gemessene Zeit in ganzen Sekunden als Zeichenkette. Bruchteile werden
 *		immer abgerundet.
 */
std::wstring CTimeMeasurement::GetSecondsStr()
{
	return std::to_wstring(GetSeconds());
}

/**	\brief Liefert die beim letzten gültigen Aufruf von Stop() oder
 *	TakeTime() gemessene Zeit in ganzen Minuten als Zeichenkette.
 *
 *	\return Die beim letzten gültigen Aufruf von Stop() oder TakeTime()
 *		gemessene Zeit in ganzen Minuten als Zeichenkette. Bruchteile werden
 *		immer abgerundet.
 */
std::wstring CTimeMeasurement::GetMinutesStr()
{
	return std::to_wstring(GetMinutes());
}

/**	\brief Liefert die beim letzten gültigen Aufruf von Stop() oder
 *	TakeTime() gemessene Zeit in ganzen Stunden als Zeichenkette.
 *
 *	\return Die beim letzten gültigen Aufruf von Stop() oder TakeTime()
 *		gemessene Zeit in ganzen Stunden als Zeichenkette. Bruchteile werden
 *		immer abgerundet.
 */
std::wstring CTimeMeasurement::GetHoursStr()
{
	return std::to_wstring(GetHours());
}

/**	\brief Liefert die beim letzten gültigen Aufruf von Stop() oder
 *	TakeTime() gemessene Zeit als Zeichenkette im Format
 *	Stunden:Minuten:Sekunden.Millisekunden.
 *
 *	\return Die beim letzen gültigen Aufruf von Stop() oder TakeTime()
 *		gemessene Zeit als Zeichenkette im Format
 *		Stunden:Minuten:Sekunden.Millisekunden.
 */
std::wstring CTimeMeasurement::GetTimeStr()
{
	long nMillis = GetMilliseconds() % 1000;
	long nSeconds = GetSeconds() % 60;
	long nMinutes = GetMinutes() % 60;
	long nHours = GetHours();

	return CStringUtilities::Format(_T("%d:%02d:%02d.%03d"), nHours, nMinutes, nSeconds, nMillis);
}
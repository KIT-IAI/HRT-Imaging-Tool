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


/**	\brief Die Klasse CTimeMeasurement stellt Methoden zur Zeitmessung zur
 *	Verfügung.
 *
 *	Die Methoden orientieren sich an einer Stoppuhr. Mit Start() wird die
 *	Zeiterfassung gestartet, mit Stop() wird sie angehalten. Weitere
 *	Start()-Stop()-Zyklen werden auf das Zeitkonto aufaddiert. Per Reset()
 *	wird das Zeitkonto wieder auf 0 zurückgesetzt. TakeTime() liest die
 *	aktuelle Zeit aus, ohne dabei die Zeiterfassung zu stoppen.
 *
 *	Die beim letzten gültigen Aufruf von Stop() oder TakeTime() genommene
 *	Zeit kann mit den Get-Methoden der Klasse auf verschiedene Weise
 *	ausgelesen werden.
 *
 *	Ein Aufruf von Start() bei laufender Zeiterfassung wird ignoriert, ebenso
 *	ein Aufruf von Stop() oder TakeTime() bei angehaltener Zeiterfassung.
 *	Reset() wird immer durchgeführt. Ein Aufruf von Reset() bei laufender
 *	Zeiterfassung stoppt diese.
 */
class  CTimeMeasurement
{

public:

	CTimeMeasurement();
	virtual ~CTimeMeasurement();

	void Start();
	void TakeTime();
	void Stop();
	void Reset();

	long GetMilliseconds();
	long GetSeconds();
	long GetMinutes();
	long GetHours();
	long GetClocks();

	std::wstring GetMillisecondsStr();
	std::wstring GetSecondsStr();
	std::wstring GetMinutesStr();
	std::wstring GetHoursStr();
	std::wstring GetTimeStr();

protected:

	/**	\brief Die beim letzten Aufruf von Stop() oder TakeTime() gemessene
	 *	Dauer.
	 *
	 *	Wenn noch keine Dauer erfasst wurde, ist der Wert 0.0.
	 */
	long m_nDuration;

private:
	/**	\brief Der Zeitpunkt des letzten gültigen Aufrufs von Start().
	 */
	clock_t m_start;

	/**	\brief Speichert die bereits vor dem letzten Aufruf von Start()
	 *	erfasste Dauer.
	 */
	long m_nOldDuration;

	/**	\brief Gibt an, ob die Zeitmessung momentan läuft (<tt>true</tt>)
	 *	oder gestoppt ist (<tt>false</tt>).
	 *
	 *	\see Start()
	 *	\see Stop()
	 */
	bool m_bRunning;

};

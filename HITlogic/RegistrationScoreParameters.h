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
/*
* Beinhaltet alle Schwellwerte von Score-Werten
*/

using namespace std;

class CRegistrationScoreParameters
{
public:
	explicit CRegistrationScoreParameters(double minscoreRigid = 4.5, double minscoreFlexible = 10.0, double certaintyScoreRigid = 17.0)
		:
		fMinScore(minscoreRigid),
		fCertainScore(certaintyScoreRigid),
		fMinScoreFlexible(minscoreFlexible),
		bAutomaticThresholdDetection(false)

	{
		fScoreThrehold = std::make_shared<double>(fMinScore);
	};

	double fMinScore;
	double fMinScoreFlexible;
	double fCertainScore;
	bool bAutomaticThresholdDetection;

	double GetScoreThreshold() const { return max(*fScoreThrehold, fMinScore); }
	void SetScoreThreshold(double Value) { *fScoreThrehold = Value; }
	void SetScoreThresholdPrt(std::shared_ptr<double> ptr) { fScoreThrehold = ptr; };

private:
	std::shared_ptr<double> fScoreThrehold;			// This HAS to be a pointer because it is the only mutable value
};


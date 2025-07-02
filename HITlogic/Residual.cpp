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
#include "Residual.h"


CResidual CResidual::CreateFromRegistration(const CRigidRegistrationResult& regResult, DPoint referenceImageCoordinates, DPoint templateImageCoordinates)
{
	double registrationDistance = regResult.GetRegistrationDistance();
	double solutionDistance = referenceImageCoordinates.distanceFrom(templateImageCoordinates);

	CResidual residual;
	residual.m_fValue = abs(solutionDistance - registrationDistance);
	residual.m_fX = abs(abs(regResult.GetX()) - abs(templateImageCoordinates.m_x - referenceImageCoordinates.m_x));
	residual.m_fY = abs(abs(regResult.GetY()) - abs(templateImageCoordinates.m_y - referenceImageCoordinates.m_y));
	residual.m_bIsInitialized = true;

	return residual;
}

CResidual CResidual::CreateFromSubimageRegistration(const CRigidRegistrationResult& reg, DPoint referenceImageCoordinates, DPoint templateImageCoordinates)
{
    CResidual residual;
    int nSubPerImg = 12;
    int nSubHeight = 32;

    //index of the subimage in templateImage
    int subImgIndex1 = static_cast<int>(reg.GetSpecialSubImageRowIndex());

    //indices of the subimages in referenceImage
    double sub = -static_cast<double>(reg.GetY()) / nSubHeight;
    int subImgIndex21 = subImgIndex1 + static_cast<int>(std::floor(sub));
    int subImgIndex22 = subImgIndex21 + 1;

    if (subImgIndex21 < 0 || subImgIndex22 >= nSubPerImg)
        return residual;  // not valid


    const DPoint pos1 = DPoint(templateImageCoordinates.m_x, templateImageCoordinates.m_y);

    const DPoint pos21 = DPoint(referenceImageCoordinates.m_x, referenceImageCoordinates.m_y);
    const DPoint pos22 = DPoint(referenceImageCoordinates.m_x, referenceImageCoordinates.m_y);



    double q = sub - std::floor(sub);
    double p = 1.0 - q;

    DPoint pos2;
    pos2.m_x = p * pos21.m_x + q * pos22.m_x;
    pos2.m_y = p * pos21.m_y + q * pos22.m_y;

    double res_x = pos2.m_x - pos1.m_x - reg.GetX();
    double res_y = pos2.m_y - pos1.m_y - reg.GetY();

    residual.m_fX = res_x;
    residual.m_fY = res_y;
    residual.m_fValue = std::sqrt(res_x * res_x + res_y * res_y);
    residual.m_bIsInitialized = true;

    return residual;
}

CResidual::CResidual()
{
}


CResidual::~CResidual()
{
}


double CResidual::CalculateMeanResidual(vector<CResidual>& allResiduals)
{
	if (allResiduals.empty())
		return 0;

	double fMean = 0.0;
	for (auto residual : allResiduals)
		fMean += residual.GetValue();
	fMean /= allResiduals.size();
	return fMean;
}
double CResidual::CalculateStdevResidual(vector<CResidual>& allResiduals, double fMean)
{
	if (allResiduals.empty())
		return 0;

	double fStdev = 0.0;
	for (auto residual : allResiduals)
		fStdev += pow(residual.GetValue() - fMean, 2);
	fStdev = sqrt(fStdev / allResiduals.size());
	return fStdev;
}

double CResidual::CalculateMedianResidual(vector<CResidual>& allResiduals)
{
	if (allResiduals.empty())
		return 0.0;

	std::vector<double> values;
	values.reserve(allResiduals.size());

	for (const auto& residual : allResiduals)
		values.push_back(residual.GetValue());

	std::sort(values.begin(), values.end());

	size_t n = values.size();
	if (n % 2 == 0)
		return (values[n / 2 - 1] + values[n / 2]) / 2.0;
	else
		return values[n / 2];
}

double CResidual::CalculateQuantileResidual(vector<CResidual>& allResiduals, double quantile)
{
	if (allResiduals.empty() || quantile <= 0.0 || quantile >= 1.0)
		return 0.0;

	std::vector<double> values;
	values.reserve(allResiduals.size());

	for (const auto& residual : allResiduals)
		values.push_back(residual.GetValue());

	std::sort(values.begin(), values.end());

	size_t index = static_cast<size_t>(std::ceil(quantile * values.size())) - 1;
	if (index >= values.size())
		index = values.size() - 1;

	return values[index];
}

double CResidual::CalculateMaximumResidual(vector<CResidual>& allResiduals)
{
	if (allResiduals.empty())
		return 0.0;

	double maxVal = allResiduals[0].GetValue();
	for (const auto& residual : allResiduals)
	{
		double val = residual.GetValue();
		if (val > maxVal)
			maxVal = val;
	}
	return maxVal;
}
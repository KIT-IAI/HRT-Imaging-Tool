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

CResidual CResidual::CreateFromSubimageRegistration(const CRigidRegistrationResult& reg, std::shared_ptr<CDenseMatrix> pRigidSolution, size_t subImageHeight, size_t subPerImg)
{
	CResidual residual;

	residual.m_fReferenceImageIndex = reg.GetReferenceImageIndex();
	residual.m_fTemplateImageIndex = reg.GetTemplateImageIndex();
	residual.m_fValidity = reg.GetValidity();
	residual.m_fXreg = reg.GetX();
	residual.m_fYreg = reg.GetY();

	//index of the subimage in referenceImage
	size_t subImg1 = reg.GetSubImageRowIndex();

	residual.m_fSubImageIndex = subImg1;
	residual.m_fScore = reg.GetScore();

	//indices in templateImage
	double subImg2 = -static_cast<double>(reg.GetY()) / subImageHeight;
	int subImg21 = static_cast<int>(subImg1) + static_cast<int>(std::floor(subImg2));
	int subImg22 = subImg21 + 1;

	if (subImg21 < 0 || subImg22 >= subPerImg)
		return residual;  // not valid


	size_t subImgIndex1 = residual.m_fReferenceImageIndex * subPerImg + subImg1;
	size_t subImgIndex21 = residual.m_fTemplateImageIndex * subPerImg + subImg21;
	size_t subImgIndex22 = residual.m_fTemplateImageIndex * subPerImg + subImg22;

	assert(pRigidSolution->Cols() == 2);
	auto getSubImagePosition = [&pRigidSolution](size_t subImageIndex)
		{
			assert(pRigidSolution->Rows() > subImageIndex);
			return DPoint((*pRigidSolution)[subImageIndex][0], (*pRigidSolution)[subImageIndex][1]);
		};

	DPoint pos1 = getSubImagePosition(subImgIndex1);
	DPoint pos21 = getSubImagePosition(subImgIndex21);
	DPoint pos22 = getSubImagePosition(subImgIndex22);

	double q = subImg2 - std::floor(subImg2);
	double p = 1.0 - q;


	DPoint pos2;
	pos2.m_x = p * pos21.m_x + q * pos22.m_x;
	pos2.m_y = p * pos21.m_y + q * pos22.m_y;

	double res_x = (pos2.m_x - pos1.m_x) - reg.GetX();
	double res_y = (pos2.m_y - pos1.m_y) - reg.GetY();

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

double CResidual::CalculateMeanResidual(const std::vector<CResidual>& allResiduals)
{
	if (allResiduals.empty())
		return 0;

	double fMean = 0.0;
	for (auto residual : allResiduals)
		fMean += residual.GetValue();
	fMean /= allResiduals.size();
	return fMean;
}

double CResidual::CalculateStdevResidual(const std::vector<CResidual>& allResiduals, double fMean)
{
	if (allResiduals.empty())
		return 0;

	double fStdev = 0.0;
	for (auto residual : allResiduals)
		fStdev += pow(residual.GetValue() - fMean, 2);
	fStdev = sqrt(fStdev / allResiduals.size());
	return fStdev;
}

double CResidual::CalculateMedianResidual(const std::vector<CResidual>& allResiduals)
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

double CResidual::CalculateQuantileResidual(const std::vector<CResidual>& allResiduals, double quantile)
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

double CResidual::CalculateMaximumResidual(const std::vector<CResidual>& allResiduals)
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

int CResidual::CountResidualsAboveThreshold(const std::vector<CResidual>& allResiduals, double lowerBound, double upperBound)
{
	int count = 0;
	for (const auto& residual : allResiduals)
	{
		double val = residual.GetValue();
		if (val > lowerBound && val <= upperBound)
			++count;
	}
	return count;
}

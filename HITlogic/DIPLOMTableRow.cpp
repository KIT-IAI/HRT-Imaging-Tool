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
#include "DIPLOMTableRow.h"



CSQLiteTableRow CDIPLOMTableRow::From(const CRigidRegistrationResult& RegResult)
{
	CSQLiteTableRow row;

	row.AddKeyValuePair(L"X", RegResult.GetX());
	row.AddKeyValuePair(L"Y", RegResult.GetY());
	row.AddKeyValuePair(L"Phi", RegResult.GetPhi());
	row.AddKeyValuePair(L"Dx", RegResult.GetDx());
	row.AddKeyValuePair(L"Dy", RegResult.GetDy());
	row.AddKeyValuePair(L"Score", RegResult.GetScore());
	row.AddKeyValuePair(L"BlockIndex", RegResult.GetBlockIndex());
	row.AddKeyValuePair(L"SubImageRowIndex", RegResult.GetSubImageRowIndex());
	row.AddKeyValuePair(L"SubImageColumnIndex", RegResult.GetSubImageColumnIndex());
	row.AddKeyValuePair(L"SubImageHeight", RegResult.GetSubImageHeight());
	row.AddKeyValuePair(L"SpecialImageRowIndex", RegResult.GetSpecialImageRowIndex());
	row.AddKeyValuePair(L"SpecialImageColumnIndex", RegResult.GetSpecialImageColumnIndex());
	row.AddKeyValuePair(L"ReferenceImageIndex", RegResult.GetReferenceImageIndex());
	row.AddKeyValuePair(L"TemplateImageIndex", RegResult.GetTemplateImageIndex());
	row.AddKeyValuePair(L"ScoreNorm2", 0);
	row.AddKeyValuePair(L"ScoreBeforeNorm", RegResult.GetScoreBeforeNorm());
	row.AddKeyValuePair(L"Validity", RegResult.GetValidity());
	row.AddKeyValuePair(L"SpecialSubImageRowIndex", RegResult.GetSpecialSubImageRowIndex());
	row.AddKeyValuePair(L"OffsetStartingPointX", RegResult.GetOffsetStartingPointX());
	row.AddKeyValuePair(L"OffsetStartingPointY", RegResult.GetOffsetStartingPointY());

	return row;
}

template <> CRigidRegistrationResult CSQLiteTableRow::Convert() const
{
	CRigidRegistrationResult result;
	size_t i = 0;

	result.SetX(Get<float>(i++));
	result.SetY(Get<float>(i++));
	result.SetPhi(Get<float>(i++));
	result.SetDx(Get<float>(i++));
	result.SetDy(Get<float>(i++));
	result.SetScore(Get<float>(i++));
	result.SetBlockIndex(Get<size_t>(i++));
	result.SetSubImageRowIndex(Get<size_t>(i++));
	result.SetSubImageColumnIndex(Get<size_t>(i++));
	result.SetSubImageHeight(Get<size_t>(i++));
	result.SetSpecialImageRowIndex(Get<size_t>(i++));
	result.SetSpecialImageColumnIndex(Get<size_t>(i++));
	result.SetReferenceImageIndex(Get<size_t>(i++));
	result.SetTemplateImageIndex(Get<size_t>(i++));
	i++; //result.SetScoreNorm2(Get<float>(i++));
	result.SetScoreBeforeNorm(Get<float>(i++));
	result.SetValidity(static_cast<CHrtValidityCodes::EValidityCode>(Get<int>(i++)));
	result.SetSpecialSubImageRowIndex(Get<size_t>(i++));
	result.SetOffsetStartingPointX(Get<double>(i++));
	result.SetOffsetStartingPointY(Get<double>(i++));

	return result;
}

CSQLiteTableRow CDIPLOMTableRow::From(const CMosaicImageProperties& MosaicProperties)
{
	CSQLiteTableRow Row;
	Row.AddKeyValuePair(L"GroupID", MosaicProperties.nGroupID);
	Row.AddKeyValuePair(L"MosaicImageSizeX", MosaicProperties.MosaicImageSize.x);
	Row.AddKeyValuePair(L"MosaicImageSizeY", MosaicProperties.MosaicImageSize.y);
	Row.AddKeyValuePair(L"DefinedMosaicArea", MosaicProperties.nDefinedMosaicArea);
	Row.AddKeyValuePair(L"FilePath", MosaicProperties.sFilePath);

	return Row;
}

template <> CMosaicImageProperties CSQLiteTableRow::Convert() const
{
	CMosaicImageProperties Properties;
	Properties.nGroupID = Get<size_t>(0);
	Properties.MosaicImageSize.x = Get<long long>(1);
	Properties.MosaicImageSize.y = Get<long long>(2);
	Properties.nDefinedMosaicArea = Get<size_t>(3);
	Properties.sFilePath = Get<wstring>(4);

	return Properties;
}

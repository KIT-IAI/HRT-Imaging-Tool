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

#include "RigidRegistrationResult.h"
#include "Point.h"
#include "TextFileOutput.h"

/**	\brief Der Standard-Konstruktor.
 */
CRigidRegistrationResult::CRigidRegistrationResult()
{
	m_fX = 0;
	m_fY = 0;
	m_fPhi = 0;
	m_fDx = 0;
	m_fDy = 0;
	m_fScore = 0;
	m_fScoreBeforeNorm = 0;

	m_nValidity = CHrtValidityCodes::eValidityInitialization;
	m_nReferenceImageIndex = 0;
	m_nTemplateImageIndex = 0;
	m_nSubImageColumnIndex = 0;
	m_nSubImageRowIndex = 0;
	m_nSubImageHeight = 0;
	m_nSpecialImageColumnIndex = 0;
	m_nSpecialImageRowIndex = 0;
	m_nSpecialSubImageRowIndex = 0;
	m_nBlockIndex = 0;

	m_fOffsetStartingPointX = 0;
	m_fOffsetStartingPointY = 0;
}
CRigidRegistrationResult::CRigidRegistrationResult(const CCorrelationOffset& offset)
	:CRigidRegistrationResult()
{
	m_fX = offset.xOffset;
	m_fY = offset.yOffset;
	m_fScoreBeforeNorm = offset.m_scoreBeforeNorm;
	m_fScore = offset.m_scoreNorm1;
	m_nValidity = offset.GetValidity();
}

/**	\brief Der Standard-Destruktor.
 */
CRigidRegistrationResult::~CRigidRegistrationResult()
{
}

/**	\brief Liefert die x-Komponente des berechneten Translationsvektors.
 *
 *	\return Die x-Komponente des berechneten Translationsvektors.
 */
float CRigidRegistrationResult::GetX() const
{
	return m_fX;
}

/**	\brief Setzt die x-Komponente des berechneten Translationsvektors.
 *
 *	\param[in] val Die x-Komponente des berechneten Translationsvektors.
 */
void CRigidRegistrationResult::SetX(float val)
{
	m_fX = val;
}

/**	\brief Liefert die y-Komponente des berechneten Translationsvektors.
 *
 *	\return Die y-Komponente des berechneten Translationsvektors.
 */
float CRigidRegistrationResult::GetY() const
{
	return m_fY;
}

/**	\brief Setzt die y-Komponente des berechneten Translationsvektors.
 *
 *	\param[in] val Die y-Komponente des berechneten Translationsvektors.
 */
void CRigidRegistrationResult::SetY(float val)
{
	m_fY = val;
}

/**	\brief Liefert den berechneten Rotationswinkel.
 *
 *	\return Den berechneten Rotationswinkel.
 */
float CRigidRegistrationResult::GetPhi() const
{
	return m_fPhi;
}

/**	\brief Setzt den berechneten Rotationswinkel.
 *
 *	\param[in] val Der berechnete Rotationswinkel.
 */
void CRigidRegistrationResult::SetPhi(float val)
{
	m_fPhi = val;
}

/**	\brief Liefert den Gradienten in x-Richtung der berechneten Translation.
 *
 *	Dieser Wert wird nur für die Registrierung zeitlich kontinuierlicher
 *	Aufnahmeserien benötigt.
 *
 *	\return Den Gradienten in x-Richtung der berechneten Translation.
 */
float CRigidRegistrationResult::GetDx() const
{
	return m_fDx;
}

/**	\brief Setzt den Gradienten in x-Richtung der berechneten Translation.
 *
 *	Dieser Wert wird nur für die Registrierung zeitlich kontinuierlicher
 *	Aufnahmeserien benötigt.
 *
 *	\param[in] val Der Gradient in x-Richtung der berechneten Translation.
 */
void CRigidRegistrationResult::SetDx(float val)
{
	m_fDx = val;
}

/**	\brief Liefert den Gradienten in y-Richtung der berechneten Translation.
 *
 *	Dieser Wert wird nur für die Registrierung zeitlich kontinuierlicher
 *	Aufnahmeserien benötigt.
 *
 *	\return Den Gradienten in y-Richtung der berechneten Translation.
 */
float CRigidRegistrationResult::GetDy() const
{
	return m_fDy;
}

/**	\brief Setzt den Gradienten in y-Richtung der berechneten Translation.
 *
 *	Dieser Wert wird nur für die Registrierung zeitlich kontinuierlicher
 *	Aufnahmeserien benötigt.
 *
 *	\param[in] val Der Gradient in y-Richtung der berechneten Translation.
 */
void CRigidRegistrationResult::SetDy(float val)
{
	m_fDy = val;
}

/**	\brief Liefert den normierten Gütewert der Registrierung.
 *
 *	\return Den normierten Gütewert der Registrierung.
 */
float CRigidRegistrationResult::GetScore() const
{
	return m_fScore;
}

/**	\brief Setzt den normierten Gütewert der Registrierung.
 *
 *	\param[in] val Der normierte Gütewert der Registrierung.
 */
void CRigidRegistrationResult::SetScore(float val)
{
	m_fScore = val;
}

/**	\brief Liefert den nicht normierten Gütewert der Registrierung.
 *
 *	\return Den nicht normierten Gütewert der Registrierung.
 */
float CRigidRegistrationResult::GetScoreBeforeNorm() const
{
	return m_fScoreBeforeNorm;
}

/**	\brief Setzt den nicht normierten Gütewert der Registrierung.
 *
 *	\param[in] val Der nicht normierte Gütewert der Registrierung.
 */
void CRigidRegistrationResult::SetScoreBeforeNorm(float val)
{
	m_fScoreBeforeNorm = val;
}

/**	\brief Liefert das Flag zur Anzeige der Gültigkeit der Registrierung
 *	unabhängig von den Gütewerten.
 *
 *	\return Das Flag zur Anzeige der Gültigkeit der Registrierung unabhängig
 *		von den Gütewerten.
 */
CHrtValidityCodes::EValidityCode CRigidRegistrationResult::GetValidity() const
{
	return m_nValidity;
}

/**	\brief Setzt das Flag zur Anzeige der Gültigkeit der Registrierung
 *	unabhängig von den Gütewerten.
 *
 *	\param[in] val Das Flag zur Anzeige der Gültigkeit der Registrierung
 *		unabhängig von den Gütewerten.
 */
void CRigidRegistrationResult::SetValidity(CHrtValidityCodes::EValidityCode val)
{
	m_nValidity = val;
}

/**	\brief Liefert den Index des Referenzbilds.
 *
 *	Dieser Wert wird nur bei der Registrierung von Bildserien benötigt.
 *
 *	\return Den Index des Referenzbilds.
 */
size_t CRigidRegistrationResult::GetReferenceImageIndex() const
{
	return m_nReferenceImageIndex;
}

/**	\brief Setzt den Index des Referenzbilds.
 *
 *	Dieser Wert wird nur bei der Registrierung von Bildserien benötigt.
 *
 *	\param[in] val Der Index des Referenzbilds.
 */
void CRigidRegistrationResult::SetReferenceImageIndex(size_t val)
{
	m_nReferenceImageIndex = val;
}

/**	\brief Liefert den Index des Templatebilds.
 *
 *	Dieser Wert wird nur bei der Registrierung von Bildserien benötigt.
 *
 *	\return Den Index des Templatebilds.
 */
size_t CRigidRegistrationResult::GetTemplateImageIndex() const
{
	return m_nTemplateImageIndex;
}

/**	\brief Setzt den Index des Templatebilds.
 *
 *	Dieser Wert wird nur bei der Registrierung von Bildserien benötigt.
 *
 *	\param[in] val Der Index des Templatebilds.
 */
void CRigidRegistrationResult::SetTemplateImageIndex(size_t val)
{
	m_nTemplateImageIndex = val;
}

/**	\brief Liefert den Index des Blocks.
 *
 *	\return Den Index des Blocks.
 */
size_t CRigidRegistrationResult::GetBlockIndex() const
{
	return m_nBlockIndex;
}

/**	\brief Setzt den Index des Blocks.
 *
 *	\param[in] val Der Index des Blocks.
 */
void CRigidRegistrationResult::SetBlockIndex(size_t val)
{
	m_nBlockIndex = val;
}

/**	\brief Liefert den Spaltenindex des Teilbilds.
 *
 *	Dieser Wert wird nur bei der Registrierung von Teilbildern benötigt.
 *
 *	\return Den Spaltenindex des Teilbilds.
 */
size_t CRigidRegistrationResult::GetSubImageColumnIndex() const
{
	return m_nSubImageColumnIndex;
}

/**	\brief Setzt den Spaltenindex des Teilbilds.
 *
 *	Dieser Wert wird nur bei der Registrierung von Teilbildern benötigt.
 *
 *	\paam[in] val Der Spaltenindex des Teilbilds.
 */
void CRigidRegistrationResult::SetSubImageColumnIndex(size_t val)
{
	m_nSubImageColumnIndex = val;
}

/**	\brief Liefert den Zeilenindex des Teilbilds.
 *
 *	Dieser Wert wird nur bei der Registrierung von Teilbildern benötigt.
 *
 *	\return Den Zeilenindex des Teilbilds.
 */
size_t CRigidRegistrationResult::GetSubImageRowIndex() const
{
	return m_nSubImageRowIndex;
}

/**	\brief Setzt den Zeilenindex des Teilbilds.
 *
 *	Dieser Wert wird nur bei der Registrierung von Teilbildern benötigt.
 *
 *	\paam[in] val Der Zeilenindex des Teilbilds.
 */
void CRigidRegistrationResult::SetSubImageRowIndex(size_t val)
{
	m_nSubImageRowIndex = val;
}

/**	\brief Liefert die Höhe des Teilbilds (in Pixeln).
 *
 *	Dieser Wert wird nur bei der Registrierung von Teilbildern benötigt.
 *
 *	\return Die Höhe des Teilbilds (in Pixeln).
 */
size_t CRigidRegistrationResult::GetSubImageHeight() const
{
	return m_nSubImageHeight;
}

/**	\brief Setzt die Höhe des Teilbilds (in Pixeln).
 *
 *	Dieser Wert wird nur bei der Registrierung von Teilbildern benötigt.
 *
 *	\param[in] val Die Höhe des Teilbilds (in Pixeln).
 */
void CRigidRegistrationResult::SetSubImageHeight(size_t val)
{
	m_nSubImageHeight = val;
}

/**	\brief Liefert den Index einer für die Registrierung relevanten
 *	Bildspalte.
 *
 *	Die Bedeutung der Bildspalte ist vom eingesetzten Registrierverfahren
 *	abhängig.
 *
 *	\return Den Index einer für die Registrierung relevanten Bildspalte.
 */
size_t CRigidRegistrationResult::GetSpecialImageColumnIndex() const
{
	return m_nSpecialImageColumnIndex;
}

/**	\brief Setzt den Index einer für die Registrierung relevanten Bildspalte.
 *
 *	Die Bedeutung der Bildspalte ist vom eingesetzten Registrierverfahren
 *	abhängig.
 *
 *	\param[in] val Der Index einer für die Registrierung relevanten
 *		Bildspalte.
 */
void CRigidRegistrationResult::SetSpecialImageColumnIndex(size_t val)
{
	m_nSpecialImageColumnIndex = val;
}

/**	\brief Liefert den Index einer für die Registrierung relevanten
 *	Bildzeile.
 *
 *	Die Bedeutung der Bildzeile ist vom eingesetzten Registrierverfahren
 *	abhängig.
 *
 *	\return Den Index einer für die Registrierung relevanten Bildzeile.
 */
size_t CRigidRegistrationResult::GetSpecialImageRowIndex() const
{
	return m_nSpecialImageRowIndex;
}

/**	\brief Setzt den Index einer für die Registrierung relevanten Bildzeile.
 *
 *	Die Bedeutung der Bildzeile ist vom eingesetzten Registrierverfahren
 *	abhängig.
 *
 *	\param[in] val Der Index einer für die Registrierung relevanten
 *		Bildzeile.
 */
void CRigidRegistrationResult::SetSpecialImageRowIndex(size_t val)
{
	m_nSpecialImageRowIndex = val;
}

/**	\brief Liefert den Zeilenindex eines für die Bildregistrierung relevanten
 *	Teilbilds.
 *
 *	Dieser Wert wird nur bei der Registrierung von Teilbildern benötigt. Die
 *	Bedeutung des Teilbilds ist vom eingesetzten Registrierverfahren
 *	abhängig.
 *
 *	\return Den Zeilenindex eines für die Bildregistrierung relevanten
 *		Teilbilds.
 */
size_t CRigidRegistrationResult::GetSpecialSubImageRowIndex() const
{
	return m_nSpecialSubImageRowIndex;
}

/**	\brief Setzt den Zeilenindex eines für die Bildregistrierung relevanten
 *	Teilbilds.
 *
 *	Dieser Wert wird nur bei der Registrierung von Teilbildern benötigt. Die
 *	Bedeutung des Teilbilds ist vom eingesetzten Registrierverfahren
 *	abhängig.
 *
 *	\param[in] val Der Zeilenindex eines für die Bildregistrierung relevanten
 *		Teilbilds.
 */
void CRigidRegistrationResult::SetSpecialSubImageRowIndex(size_t val)
{
	m_nSpecialSubImageRowIndex = val;
}

/**	\brief Liefert den Versatz in x-Richtung zum ersten Bild der Bildserie.
 *
 *	Dieser Wert wird nur bei der Registrierung von Bildserien benötigt.
 *
 *	\return Den Versatz in x-Richtung zum ersten Bild der Bildserie.
 */
double CRigidRegistrationResult::GetOffsetStartingPointX() const
{
	return m_fOffsetStartingPointX;
}

/**	\brief Setzt den Versatz in x-Richtung zum ersten Bild der Bildserie.
 *
 *	Dieser Wert wird nur bei der Registrierung von Bildserien benötigt.
 *
 *	\param[in] val Der Versatz in x-Richtung zum ersten Bild der Bildserie.
 */
void CRigidRegistrationResult::SetOffsetStartingPointX(double val)
{
	m_fOffsetStartingPointX = val;
}

/**	\brief Liefert den Versatz in y-Richtung zum ersten Bild der Bildserie.
 *
 *	Dieser Wert wird nur bei der Registrierung von Bildserien benötigt.
 *
 *	\return Den Versatz in y-Richtung zum ersten Bild der Bildserie.
 */
double CRigidRegistrationResult::GetOffsetStartingPointY() const
{
	return m_fOffsetStartingPointY;
}

/**	\brief Setzt den Versatz in y-Richtung zum ersten Bild der Bildserie.
 *
 *	Dieser Wert wird nur bei der Registrierung von Bildserien benötigt.
 *
 *	\param[in] val Der Versatz in y-Richtung zum ersten Bild der Bildserie.
 */
void CRigidRegistrationResult::SetOffsetStartingPointY(double val)
{
	m_fOffsetStartingPointY = val;
}

CRigidRegistrationResult::CRigidRegistrationResult(CDenseVector pResultLine)
{
	if (pResultLine.Size() != m_nValueCount)
	{
		assert(false);
		m_nValidity = CHrtValidityCodes::eInvalidInit;
		return;
	}
	m_fX = static_cast<float>(pResultLine[0]);
	m_fY = static_cast<float>(pResultLine[1]);
	m_fPhi = static_cast<float>(pResultLine[2]);
	m_fDx = static_cast<float>(pResultLine[3]);
	m_fDy = static_cast<float>(pResultLine[4]);
	m_fScore = static_cast<float>(pResultLine[5]);
	m_nBlockIndex = CMathTools::RoundValue(pResultLine[6]);
	m_nSubImageRowIndex = CMathTools::RoundValue(pResultLine[7]);
	m_nSubImageColumnIndex = CMathTools::RoundValue(pResultLine[8]);
	m_nSubImageHeight = CMathTools::RoundValue(pResultLine[9]);
	m_nSpecialImageRowIndex = CMathTools::RoundValue(pResultLine[10]);
	m_nSpecialImageColumnIndex = CMathTools::RoundValue(pResultLine[11]);
	m_nReferenceImageIndex = CMathTools::RoundValue(pResultLine[12]);
	m_nTemplateImageIndex = CMathTools::RoundValue(pResultLine[13]);
	m_fScoreBeforeNorm = static_cast<float>(pResultLine[15]);
	m_nValidity = static_cast<CHrtValidityCodes::EValidityCode>(CMathTools::RoundValue(pResultLine[16]));
	m_nSpecialSubImageRowIndex = CMathTools::RoundValue(pResultLine[17]);
	m_fOffsetStartingPointX = static_cast<float>(pResultLine[18]);
	m_fOffsetStartingPointY = static_cast<float>(pResultLine[19]);
}

/**	\brief Erzeugt eine Zeichenkette mit ausgewählten Werten des Objekts.
 *
 *	\return Eine Zeichenkette mit ausgewählten Werten des Objekts.
 */
wstring CRigidRegistrationResult::ToString() const
{
	wstring out = L"";
	out += L"\t" + std::to_wstring(m_fX);
	out += L"\t" + std::to_wstring(m_fY);
	out += L"\t" + std::to_wstring(m_fPhi);
	out += L"\t" + std::to_wstring(m_fDx);
	out += L"\t" + std::to_wstring(m_fDy);
	out += L"\t" + std::to_wstring(m_fScore);
	out += L"\t" + std::to_wstring(m_nBlockIndex);
	out += L"\t" + std::to_wstring(m_nSubImageRowIndex);
	out += L"\t" + std::to_wstring(m_nSubImageColumnIndex);
	out += L"\t" + std::to_wstring(m_nSubImageHeight);
	out += L"\t" + std::to_wstring(m_nSpecialImageRowIndex);
	out += L"\t" + std::to_wstring(m_nSpecialImageColumnIndex);
	out += L"\t" + std::to_wstring(m_nReferenceImageIndex);
	out += L"\t" + std::to_wstring(m_nTemplateImageIndex);
	out += L"\t" + std::to_wstring(0);
	out += L"\t" + std::to_wstring(m_fScoreBeforeNorm);
	out += L"\t" + std::to_wstring(m_nValidity);
	out += L"\t" + std::to_wstring(m_nSpecialSubImageRowIndex);
	out += L"\t" + std::to_wstring(m_fOffsetStartingPointX);
	out += L"\t" + std::to_wstring(m_fOffsetStartingPointY);
	return out;
}

double CRigidRegistrationResult::GetRegistrationDistance() const
{
	DPoint p1;
	DPoint p2(m_fX, m_fY);
	return p1.distanceFrom(p2);
}
bool CRigidRegistrationResult::IsValid() const
{
	return GetValidity() > 0;
}

vector<CRigidRegistrationResult> CRigidRegistrationResult::LoadFromFile(const wstring& fileName)
{
	vector<CRigidRegistrationResult> results;

	auto nLineCount = CFileUtilities::CountNumberOfLines(fileName, true);

	if (nLineCount <= 0)
		throw std::exception("File is empty.");

	CDenseMatrix m(nLineCount, m_nValueCount);

	if (!m.ReadMatrix(fileName))
		throw std::exception("Could not read File.");

	results.reserve(m.Rows());
	for (size_t i = 0; i < m.Rows(); i++)
		results.push_back(CRigidRegistrationResult(m[i]));

	return results;
}

bool CRigidRegistrationResult::WriteToFile(const wstring& fileName, const vector<CRigidRegistrationResult>& results)
{
	CTextFileOutput file;


	for (const auto& result : results)
	{
		file.Writeln(result.ToString());
	}
	return file.Save(fileName);
}

vector<CRigidRegistrationResult> CRigidRegistrationResult::FilterRegistrationResults(const vector<CRigidRegistrationResult>& listIn, const std::list<size_t>& listIndexes)
{
	if (listIndexes.empty())
		return{};

	vector<CRigidRegistrationResult> listOut;

	for (size_t i = 0; i < listIn.size(); i++)
	{
		INT_PTR indexref = -1;
		INT_PTR indextem = -1;
		size_t index = 0;

		for (auto indexorg : listIndexes)
		{
			if (indexorg == listIn[i].GetReferenceImageIndex())
			{
				indexref = index;
			}
			if (indexorg == listIn[i].GetTemplateImageIndex())
			{
				indextem = index;
			}
			index++;
		}
		if (indexref >= 0 && indextem >= 0)
		{
			CRigidRegistrationResult tmp = CRigidRegistrationResult(listIn[i]);
			tmp.SetReferenceImageIndex(indexref);
			tmp.SetTemplateImageIndex(indextem);
			listOut.push_back(tmp);
		}
	}
	return listOut;
}

void CRigidRegistrationResult::SearchRegistrationResults(const vector<CRigidRegistrationResult>& listIn, vector<CRigidRegistrationResult>& listOut, size_t searchedImage)
{
	listOut.clear();

	for (auto lResult : listIn)
	{
		if (lResult.GetTemplateImageIndex() == searchedImage || lResult.GetReferenceImageIndex() == searchedImage)
			listOut.push_back(lResult);
	}
}

/*
Returns a List containing all registrations that involve the searched image AND where the other image is inbetween the interval from <= i < to.
*/
void CRigidRegistrationResult::SearchRegistrationResults(const vector<CRigidRegistrationResult>& listIn, vector<CRigidRegistrationResult>& listOut, size_t searchedImage, size_t from, size_t to)
{
	listOut.clear();

	for (auto lResult : listIn)
	{
		if (lResult.GetTemplateImageIndex() == searchedImage && (lResult.GetReferenceImageIndex() >= from && lResult.GetReferenceImageIndex() < to))
			listOut.push_back(lResult);
		else if (lResult.GetReferenceImageIndex() == searchedImage && (lResult.GetTemplateImageIndex() >= from && lResult.GetTemplateImageIndex() < to))
			listOut.push_back(lResult);
	}
}

void CRigidRegistrationResult::Scale(const CRigidRegistrationResult& source, double fScalationFactor)
{
	Copy(source);
	m_fX *= static_cast<float>(fScalationFactor);
	m_fY *= static_cast<float>(fScalationFactor);
	m_fDx *= static_cast<float>(fScalationFactor);
	m_fDy *= static_cast<float>(fScalationFactor);
	m_nSubImageHeight = static_cast<long>(m_nSubImageHeight * fScalationFactor);

	//m_fScore /= fScalationFactor;
	//m_fScoreBeforeNorm /= fScalationFactor;
	//m_fScoreNorm2 /= fScalationFactor;
	//double stdFull = 1 / sqrt(FullImageFFTSize.cx*FullImageFFTSize.cy);
	//double stdPartial = 1 / sqrt(PartialImageFFTSize.cx*PartialImageFFTSize.cy);
	//double fraction = sqrt(stdFull / stdPartial);
}
void CRigidRegistrationResult::Copy(const CRigidRegistrationResult& source)
{
	m_fX = source.m_fX;
	m_fY = source.m_fY;
	m_fPhi = source.m_fPhi;
	m_fDx = source.m_fDx;
	m_fDy = source.m_fDy;
	m_fScore = source.m_fScore;
	m_fScoreBeforeNorm = source.m_fScoreBeforeNorm;

	m_nValidity = source.m_nValidity;
	m_nReferenceImageIndex = source.m_nReferenceImageIndex;
	m_nTemplateImageIndex = source.m_nTemplateImageIndex;
	m_nSubImageColumnIndex = source.m_nSubImageColumnIndex;
	m_nSubImageRowIndex = source.m_nSubImageRowIndex;
	m_nSubImageHeight = source.m_nSubImageHeight;
	m_nSpecialImageColumnIndex = source.m_nSpecialImageColumnIndex;
	m_nSpecialImageRowIndex = source.m_nSpecialImageRowIndex;
	m_nSpecialSubImageRowIndex = source.m_nSpecialSubImageRowIndex;
	m_nBlockIndex = source.m_nBlockIndex;

	m_fOffsetStartingPointX = source.m_fOffsetStartingPointX;
	m_fOffsetStartingPointY = source.m_fOffsetStartingPointY;
}
void CRigidRegistrationResult::Negate()
{
	m_fX = -m_fX;
	m_fY = -m_fY;
}

CCorrelationOffset CRigidRegistrationResult::GetCorrelationOffset() const
{
	CCorrelationOffset correlationOffset;
	correlationOffset.xOffset = m_fX;
	correlationOffset.yOffset = m_fY;

	correlationOffset.m_scoreNorm1 = m_fScore;
	correlationOffset.m_scoreBeforeNorm = m_fScoreBeforeNorm;

	correlationOffset.SetValidity(m_nValidity);
	return correlationOffset;
}
CVector2d<float> CRigidRegistrationResult::GetOffset() const
{
	return CVector2d<float>(m_fX, m_fY);
}
void CRigidRegistrationResult::SetOffset(CVector2d<float> offset)
{
	m_fX = offset.x;
	m_fY = offset.y;
}
bool CRigidRegistrationResult::operator==(const CRigidRegistrationResult& other) const
{
	return Equals(other, 0.01f);
}
bool CRigidRegistrationResult::Equals(const CRigidRegistrationResult& other, float fTolerance) const
{
	return
		abs(m_fX - other.m_fX) < fTolerance &&
		abs(m_fY - other.m_fY) < fTolerance &&
		m_fPhi == other.m_fPhi &&
		abs(m_fDx - other.m_fDx) < fTolerance &&
		abs(m_fDy - other.m_fDy) < fTolerance &&
		abs(m_fScore - other.m_fScore) < fTolerance &&
		abs(m_fScoreBeforeNorm - other.m_fScoreBeforeNorm) < fTolerance &&
		m_nValidity == other.m_nValidity &&
		m_nReferenceImageIndex == other.m_nReferenceImageIndex &&
		m_nTemplateImageIndex == other.m_nTemplateImageIndex &&
		m_nSubImageColumnIndex == other.m_nSubImageColumnIndex &&
		m_nSubImageRowIndex == other.m_nSubImageRowIndex &&
		m_nSubImageHeight == other.m_nSubImageHeight &&
		m_nSpecialImageColumnIndex == other.m_nSpecialImageColumnIndex &&
		m_nSpecialImageRowIndex == other.m_nSpecialImageRowIndex &&
		m_nSpecialSubImageRowIndex == other.m_nSpecialSubImageRowIndex &&
		m_nBlockIndex == other.m_nBlockIndex &&
		abs(m_fOffsetStartingPointX - other.m_fOffsetStartingPointX) < fTolerance &&
		abs(m_fOffsetStartingPointY - other.m_fOffsetStartingPointY) < fTolerance;
	//m_eRTUsedStrategy isn't part of Equals, because there can be the same Registration from different strategies
}


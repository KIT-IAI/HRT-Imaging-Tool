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

#include "HrtValidityCodes.h"
#include "DenseMatrix.h"
#include "CorrelationOffset.h"
#include "Vectors.h"

#include <list>


/**	\brief Die Klasse CRigidRegistrationResult fasst zahlreiche Ergebniswerte
 *	einer einzelnen (zweidimensionalen) rigiden Bildregistrierung zusammen.
 *
 *	Neben den Transformationsparametern der rigiden Registrierung
 *	(Translationsvektor und Rotationswinkel) stehen zahlreiche Felder für
 *	weitere Informationen der Bildregistrierung zur Verfügung, z.B. zwei
 *	Bildindizes, zwei Teilbildindizes, Teilbildgröße und mehrere Gütewerte.
 */
class CRigidRegistrationResult
{
private:
	// CHANGE THIS VALUE IF YOU ADD A NEW VARIBALE!
	static const int m_nValueCount = 21;

public:
	CRigidRegistrationResult();
	explicit CRigidRegistrationResult(CDenseVector pResultLine);
	explicit CRigidRegistrationResult(const CCorrelationOffset& offset);
	~CRigidRegistrationResult();

	float GetX() const;
	float GetY() const;
	float GetPhi() const;
	float GetDx() const;
	float GetDy() const;
	float GetScore() const;
	float GetScoreBeforeNorm() const;

	CHrtValidityCodes::EValidityCode GetValidity() const;
	size_t GetReferenceImageIndex() const;
	size_t GetTemplateImageIndex() const;
	size_t GetSubImageColumnIndex() const;
	size_t GetSubImageRowIndex() const;
	size_t GetSubImageHeight() const;
	size_t GetSpecialImageColumnIndex() const;
	size_t GetSpecialImageRowIndex() const;
	size_t GetSpecialSubImageRowIndex() const;
	size_t GetBlockIndex() const;

	double GetOffsetStartingPointX() const;
	double GetOffsetStartingPointY() const;


	CCorrelationOffset GetCorrelationOffset() const;
	CVector2d<float> GetOffset() const;
	void SetOffset(CVector2d<float>);

	void SetX(float val);
	void SetY(float val);
	void SetPhi(float val);
	void SetDx(float val);
	void SetDy(float val);
	void SetScore(float val);
	void SetScoreBeforeNorm(float val);

	void SetValidity(CHrtValidityCodes::EValidityCode val);
	void SetReferenceImageIndex(size_t val);
	void SetTemplateImageIndex(size_t val);
	void SetSubImageColumnIndex(size_t val);
	void SetSubImageRowIndex(size_t val);
	void SetSubImageHeight(size_t val);
	void SetSpecialImageColumnIndex(size_t val);
	void SetSpecialImageRowIndex(size_t val);
	void SetSpecialSubImageRowIndex(size_t val);
	void SetBlockIndex(size_t val);

	void SetOffsetStartingPointX(double val);
	void SetOffsetStartingPointY(double val);

	std::wstring ToString() const;
	double GetRegistrationDistance() const;
	bool IsValid() const;

	//txt import
	static std::vector<CRigidRegistrationResult> LoadFromFile(const std::wstring& fileName);
	static bool WriteToFile(const std::wstring& fileName, const std::vector<CRigidRegistrationResult>& results);

	//Filter Stuff
	static std::vector<CRigidRegistrationResult> FilterRegistrationResults(const std::vector<CRigidRegistrationResult>& listIn, const std::list<size_t>& listIndexes);

	static void SearchRegistrationResults(const std::vector<CRigidRegistrationResult>& listIn, std::vector<CRigidRegistrationResult>& listOut, size_t searchedImage);
	static void SearchRegistrationResults(const std::vector<CRigidRegistrationResult>& listIn, std::vector<CRigidRegistrationResult>& listOut, size_t searchedImage, size_t from, size_t to);

	// Scalation
	void Scale(const CRigidRegistrationResult& source, double fScalationFactor);
	void Copy(const CRigidRegistrationResult& source);
	void Negate();

	// Operatoren
	bool Equals(const CRigidRegistrationResult& other, float fTolerance) const;
	bool operator==(const CRigidRegistrationResult& other) const;

private:

	float m_fX;
	float m_fY;
	float m_fPhi;
	float m_fDx;
	float m_fDy;
	float m_fScore;
	float m_fScoreBeforeNorm;

	CHrtValidityCodes::EValidityCode m_nValidity;
	size_t m_nReferenceImageIndex;
	size_t m_nTemplateImageIndex;
	size_t m_nSubImageColumnIndex;
	size_t m_nSubImageRowIndex;
	size_t m_nSubImageHeight;
	size_t m_nSpecialImageColumnIndex;
	size_t m_nSpecialImageRowIndex;
	size_t m_nSpecialSubImageRowIndex;
	size_t m_nBlockIndex;

	double m_fOffsetStartingPointX;
	double m_fOffsetStartingPointY;

};

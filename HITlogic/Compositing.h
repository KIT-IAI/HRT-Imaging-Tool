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

#include "CompositingParameters.h"
#include <atomic>
#include "Progressing.h"
#include "Cancelable.h"
#include "UndistortedImage.h"
#include <variant>
#include "ImageRegistrationData.h"

#include "3DCloud.h"
#include "3DBuffer.h"

#include "StlImage.h"

//hier unterstützte Resulttypen angeben
//in Fuse muss ein Eintrag in Fallunterscheidung erfolgen
//auch in SNPDatasetResultLoader::ExportResult zum ausgeben + ExportResultLegacy
typedef std::variant<	\
	StlImage<float>		\
	, C3DCloud			\
	, C3DBuffer<float>  \
> CSNPCompositingResult;




/**	Die Klasse CCompositing implementiert zwei verschiedene Verfahren
*	zum Erzeugen eines Mosaikbilds aus Einzelbildserien.
*
*/
class CCompositing : public IProgressing, public ICancelable
{
public:
	explicit CCompositing(const CCompositingParameters& Param);
	virtual ~CCompositing() = default;

	void SetParameters(const CCompositingParameters& Param);
	std::unique_ptr<CSNPCompositingResult> Fuse(const CImageRegistrationData& ImageData, const CDenseMatrix& PositioningSolution);
	virtual void PerformCompositing(const CImageRegistrationData& ImageData, const CDenseMatrix& PositioningSolution) = 0;

	const CSNPCompositingResult* GetResult() const;
	size_t GetResultImageArea() const;

protected:
	CCompositingParameters m_Parameters;

	StlImageSize m_SingleImageSize;
	CSNPCompositingResult m_result;

	CDenseMatrix m_MeanImagePosition;
	CDenseMatrix m_InterpolatedPosX;
	CDenseMatrix m_InterpolatedPosY;

protected:
	void ProcessSolutionMatrix(const CDenseMatrix& solutionMatrix, size_t nImages);
	void InterpolateImageRowPositions(CDenseMatrix& posX, CDenseMatrix& posY);
	void SetSingleImageSize(const StlImageSize& szSize);

private:



	void SetMeanImagePosition(const CDenseMatrix& posX, const CDenseMatrix& posY);
	static void SetMatrix(const CDenseMatrix& sourceMatrix, size_t nImages, CDenseMatrix& posX, CDenseMatrix& posY);




	void OnCancel() override;
	void OnUncancel() override;
	CProgress GetProgress() override;
	size_t m_nTotalImages = 1;
	std::atomic_size_t m_nProcessedImages = 0;
};
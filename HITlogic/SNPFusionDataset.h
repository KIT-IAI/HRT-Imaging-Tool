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

//#include "2DNerveFiberMapping.h"
#include "ProgressListener.h"
#include "SNPDatasetParameters.h"
#include "SNPFusionEventHandler.h"
#include "ImageRegistrationResult.h"
#include "SNPGroupResult.h"
#include "SNPDatasetOutputParameters.h"
#include "ImageRegistrationData.h"
#include "HRTImageClassificationData.h"

#include "Compositing3DCloud.h"
#include "CompositingVoxelBuffer.h"


class CSNPFusionDataset : public IProgressListener, public IProgressing, public ICancelable
{
public:

	CSNPFusionDataset() = default;
	CSNPFusionDataset(const CSNPFusionDataset& from);
	CSNPFusionDataset(CSNPFusionDataset&& from);
	CSNPFusionDataset& operator=(const CSNPFusionDataset& src);

	virtual ~CSNPFusionDataset();

	static const std::wstring s_sModuleName;

	bool ProcessDataset();

	void SetParameters(const CSNPDatasetParameters& Parameters);
	void SetImageFileList(const std::vector<std::wstring>& ImageFileList);
	void SetVolImageList(const std::vector<StlImage<float>*>& VolImageFileList);
	void SetImageDepthData(const CHRTImageDepthAndAngleData& ImageDepthData);
	void SetImageClassificationData(const CHRTImageClassificationData& ImageClassificationData);
	void SetOutputDirectory(const std::wstring& sOutputPath);

	void AddEventListener(ISNPFusionEventHandler* eventHandler);
	void RemoveEventListener(ISNPFusionEventHandler* eventHandler);

	size_t GetImageCount() const;

	void OnProgress(CProgress progress) override;
	CProgress GetProgress() override;

	std::wstring Name = L"";

protected:

	void OnCancel() override;
	void OnUncancel() override;

private:

	void PerformHrtWorkflow();
	void StartProcessing();

	void CheckParameters() const;
	void ApplyGlobalParameters() const;

	void TruncateImageFileList();

	std::vector<StlImage<float>*> LoadImages();
	std::vector<StlImage<float>*> PreprocessImagesForRegistration(const std::vector<StlImage<float>*>& InputImages);
	std::vector<StlImage<float>*> PreprocessImages(const std::vector<StlImage<float>*>& InputImages, const CHrtPreProcessingParameters& Parameters);
	CImageRegistrationResult RegisterImages(const CImageRegistrationData& ImageData);
	std::vector<StlImage<float>*> PreprocessImagesForCompositing(const std::vector<StlImage<float>*>& InputImages);
	void GlobalPositioning(CSNPGroupResult& groupResult);
	void Compose(const CImageRegistrationData& GroupImageData, CSNPGroupResult& GroupResult);
	static void FreeImages(std::vector<StlImage<float>*>& Images);

	std::vector<CSNPGroupResult> ProcessRegistrationGroups(const CImageRegistrationResult& AllRegistrationResults, const CImageRegistrationData& ImageData);
	CSNPGroupResult ProcessRegistrationGroup(const CImageRegistrationResult& GroupRegistrationResults, const CImageRegistrationData& GroupImageData, const std::vector<std::wstring>& GroupImagePaths);

	std::vector<std::list<size_t>> CreateDistinctImageGroups() const;
	void OutputResults(const CImageRegistrationResult& AllRegistrationResults, const std::vector<CSNPGroupResult>& GroupResults);

	CSNPDatasetOutputParameters GenerateOutputParameters(size_t nGroupCount) const;

	void RaiseEvent(CSNPFusionEvent snpEvent);
	void RaiseEvent(CSNPFusionDataset* parpSource = nullptr, CSNPFusionEvent::EEventType parSeverity = CSNPFusionEvent::eInformation, CSNPFusionEvent::EEventSubType subType = CSNPFusionEvent::eNone, std::wstring parsMessage = L"");

	ICancelable* m_pCurrentCancelable = nullptr;
	CProgress m_CurrentProgress;

	CSNPDatasetParameters m_Parameters;
	CHrtImageParameters m_ImageParameters;

	std::vector<std::wstring> m_ImageFiles;
	std::vector<StlImage<float>*> m_VolImages;
	CHRTImageDepthAndAngleData m_ImageInfo;
	CHRTImageClassificationData m_ImageClasses;

	std::wstring sOutputPath;

	std::list<ISNPFusionEventHandler*> m_EventHandler;
};

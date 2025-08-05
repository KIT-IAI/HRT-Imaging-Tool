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

#include "SNPFusionDataset.h"

#include "SNPFusionException.h"
#include <TimeMeasurement.h>
#include "ImageRegistrator.h"
#include "ImageLoader.h"
#include "HrtPreProcessingPipeline.h"
#include "HRTGlobalPositioning.h"
#include "SNPDatasetResultLoader.h"
#include "CompositingFactory.h"
#include "CompositingFusion.h"
#include "RegStepScoreThresholdAdapter.h"

using namespace std;


/**	\brief Die Bezeichnung des Moduls.
*
*	Die Bezeichnung wird jedem Eintrag in die Protokolldatei vorangestellt.
*
*	\see CLog::Log()
*/
const wstring CSNPFusionDataset::s_sModuleName = _T("SNP-FusionDataset Batch");


void CSNPFusionDataset::SetParameters(const CSNPDatasetParameters& Parameters)
{
	m_Parameters = Parameters;
}

void CSNPFusionDataset::SetImageFileList(const vector<wstring>& ImageFileList)
{
	m_ImageFiles = ImageFileList;
}

void CSNPFusionDataset::SetVolImageList(const vector<StlImage<float>*>& VolImageFileList)
{
	for (auto img : m_VolImages)
	{
		delete img;
	}
	m_VolImages = VolImageFileList;
}

void CSNPFusionDataset::SetImageDepthData(const CHRTImageDepthAndAngleData& ImageDepthData)
{
	m_ImageInfo = ImageDepthData;
}

void CSNPFusionDataset::SetImageClassificationData(const CHRTImageClassificationData& ImageClassificationData)
{
	m_ImageClasses = ImageClassificationData;
}

void CSNPFusionDataset::SetOutputDirectory(const wstring& OutputPath)
{
	this->sOutputPath = OutputPath;
}

size_t CSNPFusionDataset::GetImageCount() const
{
	return m_ImageFiles.size();
}

void CSNPFusionDataset::OnProgress(CProgress progress)
{
	m_CurrentProgress = progress;
	ReportProgress();
}

CProgress CSNPFusionDataset::GetProgress()
{
	return m_CurrentProgress;
}

void CSNPFusionDataset::AddEventListener(ISNPFusionEventHandler* eventHandler)
{
	m_EventHandler.push_back(eventHandler);
}

void CSNPFusionDataset::RemoveEventListener(ISNPFusionEventHandler* eventHandler)
{
	auto it = std::find(m_EventHandler.begin(), m_EventHandler.end(), eventHandler);

	if (it != m_EventHandler.end())
		m_EventHandler.erase(it);
}

void CSNPFusionDataset::RaiseEvent(CSNPFusionEvent Event)
{
	for (auto& listener : m_EventHandler)
		listener->OnSNPFusionEvent(Event);
}

void CSNPFusionDataset::RaiseEvent(CSNPFusionDataset* parpSource, CSNPFusionEvent::EEventType parSeverity, CSNPFusionEvent::EEventSubType subType, wstring parsMessage)
{
	RaiseEvent(CSNPFusionEvent(parpSource, parSeverity, subType, parsMessage));
}

void CSNPFusionDataset::OnCancel()
{
	if (m_pCurrentCancelable != nullptr)
		m_pCurrentCancelable->Cancel();
}

void CSNPFusionDataset::OnUncancel()
{
	if (m_pCurrentCancelable != nullptr)
		m_pCurrentCancelable->Uncancel();
}
CSNPFusionDataset::CSNPFusionDataset(const CSNPFusionDataset& from)
{
	Name = from.Name;
	m_pCurrentCancelable = from.m_pCurrentCancelable;
	m_CurrentProgress = from.m_CurrentProgress;

	m_Parameters = from.m_Parameters;
	m_ImageParameters = from.m_ImageParameters;

	m_ImageFiles = from.m_ImageFiles;
	for (auto img : from.m_VolImages)
	{
		m_VolImages.push_back(new StlImage<float>());
		(*m_VolImages.back()) = (*img);
	}
	m_ImageInfo = from.m_ImageInfo;
	m_ImageClasses = from.m_ImageClasses;

	sOutputPath = from.sOutputPath;

	m_EventHandler = from.m_EventHandler;
}
CSNPFusionDataset::CSNPFusionDataset(CSNPFusionDataset&& from)
{
	std::swap(Name, from.Name);
	std::swap(m_pCurrentCancelable, from.m_pCurrentCancelable);
	std::swap(m_CurrentProgress, from.m_CurrentProgress);

	std::swap(m_Parameters, from.m_Parameters);
	std::swap(m_ImageParameters, from.m_ImageParameters);

	std::swap(m_ImageFiles, from.m_ImageFiles);
	std::swap(m_VolImages, from.m_VolImages);

	std::swap(m_ImageInfo, from.m_ImageInfo);
	std::swap(m_ImageClasses, from.m_ImageClasses);

	std::swap(sOutputPath, from.sOutputPath);

	std::swap(m_EventHandler, from.m_EventHandler);
}
CSNPFusionDataset& CSNPFusionDataset::operator=(const CSNPFusionDataset& src)
{
	if (this != &src)
	{
		Name = src.Name;
		m_pCurrentCancelable = src.m_pCurrentCancelable;
		m_CurrentProgress = src.m_CurrentProgress;

		m_Parameters = src.m_Parameters;
		m_ImageParameters = src.m_ImageParameters;

		m_ImageFiles = src.m_ImageFiles;
		for (auto img : src.m_VolImages)
		{
			m_VolImages.push_back(new StlImage<float>());
			(*m_VolImages.back()) = (*img);
		}
		m_ImageInfo = src.m_ImageInfo;
		m_ImageClasses = src.m_ImageClasses;

		sOutputPath = src.sOutputPath;

		m_EventHandler = src.m_EventHandler;

	}
	return *this;
}
CSNPFusionDataset::~CSNPFusionDataset()
{
	for (auto img : m_VolImages)
	{
		delete img;
	}
}
bool CSNPFusionDataset::ProcessDataset()
{
	CTimeMeasurement stopWatch;
	stopWatch.Start();

	TruncateImageFileList();

	StartProcessing();

	// CARFUL BLOCKBASED REGISTRATION DOES NOT WORK PROPERLY ATM
	ASSERT(m_Parameters.eProcessType != CProcessType::eBlockBasedRegistration);
	bool bSuccess = true;

	try
	{
		CheckParameters();
		ApplyGlobalParameters();
		PerformHrtWorkflow();
	}
	catch (std::exception& ex)
	{
		RaiseEvent(this, CSNPFusionEvent::eError, CSNPFusionEvent::eNone, CStringUtilities::ConvertToStdWstring(string_view(ex.what())));
		bSuccess = false;
	}

	stopWatch.Stop();

	RaiseEvent(this, CSNPFusionEvent::eInformation, CSNPFusionEvent::eCalculationTime, L"Berechnungsdauer: " + wstring(stopWatch.GetTimeStr()));

	if (m_bIsCanceled)
	{
		RaiseEvent(this, CSNPFusionEvent::eDatasetDone, CSNPFusionEvent::eProcessingCanceled, L"Berechnung durch Nutzer abgebrochen.");
		bSuccess = false;
	}
	else
		RaiseEvent(this, CSNPFusionEvent::eDatasetDone, CSNPFusionEvent::eProcessingSuccessful, L"Berechnung abgeschlossen.");

	return bSuccess;
}

void CSNPFusionDataset::PerformHrtWorkflow()
{
	if (m_ImageFiles.empty())
		return;

	if (m_ImageInfo.isAvailable() && (m_ImageFiles.size() != m_ImageInfo.getCount()))
	{
		RaiseEvent(this, CSNPFusionEvent::eWarning, CSNPFusionEvent::eLoadingImages, L"Fokusinformationen nicht kompatibel mit dem Datensatz; sie werden ignoriert.");
	}

	if (m_ImageClasses.IsLoaded() && (m_ImageFiles.size() != m_ImageClasses.GetSize()))
	{
		RaiseEvent(this, CSNPFusionEvent::eWarning, CSNPFusionEvent::eLoadingImages, L"Klassifikationsdaten nicht kompatibel mit dem Datensatz; sie werden ignoriert.");
	}

	auto UnProcessedImages = LoadImages();
	auto PreprocessedImages = PreprocessImagesForRegistration(UnProcessedImages);

	CImageRegistrationData registrationImageData;
	registrationImageData.Images = PreprocessedImages;
	registrationImageData.ImageInfo = m_ImageInfo;
	registrationImageData.ImageClasses = m_ImageClasses;
	auto AllRegistrationResults = RegisterImages(registrationImageData);

	FreeImages(PreprocessedImages);

	CImageRegistrationData compositingImageData;
	compositingImageData.Images = UnProcessedImages;
	compositingImageData.ImageInfo = m_ImageInfo;
	compositingImageData.ImageClasses = m_ImageClasses;
	auto GroupResults = ProcessRegistrationGroups(AllRegistrationResults, compositingImageData);

	FreeImages(UnProcessedImages);

	OutputResults(AllRegistrationResults, GroupResults);
}

void CSNPFusionDataset::CheckParameters() const
{
	if (!m_Parameters.IsValidParameterset())
		throw CSNPFusionException(L"Invalid parameterset.");
}

void CSNPFusionDataset::ApplyGlobalParameters() const
{
	CScoreThresholdAdapter::EnableDetailedLogging(m_Parameters.bDetailedLogging);
}

CSNPGroupResult CSNPFusionDataset::ProcessRegistrationGroup(const CImageRegistrationResult& RegistrationResult, const CImageRegistrationData& GroupImageData, const vector<wstring>& GroupImagePaths)
{
	CSNPGroupResult GroupResult;
	GroupResult.RegistrationResult = RegistrationResult;
	GroupResult.UsedImagePaths = GroupImagePaths;
	GroupResult.Parameters = m_Parameters;

	GlobalPositioning(GroupResult);
	Compose(GroupImageData, GroupResult);

	return GroupResult;
}

vector<std::list<size_t>> CSNPFusionDataset::CreateDistinctImageGroups() const
{
	vector<std::list<size_t>> Groups;

	for (size_t i = 0; i < m_ImageFiles.size(); i++)
		Groups.push_back({ i });

	return Groups;
}

vector<CSNPGroupResult> CSNPFusionDataset::ProcessRegistrationGroups(const CImageRegistrationResult& AllRegistrationResults, const CImageRegistrationData& ImageData)
{
	if (m_bIsCanceled)
		return{};

	vector<CSNPGroupResult> Results;

	auto PreprocessedImages = PreprocessImagesForCompositing(ImageData.Images);

	size_t nGroupIndex = 0;

	for (const auto& ResultGroup : AllRegistrationResults.GetResultsByGroup())
	{
		RaiseEvent(this, CSNPFusionEvent::eProgress, CSNPFusionEvent::eProcessingImageGroup, L"Bildgruppe " + std::to_wstring(nGroupIndex + 1) + L" wird bearbeitet.");

		auto GroupIndices = AllRegistrationResults.ImageGroups[nGroupIndex];
		//auto GroupUnprocessedImages = CArrayUtilities::FilterByIndex(PreprocessedImages, GroupIndices);
		auto GroupImagePaths = CArrayUtilities::FilterByIndex(m_ImageFiles, GroupIndices);
		//auto GroupDepths = m_ImageInfo.filter(GroupIndices);

		CImageRegistrationData GroupImageData;
		GroupImageData.Images = CArrayUtilities::FilterByIndex(PreprocessedImages, GroupIndices);
		GroupImageData.ImageInfo = m_ImageInfo.filter(GroupIndices);
		GroupImageData.ImageClasses = m_ImageClasses.Filter(GroupIndices);
		auto GroupResult = ProcessRegistrationGroup(ResultGroup, GroupImageData, GroupImagePaths);

		Results.push_back(std::move(GroupResult));
		nGroupIndex++;
	}

	FreeImages(PreprocessedImages);

	return Results;
}

void CSNPFusionDataset::FreeImages(vector<StlImage<float>*>& Images)
{
	for (auto pImage : Images)
		delete pImage;
	Images.clear();
}

void CSNPFusionDataset::StartProcessing()
{
	m_bIsCanceled = false;
	wstring sLogLine = L"Beginn der Berechnung (" + std::to_wstring(m_ImageFiles.size()) + L" Bilder)";
	RaiseEvent(this, CSNPFusionEvent::eInformation, CSNPFusionEvent::eProcessingStart, sLogLine);
}

void CSNPFusionDataset::TruncateImageFileList()
{
	if ((m_Parameters.nMaxSequenceLength > 0) && (m_ImageFiles.size() > m_Parameters.nMaxSequenceLength))
	{
		m_ImageFiles.resize(m_Parameters.nMaxSequenceLength);
		if (m_VolImages.size() > m_Parameters.nMaxSequenceLength)
		{
			for (size_t i = m_Parameters.nMaxSequenceLength; i < m_VolImages.size(); i++)
			{
				delete m_VolImages[i];
			}
			m_VolImages.resize(m_Parameters.nMaxSequenceLength);
		}
	}
}

vector<StlImage<float>*> CSNPFusionDataset::LoadImages()
{
	if (m_bIsCanceled)
		return{};


	if (m_VolImages.size() != m_ImageFiles.size())
	{
		CImageLoader Loader;

		Loader.AddProgressListener(this);
		m_pCurrentCancelable = &Loader;
		RaiseEvent(this, CSNPFusionEvent::eProgress, CSNPFusionEvent::eLoadingImages, L"Bilder werden geladen.");

		auto result = Loader.LoadImages(m_ImageFiles);;

		m_pCurrentCancelable = nullptr;
		return result;
	}
	else
	{
		RaiseEvent(this, CSNPFusionEvent::eProgress, CSNPFusionEvent::eLoadingImages, L"Bilder werden kopiert.");

		vector<StlImage<float>*> result;
		for (auto img : m_VolImages)
		{
			result.push_back(new StlImage<float>());
			(*result.back()) = (*img);
		}
		return result;
	}
}

vector<StlImage<float>*> CSNPFusionDataset::PreprocessImagesForRegistration(const vector<StlImage<float>*>& InputImages)
{
	auto Result = PreprocessImages(InputImages, m_Parameters.GeneratePreProcessingParameters());

	/******************************   NOTE   ******************************/
	/*                                                                    */
	/* For some time in the past, the images were always smoothed before  */
	/* calculating the phase correlation function. This has been removed  */
	/* since, but it is worth considering to put it back in, particularly */
	/* if no shift correction is performed (which also has a smoothing    */
	/* effect through interpolation). The following lines would           */
	/* reimplement the old behaviour.                                     */
	/*                                                                    */
	/**********************************************************************/
	//for (auto pImg : Result)
	//{
	//	pImg->Convolve(*pImg, M_SMOOTH);
	//}

	ASSERT(Result.size() > 0);

	m_ImageParameters = m_Parameters.GenerateImageParameters(Result[0]->GetSize());

	return Result;
}

vector<StlImage<float>*> CSNPFusionDataset::PreprocessImages(const vector<StlImage<float>*>& InputImages, const CHrtPreProcessingParameters& Parameters)
{
	if (m_bIsCanceled)
		return{};

	CHrtPreProcessingPipeline Preprocessor(Parameters);

	Preprocessor.AddProgressListener(this);
	m_pCurrentCancelable = &Preprocessor;
	RaiseEvent(this, CSNPFusionEvent::eProgress, CSNPFusionEvent::ePreprocessingImages, L"Bilder werden vorverarbeitet.");

	auto result = Preprocessor.ProcessImages(InputImages);

	m_pCurrentCancelable = nullptr;
	return result;
}

CImageRegistrationResult CSNPFusionDataset::RegisterImages(const CImageRegistrationData& ImageData)
{
	if (m_bIsCanceled)
		return CImageRegistrationResult();

	CImageRegistrator Registrator(m_Parameters.GenerateRegistrationProcedureParameters(m_ImageParameters.ImageSize));

	Registrator.AddProgressListener(this);
	m_pCurrentCancelable = &Registrator;
	RaiseEvent(this, CSNPFusionEvent::eProgress, CSNPFusionEvent::eRegisteringImages, L"Bilder werden registriert.");

	auto result = Registrator.RegisterImages(ImageData);

	m_pCurrentCancelable = nullptr;
	return result;
}

vector<StlImage<float>*> CSNPFusionDataset::PreprocessImagesForCompositing(const vector<StlImage<float>*>& InputImages)
{
	return PreprocessImages(InputImages, m_Parameters.GenerateCompositingPreProcessingParameters());
}

void CSNPFusionDataset::GlobalPositioning(CSNPGroupResult& groupResult)
{
	if (m_bIsCanceled)
		return;

	CHRTGlobalPositioning GlobalPositioning(m_Parameters.GenerateGlobalPositioningParameters());

	m_pCurrentCancelable = &GlobalPositioning;
	RaiseEvent(this, CSNPFusionEvent::eProgress, CSNPFusionEvent::ePositioningImages, L"Bilder werden positioniert.");

	groupResult.PositioningSolution = GlobalPositioning.SolvePositioning(groupResult.RegistrationResult, m_ImageParameters);

	m_pCurrentCancelable = nullptr;
}

void CSNPFusionDataset::Compose(const CImageRegistrationData& GroupImageData, CSNPGroupResult& GroupResult)
{
	if (m_bIsCanceled)
		return;

	auto Compositer = CCompositingFactory::CreateCompositing(m_Parameters.GenerateCompositingParameters());
	wstring modeName = L"Mosaikbild";

	//je nach Modus andere Ausgabe
	if (m_Parameters.eCompositingMode == CCompositingParameters::ECompositingType::e3DCloud) {
		modeName = L"3D-Punktwolke";
	}
	else if (m_Parameters.eCompositingMode == CCompositingParameters::ECompositingType::eVoxelBuffer) {
		modeName = L"VoxelBuffer";
	}

	m_pCurrentCancelable = Compositer.get();
	RaiseEvent(this, CSNPFusionEvent::eProgress, CSNPFusionEvent::eCompositing, modeName + L" wird erstellt.");

	GroupResult.pResult = Compositer->Fuse(GroupImageData, GroupResult.PositioningSolution);

	// get motion-corrected images (currently only works for the three
	// compositors below; in other cases they are not returned)
	if (m_Parameters.eCompositingMode == CCompositingParameters::ECompositingType::eFusion)
		GroupResult.MotionCorrectedImages = std::dynamic_pointer_cast<CCompositingFusion>(Compositer)->GetUndistortedImages();
	else if (m_Parameters.eCompositingMode == CCompositingParameters::ECompositingType::e3DCloud)
		GroupResult.MotionCorrectedImages = std::dynamic_pointer_cast<CCompositing3DCloud>(Compositer)->GetUndistortedImages();
	else if (m_Parameters.eCompositingMode == CCompositingParameters::ECompositingType::eVoxelBuffer)
		GroupResult.MotionCorrectedImages = std::dynamic_pointer_cast<CCompositingVoxelBuffer>(Compositer)->GetUndistortedImages();

	m_pCurrentCancelable = nullptr;
}

void CSNPFusionDataset::OutputResults(const CImageRegistrationResult& AllRegistrationResults, const vector<CSNPGroupResult>& GroupResults)
{
	if (m_bIsCanceled)
		return;
	RaiseEvent(this, CSNPFusionEvent::eProgress, CSNPFusionEvent::eOutputingResults, L"Finale Ausgabedateien werden erstellt.");

	auto ImageGroups = AllRegistrationResults.ImageGroups;

	if (ImageGroups.empty())
		ImageGroups = CreateDistinctImageGroups();

	auto OutputParameters = GenerateOutputParameters(ImageGroups.size());

	vector<CDenseMatrix> AllSolutions;
	vector<CSNPCompositingResult*> AllResults;
	for (const auto& Group : GroupResults)
	{
		AllSolutions.push_back(Group.PositioningSolution);
		AllResults.push_back(Group.pResult.get());
	}

	auto ResultLoader = CSNPDatasetResultLoader::FromImagePath(m_ImageFiles[0]);

	if (!sOutputPath.empty())
		ResultLoader = CSNPDatasetResultLoader::FromOutputFolder(sOutputPath);

	ResultLoader.CreateFolderStructure(ImageGroups.size());
	ResultLoader.ExportRegistrations(AllRegistrationResults);
	ResultLoader.ExportImageList(m_ImageFiles, ImageGroups);
	ResultLoader.ExportInputParameters(m_Parameters);
	ResultLoader.ExportOutputParameters(OutputParameters);
	ResultLoader.ExportPositioningSolution(AllSolutions);
	ResultLoader.ExportResults(AllResults);
	if (m_Parameters.bForceExportMotionCorrectedImages) ResultLoader.ExportMotionCorrectedImages(GroupResults);
}

CSNPDatasetOutputParameters CSNPFusionDataset::GenerateOutputParameters(size_t nGroupCount) const
{
	auto SeriesOutputParameters = CSNPDatasetOutputParameters::ConstructWithCurrentUserdata();
	SeriesOutputParameters.fDetectedThreshold = *m_Parameters.fScoreThreshold;
	SeriesOutputParameters.ImageParameters = m_ImageParameters;
	SeriesOutputParameters.nDetectedImageGroups = nGroupCount;
	SeriesOutputParameters.nImageCount = m_ImageFiles.size();
	return SeriesOutputParameters;
}

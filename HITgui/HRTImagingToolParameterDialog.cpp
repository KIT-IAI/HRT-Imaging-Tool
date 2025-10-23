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



#include "pch.h"
#include "HRTImagingToolIncludes.h"

const std::vector<std::pair<wxString, wxString>> HRTImagingToolParameterDialog::s_parameterHelpStrings = {
	{L"nMaxSequenceLength (default: 0)",
		L"Process only the first n images of each dataset and ignore all later "
		L"images. A value of 0 means that no such limit is imposed (i.e. all "
		L"images of each dataset are processed)."},
	{L"nVignettingCreationMaxImages (default: 0)",
		L"Use only the first n images of each dataset when creating a "
		L"dataset-specific vignetting correction profile. A value of 0 means that "
		L"no such limit is imposed (i.e. all images of each dataset are used to "
		L"create dataset-specific vignetting correction profiles.). This parameter "
		L"has no effect if global vignetting correction profiles are used or if "
		L"vignetting correction is not activated."},
	{L"bShiftCorrection (default: off)",
		L"Correct shift artifacts between even and odd image rows before pocessing "
		L"a dataset. An individual shift value is calculated for each dataset. "
		L"Shift correction is not recommended for datasets consisting of less than "
		L"100 images."},
	{L"bBrightnessCorrectionBeforeRegistration (default: off)",
		L"Adjust global image brightness differences within a dataset before "
		L"performing image registration. This parameter only affects the image "
		L"registration process, it has no effect on the result image composition."},
	{L"bVignettingCorrectionBeforeRegistration (default: off)",
		L"Correct vignetting (illumination falloff) effects in the images before "
		L"performing image registration. The same (global or dataset-specific) "
		L"profile is used for all images in a dataset. This parameter only affects "
		L"the image registration process, it has no effect on the result image "
		L"composition."},
	{L"sVignettingFileBeforeRegistration (default: ' ')",
		L"Full file path of the global vignetting correction profile for the image "
		L"registration process. The dataset processing will result in an error if "
		L"the file can not be accessed. This parameter has no effect if vignetting "
		L"correction is not activated."},
	{L"nVignettingSmoothingBeforeRegistration (default: 0)",
		L"Perform a smoothing operation on the vignetting correction profile for "
		L"the image registration process n times before usage. This parameter has "
		L"no effect if vignetting correction is not activated."},
	{L"eStrategie (default: 8)",
		L"The image pair selection strategy used for the image registration "
		L"process. Extensive selection strategies lead to longer computation "
		L"times, but are more likely to yield a better result quality than more "
		L"restrictive selection strategies. Choose 'complete' to guarantee best "
		L"result quality."},
	{L"fSelectionHeight (default: 1.0)",
		L"Threshold value for the focus depth difference. Only register image "
		L"pairs with a focus depth difference not greater than this value. This "
		L"parameter only affects the 'plane-based' image pair selection strategy."},
	{L"nMaxImageDistance (default: 400)",
		L"Threshold value for the image index difference. Only register image "
		L"pairs with an image index difference not greater than this value. This "
		L"parameter only affects the 'forward/backward', 'fixed stepsize', "
		L"'sector-based' and 'position-based' image pair selection strategies."},
	{L"nMaxRegDistance (default: 8)",
		L"Threshold value for the image node distance in the registration graph. "
		L"Only register image pairs with an image node distance in the current "
		L"registration graph greater than this value. This parameter only affects "
		L"the 'position-based' image pair selection strategy."},
	{L"nWindowSize (default: 3)",
		L"Threshold value for the image index difference. Register all image pairs "
		L"with an image index difference not greater than this value. This "
		L"parameter only affects the 'window', 'fixed stepsize', 'sector-based' "
		L"and 'plane-based' image pair selection strategies."},
	{L"nStepSize (default: 3)",
		L"Fixed image index step size. Register image pairs with an image index "
		L"difference equal to a multiple of this value. This parameter only "
		L"affects the 'fixed stepsize' image pair selection strategy."},
	{L"nSectorSize (default: 30)",
		L"Fixed image interval size. For a given image, only register one other "
		L"image out of each image interval (sector) of the given size. This "
		L"parameter only affects the 'sector-based' image pair selection strategy."},
	{L"eImagePairVerificator (default: 0)",
		L"Apply an additional validation filter to the image pairs returned by the "
		L"image pair selection strategy before performing the image registration "
		L"process."},
	{L"eProcessType (default: 4)",
		L"The image registration approach. Determines whether to perform only a "
		L"fast rigid image registration or the more elaborate motion artifact "
		L"correction procedure. The latter requires a further discrimination "
		L"between continuous datasets and datasets containing independent images."},
	{L"fMinScore (default: 4.5)",
		L"Correlation value threshold for rigid image registration. Registration "
		L"results with a lower correlation value are considered to be incorrect. "
		L"Registration results with a higher correlation value may be considered "
		L"to be correct after further validation."},
	{L"fMinScoreFlexible (default: 9.0)",
		L"Correlation value threshold for sub-image registration. Registration "
		L"results with a lower correlation value are considered to be incorrect. "
		L"Registration results with a higher correlation value may be considered "
		L"to be correct after further validation."},
	{L"fCertainScore (default: 20.0)",
		L"Correlation value threshold for rigid image registration. Registration "
		L"results with a higher correlation value are considered to be correct "
		L"without further validation. Registration results with a lower "
		L"correlation value undergo further validation."},
	{L"fScalation (default: 1.0)",
		L"Scaling factor for rigid image registration. A value greater than 1.0 "
		L"means that the images are down-scaled by the respective factor. A value "
		L"of 1.0 means that no down-scaling of the images happens (i.e. the "
		L"original images are used)."},
	{L"fScalationSubImages (default: 1.0)",
		L"Scaling factor for sub-image registration. A value greater than 1.0 "
		L"means that the images are down-scaled by the respective factor. A value "
		L"of 1.0 means that no down-scaling of the images happens (i.e. the "
		L"original images are used)."},
	{L"nSubImageHeight (default: 32)",
		L"The height (i.e. the number of image rows) of the sub-images used for "
		L"sub-image registration. The number should be a divisor of the original "
		L"image height. Using any value other than 32 is not recommended."},
	{L"bConsistencyCheck (default: off)",
		L"Iteratively exclude inconsistent equations from the system of linear "
		L"equations defined by the registration results. This requires the system "
		L"of linear equations to be sufficiently redundant; it will do nothing if "
		L"used with the 'simple forward' or 'forward/backward' image pair "
		L"selection strategies. It only works well for a relatively small number "
		L"of inconsistent equations."},
	{L"bAutomaticThresholdDetection (default: off)",
		L"Determine the correlation value threshold (for rigid image registration "
		L"or sub-image registration, depending on the selected image registration "
		L"approach) automatically. In case of rigid image registration, it is "
		L"recommended to also activate consistency checking. Deactivate this "
		L"option and find appropriate fixed threshold values if the resulting "
		L"images become very fragmented."},
	{L"fResidualThreshold (default: 10.0)",
		L"Largest allowed residual error value during automatic correlation value "
		L"threshold detection. The correlation value threshold is increased until "
		L"all residual errors are smaller than this value.This parameter has no "
		L"effect if automatic correlation value threhold detection is not "
		L"activated or if rigid image registration is used."},
	{L"eSolver (default: 6)",
		L"Algorithm for solving the system of linear equations defined by the "
		L"registration results. It is strongly recommended to select one of the "
		L"options with the '(alglib)' extension."},
	{L"bBrightnessCorrectionBeforeCompositing (default: off)",
		L"Adjust global image brightness differences within a dataset before "
		L"performing result image composition. This parameter only affects the "
		L"result image composition, it has no effect on the image registration "
		L"process."},
	{L"bVignettingCorrectionBeforeCompositing (default: off)",
		L"Correct vignetting (illumination falloff) effects in the images before "
		L"performing result image composition. The same (global or "
		L"dataset-specific) profile is used for all images in a dataset. This "
		L"parameter only affects the result image composition, it has no effect on "
		L"the image registration process."},
	{L"sVignettingFileBeforeCompositing (default: ' ')",
		L"Full file path of the global vignetting correction profile for the "
		L"result image composition. The dataset processing will result in an error "
		L"if the file can not be accessed. This parameter has no effect if "
		L"vignetting correction is not activated."},
	{L"nVignettingSmoothingBeforeCompositing (default: 0)",
		L"Perform a smoothing operation on the vignetting correction profile for "
		L"the result image composition n times before usage. This parameter has no "
		L"effect if vignetting correction is not activated."},
	{L"eCompositingMode (default: 0)",
		L"Determines the output image dimensionality (2D or 3D) and representation "
		L"(voxel or point cloud)."},
	{L"eWeightMode (default: 1)",
		L"The weighting function used to weight image pixel values for the "
		L"composition process. Using any setting other than '2D cosine' is not "
		L"recommended."},
	{L"nBorder (default: 10)",
		L"Frame the result image with a border of n pixels/voxels."},
	{L"cBackgroundColor (default: 0)",
		L"The intensity value used for result pixels/voxels that contain no "
		L"information. This value is also used for the border around the result "
		L"image."},
	{L"f3dIntensityLimit (default: 0.0)",
		L"Ignore image pixels with an intensity value less than this value for the "
		L"point cloud. This parameter has no effect for result image composition "
		L"methods other than '3D point cloud'."},
	{L"f3dZMultiplier (default: 2.0)",
		L"Stretch factor for the depth coordinate. A value greater than 1.0 means "
		L"that the resulting volume is expanded along the z-coordinate by the "
		L"respective factor, and vice versa. A value of 1.0 means that the volume "
		L"remains unchanged (i.e. the original focus values are used)."},
	{L"bExcludeNonSNPImages (default: off)",
		L"Exclude all non-SNP images from the result composition. This requires "
		L"classification data to be present to work correctly. This parameter only "
		L"affects the 2D fusion composition method."},
	{L"bForceExportMotionCorrectedImages (default: off)",
		L"Force export of motion-corrected images to image files. This only works "
		L"with the 2D fusion and the 3D point cloud composition methods. The 3D "
		L"voxel volume composition exports the volume itself as a stack of "
		L"motion-corrected images. For all other composition methods, this "
		L"parameter has no effect."},
	{ L"bDetailedLogging (default: off)",
		L"Enable detailed process logging. This should be used for debugging only, "
		L"as it will potentially affect execution time."}
};

wxBEGIN_EVENT_TABLE(HRTImagingToolParameterDialog, wxHITParameterDialog_Base)
EVT_INIT_DIALOG(HRTImagingToolParameterDialog::OnInitDialog)
EVT_BUTTON(XRCID("m_buttonPreset2D"), HRTImagingToolParameterDialog::OnButtonPreset2D)
EVT_BUTTON(XRCID("m_buttonPresetSNP"), HRTImagingToolParameterDialog::OnButtonPresetSNP)
EVT_BUTTON(XRCID("m_buttonPreset3DVoxel"), HRTImagingToolParameterDialog::OnButtonPreset3DVoxel)
EVT_BUTTON(XRCID("wxID_OK"), HRTImagingToolParameterDialog::OnOK)
EVT_BUTTON(XRCID("wxID_CANCEL"), HRTImagingToolParameterDialog::OnCancel)
EVT_CLOSE(HRTImagingToolParameterDialog::OnClose)
wxEND_EVENT_TABLE()

HRTImagingToolParameterDialog::HRTImagingToolParameterDialog()
{
}

HRTImagingToolParameterDialog::~HRTImagingToolParameterDialog()
{
}

void HRTImagingToolParameterDialog::OnInitDialog(wxInitDialogEvent& event)
{
	BindChildCtrlEvents();

	ImportParameters();
}

void HRTImagingToolParameterDialog::OnButtonPreset2D(wxCommandEvent& event)
{
	CSNPDatasetParameters presetParameters = CreateDefaults2D();
	CSNPDatasetParameters currentParameters;

	SyncDlgToPar(currentParameters);

	if (currentParameters != presetParameters)
	{
		wxString sMsg = L"Are you sure you want to change the current settings with a default parameter set? (You can still quit without saving later.)";
		if (wxMessageBox(sMsg, L"Change whole parameter set?", wxYES_NO | wxCANCEL) == wxYES)
		{
			SyncParToDlg(presetParameters);
		}
	}
}

void HRTImagingToolParameterDialog::OnButtonPresetSNP(wxCommandEvent& event)
{
	CSNPDatasetParameters presetParameters = CreateDefaultsSNP();
	CSNPDatasetParameters currentParameters;

	SyncDlgToPar(currentParameters);

	if (currentParameters != presetParameters)
	{
		wxString sMsg = L"Are you sure you want to change the current settings with a default parameter set? (You can still quit without saving later.)";
		if (wxMessageBox(sMsg, L"Change whole parameter set?", wxYES_NO | wxCANCEL) == wxYES)
		{
			SyncParToDlg(presetParameters);
		}
	}
}

void HRTImagingToolParameterDialog::OnButtonPreset3DVoxel(wxCommandEvent& event)
{
	CSNPDatasetParameters presetParameters = CreateDefaults3DVoxel();
	CSNPDatasetParameters currentParameters;

	SyncDlgToPar(currentParameters);

	if (currentParameters != presetParameters)
	{
		wxString sMsg = L"Are you sure you want to change the current settings with a default parameter set? (You can still quit without saving later.)";
		if (wxMessageBox(sMsg, L"Change whole parameter set?", wxYES_NO | wxCANCEL) == wxYES)
		{
			SyncParToDlg(presetParameters);
		}
	}
}

void HRTImagingToolParameterDialog::OnOK(wxCommandEvent& event)
{
	ExportParameters();

	SyncDlgToPar(m_initialParameters);

	Close();
}

void HRTImagingToolParameterDialog::OnCancel(wxCommandEvent& event)
{
	Close();
}

void HRTImagingToolParameterDialog::OnClose(wxCloseEvent& event)
{
	CSNPDatasetParameters parameters;

	SyncDlgToPar(parameters);

	if (parameters != m_initialParameters)
	{
		if (wxMessageBox(L"Some parameters have been changed. Do you really want to quit without saving the changes?", L"Quit without saving?", wxYES_NO | wxCANCEL) != wxYES)
			event.Veto();
		else
			Destroy();
	}
	else
	{
		Destroy();
	}
}

void HRTImagingToolParameterDialog::BindChildCtrlEvents() const
{
	auto BindEvents = [this](wxStaticText* label, wxControl* control, size_t helpIndex) -> void {
		auto ChangeHelpText = [this](size_t index) -> void {
			ASSERT(index < s_parameterHelpStrings.size());
			m_staticTextParameterName->SetLabel(s_parameterHelpStrings[index].first);
			m_textCtrlParameterHelp->SetValue(s_parameterHelpStrings[index].second);
			};

		label->Bind(wxEVT_LEFT_UP, [control](wxMouseEvent& event) -> void { ASSERT(control->CanAcceptFocus()); control->SetFocus(); event.Skip(); });
		control->Bind(wxEVT_CHILD_FOCUS, [ChangeHelpText, helpIndex](wxChildFocusEvent& event) -> void { ChangeHelpText(helpIndex); event.Skip(); });
		};

	BindEvents(m_staticTextPar01, m_spinCtrlPar01, 0);
	BindEvents(m_staticTextPar02, m_spinCtrlPar02, 1);
	BindEvents(m_staticTextPar03, m_checkBoxPar03, 2);
	BindEvents(m_staticTextPar04, m_checkBoxPar04, 3);
	BindEvents(m_staticTextPar05, m_checkBoxPar05, 4);
	BindEvents(m_staticTextPar06, m_filePickerPar06, 5);
	BindEvents(m_staticTextPar07, m_spinCtrlPar07, 6);
	BindEvents(m_staticTextPar08, m_choicePar08, 7);
	BindEvents(m_staticTextPar09, m_spinCtrlDoublePar09, 8);
	BindEvents(m_staticTextPar10, m_spinCtrlPar10, 9);
	BindEvents(m_staticTextPar11, m_spinCtrlPar11, 10);
	BindEvents(m_staticTextPar12, m_spinCtrlPar12, 11);
	BindEvents(m_staticTextPar13, m_spinCtrlPar13, 12);
	BindEvents(m_staticTextPar14, m_spinCtrlPar14, 13);
	BindEvents(m_staticTextPar15, m_choicePar15, 14);
	BindEvents(m_staticTextPar16, m_choicePar16, 15);
	BindEvents(m_staticTextPar17, m_spinCtrlDoublePar17, 16);
	BindEvents(m_staticTextPar18, m_spinCtrlDoublePar18, 17);
	BindEvents(m_staticTextPar19, m_spinCtrlDoublePar19, 18);
	BindEvents(m_staticTextPar20, m_spinCtrlDoublePar20, 19);
	BindEvents(m_staticTextPar21, m_spinCtrlDoublePar21, 20);
	BindEvents(m_staticTextPar22, m_spinCtrlPar22, 21);
	BindEvents(m_staticTextPar23, m_checkBoxPar23, 22);
	BindEvents(m_staticTextPar24, m_checkBoxPar24, 23);
	BindEvents(m_staticTextPar39, m_spinCtrlDoublePar39, 24);
	BindEvents(m_staticTextPar25, m_choicePar25, 25);
	BindEvents(m_staticTextPar26, m_checkBoxPar26, 26);
	BindEvents(m_staticTextPar27, m_checkBoxPar27, 27);
	BindEvents(m_staticTextPar28, m_filePickerPar28, 28);
	BindEvents(m_staticTextPar29, m_spinCtrlPar29, 29);
	BindEvents(m_staticTextPar30, m_choicePar30, 30);
	BindEvents(m_staticTextPar31, m_choicePar31, 31);
	BindEvents(m_staticTextPar32, m_spinCtrlPar32, 32);
	BindEvents(m_staticTextPar33, m_spinCtrlPar33, 33);
	BindEvents(m_staticTextPar34, m_spinCtrlDoublePar34, 34);
	BindEvents(m_staticTextPar35, m_spinCtrlDoublePar35, 35);
	BindEvents(m_staticTextPar36, m_checkBoxPar36, 36);
	BindEvents(m_staticTextPar37, m_checkBoxPar37, 37);
	BindEvents(m_staticTextPar38, m_checkBoxPar38, 38);
}

void HRTImagingToolParameterDialog::SyncParToDlg(const CSNPDatasetParameters& parameters) const
{
	auto SelectChoiceEntry = [](wxChoice* control, ptrdiff_t value) -> void {
		for (size_t i = 0; i < control->GetCount(); i++)
		{
			ptrdiff_t temp;
			std::wistringstream iss(control->GetString(i).ToStdWstring());
			iss >> temp;
			ASSERT(!iss.fail());
			if (!iss.fail() && (temp == value))
			{
				control->SetSelection(i);
				break;
			}
		}
		ASSERT(control->GetSelection() != wxNOT_FOUND);
		};

	m_spinCtrlPar01->SetValue(parameters.nMaxSequenceLength);
	m_spinCtrlPar02->SetValue(parameters.nVignettingCreationMaxImages);
	m_checkBoxPar03->SetValue(parameters.bShiftCorrection);
	m_checkBoxPar04->SetValue(parameters.bBrightnessCorrectionBeforeRegistration);
	m_checkBoxPar05->SetValue(parameters.bVignettingCorrectionBeforeRegistration);
	m_filePickerPar06->SetPath(parameters.sVignettingFileBeforeRegistration);
	m_spinCtrlPar07->SetValue(parameters.nVignettingSmoothingBeforeRegistration);
	SelectChoiceEntry(m_choicePar08, static_cast<ptrdiff_t>(parameters.eStrategie));
	m_spinCtrlDoublePar09->SetValue(parameters.fSelectionHeight);
	m_spinCtrlPar10->SetValue(parameters.nMaxImageDistance);
	m_spinCtrlPar11->SetValue(parameters.nMaxRegDistance);
	m_spinCtrlPar12->SetValue(parameters.nWindowSize);
	m_spinCtrlPar13->SetValue(parameters.nStepSize);
	m_spinCtrlPar14->SetValue(parameters.nSectorSize);
	SelectChoiceEntry(m_choicePar15, static_cast<ptrdiff_t>(parameters.eImagePairVerificator));
	SelectChoiceEntry(m_choicePar16, static_cast<ptrdiff_t>(parameters.eProcessType));
	m_spinCtrlDoublePar17->SetValue(parameters.fMinScore);
	m_spinCtrlDoublePar18->SetValue(parameters.fMinScoreFlexible);
	m_spinCtrlDoublePar19->SetValue(parameters.fCertainScore);
	m_spinCtrlDoublePar20->SetValue(parameters.fScalation);
	m_spinCtrlDoublePar21->SetValue(parameters.fScalationSubImages);
	m_spinCtrlPar22->SetValue(parameters.nSubImageHeight);
	m_checkBoxPar23->SetValue(parameters.bConsistencyCheck);
	m_checkBoxPar24->SetValue(parameters.bAutomaticThresholdDetection);
	m_spinCtrlDoublePar39->SetValue(parameters.fResidualThreshold);
	SelectChoiceEntry(m_choicePar25, static_cast<ptrdiff_t>(parameters.eSolver));
	m_checkBoxPar26->SetValue(parameters.bBrightnessCorrectionBeforeCompositing);
	m_checkBoxPar27->SetValue(parameters.bVignettingCorrectionBeforeCompositing);
	m_filePickerPar28->SetPath(parameters.sVignettingFileBeforeCompositing);
	m_spinCtrlPar29->SetValue(parameters.nVignettingSmoothingBeforeCompositing);
	SelectChoiceEntry(m_choicePar30, static_cast<ptrdiff_t>(parameters.eCompositingMode));
	SelectChoiceEntry(m_choicePar31, static_cast<ptrdiff_t>(parameters.eWeightMode));
	m_spinCtrlPar32->SetValue(parameters.nBorder);
	m_spinCtrlPar33->SetValue(parameters.cBackgroundColor);
	m_spinCtrlDoublePar34->SetValue(parameters.fIntensityLimit);
	m_spinCtrlDoublePar35->SetValue(parameters.fZMultiplier);
	m_checkBoxPar36->SetValue(parameters.bExcludeNonSNPImages);
	m_checkBoxPar37->SetValue(parameters.bForceExportMotionCorrectedImages);
	m_checkBoxPar38->SetValue(parameters.bDetailedLogging);
}

void HRTImagingToolParameterDialog::SyncDlgToPar(CSNPDatasetParameters& parameters) const
{
	auto ParseChoiceEntry = [](wxChoice* control, ptrdiff_t& value) -> bool {
		ASSERT(!control->GetStringSelection().empty());
		ptrdiff_t temp;
		std::wistringstream iss(control->GetStringSelection().ToStdWstring());
		iss >> temp;
		ASSERT(!iss.fail());
		if (!iss.fail())
		{
			value = temp;
			return true;
		}
		return false;
		};

	ptrdiff_t enumValue;
	parameters.nMaxSequenceLength = m_spinCtrlPar01->GetValue();
	parameters.nVignettingCreationMaxImages = m_spinCtrlPar02->GetValue();
	parameters.bShiftCorrection = m_checkBoxPar03->GetValue();
	parameters.bBrightnessCorrectionBeforeRegistration = m_checkBoxPar04->GetValue();
	parameters.bVignettingCorrectionBeforeRegistration = m_checkBoxPar05->GetValue();
	parameters.sVignettingFileBeforeRegistration = m_filePickerPar06->GetPath();
	parameters.nVignettingSmoothingBeforeRegistration = m_spinCtrlPar07->GetValue();
	if (ParseChoiceEntry(m_choicePar08, enumValue)) parameters.eStrategie = static_cast<ERegistrationStrategy>(enumValue);
	parameters.fSelectionHeight = m_spinCtrlDoublePar09->GetValue();
	parameters.nMaxImageDistance = m_spinCtrlPar10->GetValue();
	parameters.nMaxRegDistance = m_spinCtrlPar11->GetValue();
	parameters.nWindowSize = m_spinCtrlPar12->GetValue();
	parameters.nStepSize = m_spinCtrlPar13->GetValue();
	parameters.nSectorSize = m_spinCtrlPar14->GetValue();
	if (ParseChoiceEntry(m_choicePar15, enumValue)) parameters.eImagePairVerificator = static_cast<EImagePairVerificator>(enumValue);
	if (ParseChoiceEntry(m_choicePar16, enumValue)) parameters.eProcessType = static_cast<CProcessType::EProcessType>(enumValue);
	parameters.fMinScore = m_spinCtrlDoublePar17->GetValue();
	parameters.fMinScoreFlexible = m_spinCtrlDoublePar18->GetValue();
	parameters.fCertainScore = m_spinCtrlDoublePar19->GetValue();
	parameters.fScalation = m_spinCtrlDoublePar20->GetValue();
	parameters.fScalationSubImages = m_spinCtrlDoublePar21->GetValue();
	parameters.nSubImageHeight = m_spinCtrlPar22->GetValue();
	parameters.bConsistencyCheck = m_checkBoxPar23->GetValue();
	parameters.bAutomaticThresholdDetection = m_checkBoxPar24->GetValue();
	parameters.fResidualThreshold = m_spinCtrlDoublePar39->GetValue();
	if (ParseChoiceEntry(m_choicePar25, enumValue)) parameters.eSolver = static_cast<CSLESolver::EAlgorithm>(enumValue);
	parameters.bBrightnessCorrectionBeforeCompositing = m_checkBoxPar26->GetValue();
	parameters.bVignettingCorrectionBeforeCompositing = m_checkBoxPar27->GetValue();
	parameters.sVignettingFileBeforeCompositing = m_filePickerPar28->GetPath();
	parameters.nVignettingSmoothingBeforeCompositing = m_spinCtrlPar29->GetValue();
	if (ParseChoiceEntry(m_choicePar30, enumValue)) parameters.eCompositingMode = static_cast<CCompositingParameters::ECompositingType>(enumValue);
	if (ParseChoiceEntry(m_choicePar31, enumValue)) parameters.eWeightMode = static_cast<CCompositingParameters::EWeightMode>(enumValue);
	parameters.nBorder = m_spinCtrlPar32->GetValue();
	parameters.cBackgroundColor = m_spinCtrlPar33->GetValue();
	parameters.fIntensityLimit = m_spinCtrlDoublePar34->GetValue();
	parameters.fZMultiplier = m_spinCtrlDoublePar35->GetValue();
	parameters.bExcludeNonSNPImages = m_checkBoxPar36->GetValue();
	parameters.bForceExportMotionCorrectedImages = m_checkBoxPar37->GetValue();
	parameters.bDetailedLogging = m_checkBoxPar38->GetValue();
}

void HRTImagingToolParameterDialog::ImportParameters()
{
	std::wstring regpath = L"HIT/Offline";
	m_initialParameters = CSNPDatasetParameterLoader::FromRegistry(regpath);

	SyncParToDlg(m_initialParameters);
}

void HRTImagingToolParameterDialog::ExportParameters() const
{
	CSNPDatasetParameters parameters;

	SyncDlgToPar(parameters);

	std::wstring regpath = L"HIT/Offline";
	CSNPDatasetParameterExporter::ExportChangesToRegistry(regpath, parameters, m_initialParameters);
}

CSNPDatasetParameters HRTImagingToolParameterDialog::CreateDefaults()
{
	CSNPDatasetParameters parameters;

	parameters.nMaxSequenceLength = 0;
	parameters.nVignettingCreationMaxImages = 0;
	parameters.bShiftCorrection = false;
	parameters.bBrightnessCorrectionBeforeRegistration = false;
	parameters.bVignettingCorrectionBeforeRegistration = true;
	parameters.sVignettingFileBeforeRegistration = L"";
	parameters.nVignettingSmoothingBeforeRegistration = 0;
	parameters.eStrategie = ERegistrationStrategy::eComplete;
	parameters.fSelectionHeight = 1.0;
	parameters.nMaxImageDistance = 400;
	parameters.nMaxRegDistance = 8;
	parameters.nWindowSize = 3;
	parameters.nStepSize = 3;
	parameters.nSectorSize = 30;
	parameters.eImagePairVerificator = EImagePairVerificator::eNone;
	parameters.eProcessType = CProcessType::eHRTImageRegistration;
	parameters.fMinScore = 8.0;
	parameters.fMinScoreFlexible = 16.0;
	parameters.fCertainScore = 24.0;
	parameters.fScalation = 3.0;
	parameters.fScalationSubImages = 2.0;
	parameters.nSubImageHeight = 32;
	parameters.bConsistencyCheck = false;
	parameters.bAutomaticThresholdDetection = false;
	parameters.fResidualThreshold = 10.0;
	parameters.eSolver = CSLESolver::EAlgorithm::eCGALGLIB;
	parameters.bBrightnessCorrectionBeforeCompositing = false;
	parameters.bVignettingCorrectionBeforeCompositing = true;
	parameters.sVignettingFileBeforeCompositing = L"";
	parameters.nVignettingSmoothingBeforeCompositing = 0;
	parameters.eCompositingMode = CCompositingParameters::ECompositingType::eFusion;
	parameters.eWeightMode = CCompositingParameters::EWeightMode::eCos2;
	parameters.nBorder = 0;
	parameters.cBackgroundColor = 0.0;
	parameters.fIntensityLimit = 0.0;
	parameters.fZMultiplier = 2.0;
	parameters.bExcludeNonSNPImages = false;
	parameters.bForceExportMotionCorrectedImages = false;
	parameters.bDetailedLogging = false;

	return parameters;
}

CSNPDatasetParameters HRTImagingToolParameterDialog::CreateDefaults2D()
{
	CSNPDatasetParameters parameters = CreateDefaults();

	parameters.eStrategie = ERegistrationStrategy::eComplete;
	parameters.fSelectionHeight = 8.0;
	parameters.fMinScore = 5.0;
	parameters.fMinScoreFlexible = 10.0;
	parameters.fCertainScore = 30.0;
	parameters.fScalation = 3.0;
	parameters.fScalationSubImages = 2.0;
	parameters.bConsistencyCheck = true;
	parameters.bAutomaticThresholdDetection = true;
	parameters.fResidualThreshold = 10.0;
	parameters.fZMultiplier = 1.0;

	return parameters;
}

CSNPDatasetParameters HRTImagingToolParameterDialog::CreateDefaultsSNP()
{
	CSNPDatasetParameters parameters = CreateDefaults();

	parameters.eStrategie = ERegistrationStrategy::eComplete;
	parameters.fSelectionHeight = 8.0;
	parameters.fMinScore = 5.0;
	parameters.fMinScoreFlexible = 10.0;
	parameters.fCertainScore = 30.0;
	parameters.fScalation = 3.0;
	parameters.fScalationSubImages = 2.0;
	parameters.bConsistencyCheck = true;
	parameters.bAutomaticThresholdDetection = true;
	parameters.fZMultiplier = 1.0;
	parameters.bExcludeNonSNPImages = true;

	return parameters;
}

CSNPDatasetParameters HRTImagingToolParameterDialog::CreateDefaults3DVoxel()
{
	CSNPDatasetParameters parameters = CreateDefaults();

	parameters.eStrategie = ERegistrationStrategy::ePlaneSelect;
	parameters.fSelectionHeight = 8.0;
	parameters.fMinScore = 5.0;
	parameters.fMinScoreFlexible = 10.0;
	parameters.fCertainScore = 30.0;
	parameters.fScalation = 3.0;
	parameters.fScalationSubImages = 2.0;
	parameters.bConsistencyCheck = true;
	parameters.bAutomaticThresholdDetection = true;
	parameters.eCompositingMode = CCompositingParameters::ECompositingType::eVoxelBuffer;
	parameters.fZMultiplier = 1.0;
	parameters.bExcludeNonSNPImages = true;

	return parameters;
}

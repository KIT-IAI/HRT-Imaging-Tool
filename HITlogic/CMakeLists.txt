# CMakeList.txt: CMake project for "HITlogic".
# Include the sources and define project-specific logic here.
#

# Specify build target for the library
add_library (HITlogic STATIC)

# Define list of project source files
set(SRC_FILES
    3DBuffer.cpp
    3DCloud.cpp
    BorderCropper.cpp
    BrightnessCorrection.cpp
    Compositing.cpp
    Compositing3DCloud.cpp
    CompositingFactory.cpp
    CompositingFusion.cpp
    CompositingMax.cpp
    CompositingParameters.cpp
    CompositingTiling.cpp
    CompositingVoxelBuffer.cpp
    CorrelationOffset.cpp
    CorrelationOffsetClassifier.cpp
    CorrelationParameters.cpp
    DIPLOMTable.cpp
    DIPLOMTableRow.cpp
    HRTGlobalPositioning.cpp
    HRTImageClassificationData.cpp
    HRTImageDepthAndAngleCalculator.cpp
    HRTImageDepthAndAngleData.cpp
    HrtIterativeSLEWorker.cpp
    HrtPreProcessingPipeline.cpp
    ImageCorrelation.cpp
    ImageCorrelator.cpp
    ImageLoader.cpp
    ImageMasker.cpp
    ImagePair.cpp
    ImagePairVerificator.cpp
    ImageRegistrationData.cpp
    ImageRegistrationResult.cpp
    ImageRegistrator.cpp
    ImageUndistortion.cpp
    LockList.cpp
    MosaicImageProperties.cpp
    NoPairVerificator.cpp
    PairVerificatorFactory.cpp
    PiezoImageTime.cpp
    PositionAwarePairVerificator.cpp
    ProcessType.cpp
    RegistrationPostProcessor.cpp
    RegistrationProcedureParameters.cpp
    RegistrationResult.cpp
    RegistrationResultLoader.cpp
    RegistrationStrategy.cpp
    RegistrationStrategyFactory.cpp
    RegStepFlexibleRegistration.cpp
    RegStepImageScalation.cpp
    RegStepResidualFilter.cpp
    RegStepRigidRegistration.cpp
    RegStepScoreThresholdAdapter.cpp
    RegStepScoreThresholdApproximator.cpp
    RegStepScoreThresholdOptimizer.cpp
    RegStepZeroOffsetFilter.cpp
    Residual.cpp
    RigidRegistrationResult.cpp
    ScoreContainer.cpp
    ScoreNormalizer.cpp
    SNPDatasetBatchProcessor.cpp
    SNPDatasetLoader.cpp
    SNPDatasetOutputParameters.cpp
    SNPDatasetParameterExporter.cpp
    SNPDatasetParameterLoader.cpp
    SNPDatasetParameters.cpp
    SNPDatasetResultLoader.cpp
    SNPFusionDataset.cpp
    SNPFusionEvent.cpp
    SNPFusionException.cpp
    StlImage.cpp
    StrategyBackForth.cpp
    StrategyComplete.cpp
    StrategyGraph.cpp
    StrategyPlanes.cpp
    StrategySector.cpp
    StrategyStep.cpp
    StrategyWindow.cpp
    SubImageCorrelation.cpp
    SubImageCorrelator.cpp
    RegStepSubImageScoreThresholdAdapter.cpp
    TreeBasedPairVerificator.cpp
    VignettingCorrection.cpp
    )

# Define list of project header files
set(HDR_FILES
    3DBuffer.h
    3DCloud.h
    BorderCropper.h
    BrightnessCorrection.h
    Compositing.h
    Compositing3DCloud.h
    CompositingFactory.h
    CompositingFusion.h
    CompositingMax.h
    CompositingParameters.h
    CompositingTiling.h
    CompositingVoxelBuffer.h
    CorrelationOffset.h
    CorrelationOffsetClassifier.h
    CorrelationParameters.h
    DIPLOMTable.h
    DIPLOMTableRow.h
    GlobalPositioningParameters.h
    HRTGlobalPositioning.h
    HRTImageClassificationData.h
    HRTImageDepthAndAngleCalculator.h
    HRTImageDepthAndAngleData.h
    HrtImageParameters.h
    HrtIterativeSLEWorker.h
    HrtPreProcessingParameters.h
    HrtPreProcessingPipeline.h
    HrtValidityCodes.h
    ImageCorrelation.h
    ImageCorrelator.h
    ImageLoader.h
    ImageMasker.h
    ImagePair.h
    ImagePairVerificator.h
    ImageRegistrationData.h
    ImageRegistrationResult.h
    ImageRegistrator.h
    ImageUndistortion.h
    LockList.h
    MosaicImageProperties.h
    NoPairVerificator.h
    PairVerificatorFactory.h
    PiezoImageTime.h
    PositionAwarePairVerificator.h
    ProcessType.h
    RegistrationPostProcessor.h
    RegistrationProcedureParameters.h
    RegistrationResult.h
    RegistrationResultLoader.h
    RegistrationScoreParameters.h
    RegistrationStrategy.h
    RegistrationStrategyFactory.h
    RegistrationStrategyParameters.h
    RegistrationVerificationParameters.h
    RegStepFlexibleRegistration.h
    RegStepImageScalation.h
    RegStepResidualFilter.h
    RegStepRigidRegistration.h
    RegStepScoreThresholdAdapter.h
    RegStepScoreThresholdApproximator.h
    RegStepScoreThresholdOptimizer.h
    RegStepZeroOffsetFilter.h
    Residual.h
    RigidRegistrationResult.h
    ScoreContainer.h
    ScoreNormalizer.h
    SNPDatasetBatchProcessor.h
    SNPDatasetLoader.h
    SNPDatasetOutputParameters.h
    SNPDatasetParameterExporter.h
    SNPDatasetParameterLoader.h
    SNPDatasetParameters.h
    SNPDatasetResultLoader.h
    SNPFusionDataset.h
    SNPFusionEvent.h
    SNPFusionEventHandler.h
    SNPFusionException.h
    SNPGroupResult.h
    StlImage.h
    StlImageAlignedAllocator.h
    StrategyBackForth.h
    StrategyComplete.h
    StrategyGraph.h
    StrategyPlanes.h
    StrategySector.h
    StrategyStep.h
    StrategyWindow.h
    SubImageCorrelation.h
    SubImageCorrelator.h
    RegStepSubImageScoreThresholdAdapter.h
    TreeBasedPairVerificator.h
    UndistortedImage.h
    VignettingCorrection.h
    )

# Define list of project include folders
set(HDR_FOLDERS
    ${PROJECT_SOURCE_DIR}/alglib
    ${PROJECT_SOURCE_DIR}/ImageIO
    ${PROJECT_SOURCE_DIR}/Utilities
    )

# The following line can provide the memcpy_s() and some other ..._s() functions
# on some systems, but apparently Linux systems are generally not among them
# target_compile_definitions(HITlogic PRIVATE __STDC_WANT_LIB_EXT1__=1)

# Specify precompiled headers
target_precompile_headers(HITlogic PRIVATE stdafx.h)

# Specify source and header files
target_sources(HITlogic PRIVATE ${SRC_FILES})
target_sources(HITlogic PRIVATE ${HDR_FILES})

# Specify include directories
target_include_directories(HITlogic PRIVATE ${HDR_FOLDERS})

# link required libraries
target_link_libraries(HITlogic PRIVATE MKL::MKL)
target_link_libraries(HITlogic PRIVATE TBB::tbb TBB::tbbmalloc)
target_link_libraries(HITlogic PRIVATE alglib)
target_link_libraries(HITlogic PRIVATE ImageIO)
target_link_libraries(HITlogic PRIVATE Utilities)

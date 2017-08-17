// Copyright (c) Henry Cooney 2017
 
#include "rtikEditor.h"
 
IMPLEMENT_GAME_MODULE(FrtikEditorModule, rtikEditor);

DEFINE_LOG_CATEGORY(LogRTIKEditor)
 
void FrtikEditorModule::StartupModule()
{
	UE_LOG(LogRTIKEditor, Warning, TEXT("IK editor module staring"));
}
 
void FrtikEditorModule::ShutdownModule()
{
	UE_LOG(LogRTIKEditor, Warning, TEXT("IK editor module shutdown"));
}
 

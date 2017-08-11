#include "ue4ikEditor.h"
 
IMPLEMENT_GAME_MODULE(Fue4ikEditorModule, ue4ikEditor);

DEFINE_LOG_CATEGORY(LogRTIKEditor)
 
void Fue4ikEditorModule::StartupModule()
{
	UE_LOG(LogRTIKEditor, Warning, TEXT("IK editor module staring"));
}
 
void Fue4ikEditorModule::ShutdownModule()
{
	UE_LOG(LogRTIKEditor, Warning, TEXT("IK editor module shutdown"));
}
 

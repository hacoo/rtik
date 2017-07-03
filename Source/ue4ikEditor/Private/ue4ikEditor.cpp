#include "ue4ikEditor.h"
 
IMPLEMENT_GAME_MODULE(Fue4ikEditorModule, ue4ikEditor);

DEFINE_LOG_CATEGORY(LogIKEditor)
 
void Fue4ikEditorModule::StartupModule()
{
	UE_LOG(LogIKEditor, Warning, TEXT("IK editor module staring"));
}
 
void Fue4ikEditorModule::ShutdownModule()
{
	UE_LOG(LogIKEditor, Warning, TEXT("IK editor module shutdown"));
}
 

// Copyright (c) Henry Cooney 2017
 
#pragma once
 
#include "Engine.h"
#include "ModuleManager.h"
#include "LogMacros.h"
#include "UnrealEd.h"
 
DECLARE_LOG_CATEGORY_EXTERN(LogRTIKEditor, All, All)
 
class FrtikEditorModule
	: public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
 
};

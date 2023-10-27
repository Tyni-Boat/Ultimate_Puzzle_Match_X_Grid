// Copyright © 2023 by Tyni Boat. All Rights Reserved.

#include "Match3Puzzle.h"

#define LOCTEXT_NAMESPACE "FMatch3PuzzleModule"

void FMatch3PuzzleModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FMatch3PuzzleModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMatch3PuzzleModule, Match3Puzzle)
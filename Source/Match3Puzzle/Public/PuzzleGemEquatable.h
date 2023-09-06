// Copyright © 2023 by Tyni Boat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PuzzleGemEquatable.generated.h"


// This class does not need to be modified.
UINTERFACE()
class UPuzzleGemEquatable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for gem comparison. It can be a color, a number or anything that can be compared. It used for matching in the grid. 
 */
class MATCH3PUZZLE_API IPuzzleGemEquatable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	//Compare this gem equatable to another one. return true if they match.
	UFUNCTION(BlueprintNativeEvent)
	bool GemEquals(const TScriptInterface<IPuzzleGemEquatable>& other);

	
	//Compare this gem equatable to another one. return true if they match. c++
	virtual bool GemEquals_Implementation(const TScriptInterface<IPuzzleGemEquatable>& other);
};

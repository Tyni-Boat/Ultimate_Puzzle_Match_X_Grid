// Copyright © 2023 by Tyni Boat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"



#pragma region Enums

//Represent the state of a gem
UENUM(BlueprintType)
enum EGemState
{
	none = 0,
	idle = 1,
	falling = 2,
	swapping = 3,
	selected = 4,
	pendingDeletion = 5,
};


//The movement easing function type
UENUM(BlueprintType)
enum EMoveEasingType
{
	linear = 0,
	outExpo = 1,
	outSine = 2,
	outBack = 3,
	outBounce = 4,
};


//The apparition point of the gem when node request came from the grid deletion pool
UENUM(BlueprintType)
enum EGridGemNodePopMethod
{
	fromEndOfLane,
	fromBeginOfLane,
	fromLastLaneDirection,
	fromNextLaneDirection,
	fromGridNormal,
	fromGridInverseNormal,
	popAtPosition,
};

//The gem filling strategy of the grid
UENUM(BlueprintType)
enum EGridFillingStrategy
{
	NoStrategy,
	EveryOnePop,
	InnerOnlyPop,
	EveryOneNormal,
	InnerOnlyNormal,
	EveryOneNormalInverse,
	InnerOnlyNormalInverse,
};

//The Grid Gameplay Mode
UENUM(BlueprintType)
enum EGridGameplayMode
{
	SwapGemAndMatch,
	ClickAndDestroy,
};

#pragma endregion
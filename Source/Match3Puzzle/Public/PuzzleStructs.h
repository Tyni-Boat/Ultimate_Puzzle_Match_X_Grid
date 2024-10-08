// Copyright � 2023 by Tyni Boat. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "PuzzleGem.h"
#include "PuzzleGemUIWidget.h"
#include "Components/SplineComponent.h"
#include "PuzzleStructs.generated.h"


#pragma region Structures

//The structure used to swap gems.
USTRUCT(BlueprintType)
struct FGemSwapHandler
{
	GENERATED_BODY()

public:
	//First gem of the swap
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Match3Puzzle")
	APuzzleGem* GemA;

	//second gem of the swap
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Match3Puzzle")
	APuzzleGem* GemB;

	//The completion percentage of the swap [0-1]
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Match3Puzzle")
	float swapCompletion;

	//Is the swap made by the user/auto match or from a swap back action?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Match3Puzzle")
	bool isUserMadeSwap;

	//The path the gems will follow while swapping
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Match3Puzzle")
	USplineComponent* CustomPath;

public:
	FGemSwapHandler()
	{
		GemA = nullptr;
		GemB = nullptr;
		swapCompletion = 0;
		isUserMadeSwap = false;
	}

	FGemSwapHandler(APuzzleGem* A, APuzzleGem* B, bool userMade = false)
	{
		GemA = A;
		GemB = B;
		swapCompletion = 0;
		isUserMadeSwap = userMade;
	}


	//Return true if the swap object is valid
	bool IsValid() const { return GemA && GemB && GemA->CanMoveGem() && GemB->CanMoveGem(); }
};


//The structure used to swap gems in UI
USTRUCT(BlueprintType)
struct FUIGemSwapHandler
{
	GENERATED_BODY()

public:
	//First gem of the swap
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Match3Puzzle")
	UPuzzleGemUIWidget* GemA;

	//second gem of the swap
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Match3Puzzle")
	UPuzzleGemUIWidget* GemB;

	//The completion percentage of the swap [0-1]
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Match3Puzzle")
	float swapCompletion;

	//Is the swap made by the user/auto match or from a swap back action?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Match3Puzzle")
	bool isUserMadeSwap;

	//The path the gems will follow while swapping
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Match3Puzzle")
	USplineComponent* CustomPath;

public:
	FUIGemSwapHandler()
	{
		GemA = nullptr;
		GemB = nullptr;
		swapCompletion = 0;
		isUserMadeSwap = false;
	}

	FUIGemSwapHandler(UPuzzleGemUIWidget* A, UPuzzleGemUIWidget* B, bool userMade = false)
	{
		GemA = A;
		GemB = B;
		swapCompletion = 0;
		isUserMadeSwap = userMade;
	}


	//Return true if the swap object is valid
	bool IsValid() const { return GemA && GemB && GemA->CanMoveGem() && GemB->CanMoveGem(); }
};


//The structure used handle matches.
USTRUCT(BlueprintType)
struct FGridMatch
{
	GENERATED_BODY()

public:
	//The gem positions of this match
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Match3Puzzle")
	TArray<FVector2D> MatchPositions;

public:
	FGridMatch()
	{
	}

	FGridMatch(TArray<FVector2D> positions)
	{
		MatchPositions.Empty();
		if (positions.Num() > 0)
		{
			for (auto pos : positions)
				MatchPositions.AddUnique(pos);
		}
	}

	FGridMatch(TArray<FVector2D> positions, TArray<FVector2D> swappedGems)
	{
		MatchPositions.Empty();
		if (positions.Num() > 0)
		{
			for (auto pos : positions)
				MatchPositions.AddUnique(pos);
		}

		if (MatchPositions.Num() > 0 && swappedGems.Num() > 0)
		{
			int lowestIndexInSwaps = TNumericLimits<int>().Max();
			for (int i = MatchPositions.Num() - 1; i >= 0; i--)
			{
				FVector2D matchPos = MatchPositions[i];
				const int swapIndex = swappedGems.IndexOfByPredicate([matchPos](FVector2D swap)-> bool
				{
					return swap == matchPos;
				});
				if(!swappedGems.IsValidIndex(swapIndex))
					continue;
				if(swapIndex >= lowestIndexInSwaps)
					continue;
				lowestIndexInSwaps = swapIndex;
				MatchPositions.Add(matchPos);
				MatchPositions.RemoveAt(i);
			}
		}
	}

	//Check intersection with another match and return intersection indexes; X=this index, Y=other index
	FVector2D Intersect(FGridMatch other)
	{
		if (MatchPositions.Num() <= 0 || other.MatchPositions.Num() <= 0)
			return FVector2D(-1, -1);
		for (int i = 0; i < MatchPositions.Num(); i++)
		{
			for (int j = 0; j < other.MatchPositions.Num(); j++)
			{
				if (MatchPositions[i] == other.MatchPositions[j])
					return FVector2D(i, j);
			}
		}
		return FVector2D(-1, -1);
	}

	//Clear the match
	void Clear()
	{
		MatchPositions.Empty();
	}

	//Check if match is empty
	bool IsEmpty()
	{
		return MatchPositions.IsEmpty();
	}
};

#pragma endregion

// Copyright © 2023 by Tyni Boat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PuzzleEnums.h"
#include "PuzzleGridUIWidget.h"
#include "PuzzleGemUIWidget.h"

#ifndef UINODES
#define UINODES
#include "PuzzleNodeUIWidget.h"
#endif

#include "Components/VerticalBox.h"
#include "PuzzleLaneUIWidget.generated.h"


// A Puzzle Lane is a virtual line, holding a row or column of Puzzle Nodes. Cannot Operate outside of a Puzzle Grid.
UCLASS()
class MATCH3PUZZLE_API UPuzzleLaneUIWidget : public UVerticalBox
{
	GENERATED_BODY()

#pragma region Properties

public:
	//The Nodes's Class
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Lane|Class Defaults")
	TSubclassOf<UPuzzleNodeUIWidget> NodeClass;

	//The Distance of the recycling zone
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Lane|Lane SetUp")
	float RecyclingZoneDistance = 5;


#pragma endregion

#pragma region Caches

protected:
	// The nodes currently in the Lane.
	UPROPERTY()
	TArray<UPuzzleNodeUIWidget*> _nodesInLane;

	// The reference to the parent grid
	UPROPERTY()
	UPuzzleGridUIWidget* _grid;

	// The Lane's Index i the grid
	UPROPERTY()
	int _indexInGrid;

	// The vector from the lane position to the recycler one
	UPROPERTY()
	FVector2D _directionToRecycler;

	// The position of the center of the lane
	UPROPERTY()
	FVector2D _laneCenter;

#pragma endregion

#pragma region Public functions

public:
	//Get the lane's Parent Grid
	UFUNCTION(BlueprintCallable, Category="Query")
	FORCEINLINE UPuzzleGridUIWidget* GetParentGrid() { return _grid; }

	//Get the node at the required index
	UFUNCTION(BlueprintCallable, Category="Query")
	FORCEINLINE UPuzzleNodeUIWidget* GetNodeAtIndex(int index)
	{
		if (_nodesInLane.IsValidIndex(index))
			return _nodesInLane[index];
		return nullptr;
	}

	//Get the lane's index in Parent Grid
	UFUNCTION(BlueprintCallable, Category="Query")
	FORCEINLINE int GetIndexInGrid() { return _indexInGrid; }

#pragma endregion

#pragma region internal functions

public:
	//Initialize the lane
	UFUNCTION(BlueprintCallable, Category="Puzzle Lane|Life Time")
	float InitializeLane(UPuzzleGridUIWidget* grid, int lane_index,
	                    EGridGemNodePopMethod innerGemPopMethod = popAtPosition,
	                    bool popMethodForAll = false);

	//Clear the lane and delete all nodes
	UFUNCTION(BlueprintCallable, Category="Puzzle Lane|Life Time")
	void CLearLane();

	//Get the Lane recycler position
	UFUNCTION(BlueprintCallable, Category="Puzzle Lane|Query")
	FVector2D GetLaneRecyclerLocation()
	{
		return _laneCenter + _directionToRecycler;
	}

	//Get a gem form the next node or fron the grid
	UFUNCTION(BlueprintCallable, Category="Puzzle Lane|Query")
	UPuzzleGemUIWidget* GetGemCascade(int nodeYindex, float deltaTime, bool& fromGrid);

	//Set gem at index as the gem in grid at that index
	UFUNCTION(BlueprintCallable, Category="Puzzle Lane|Query")
	void SetGemInGrid(int nodeYindex, APuzzleGem* gem);

	//Add force to all nodes
	void AddForce(FVector2D force);

	//Add radial force to nodes in range. maxIntensity is the force strength at the center.
	void AddRadialForce(FVector2D center, float radius, float maxIntensity);

#pragma endregion

#pragma region Class FLow

protected:
	// Called when the game starts
	UFUNCTION(BlueprintCallable, Category="Program Flow")
	virtual void BeginPlay();

public:
	// Called every frame
	UFUNCTION(BlueprintCallable, Category="Program Flow")
	virtual void TickComponent(float DeltaTime);

#pragma endregion
};

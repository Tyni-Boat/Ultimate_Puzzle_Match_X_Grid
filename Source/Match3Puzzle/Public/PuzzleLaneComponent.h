// Copyright © 2023 by Tyni Boat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#ifndef NODES
#define NODES
#include "PuzzleNodeComponent.h"
#endif
#ifndef GRID
#define GRID
#include "PuzzleGridComponent.h"
#endif
#include "PuzzleLaneComponent.generated.h"


// A Puzzle Lane is a virtual line, holding a row or column of Puzzle Nodes. Cannot Operate outside of a Puzzle Grid.
UCLASS(ClassGroup = (Match3Puzzle), BlueprintType, Blueprintable
	, hidecategories = (Object, LOD, Lighting, TextureStreaming, Velocity, PlanarMovement, MovementComponent, Tags,
		Activation, Cooking, AssetUserData, Collision, Physics, ComponentTick)
	, meta = (BlueprintSpawnableComponent))
class MATCH3PUZZLE_API UPuzzleLaneComponent : public USceneComponent
{
	GENERATED_BODY()


#pragma region Properties

public:
	//The Nodes's Class
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Lane|Class Defaults")
	TSubclassOf<UPuzzleNodeComponent> NodeClass;

	//The Distance of the recycling zone
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Lane|Lane SetUp")
	float RecyclingZoneDistance = 5;


#pragma endregion

#pragma region Caches

protected:
	// The nodes currently in the Lane.
	UPROPERTY()
	TArray<UPuzzleNodeComponent*> _nodesInLane;

	// The reference to the parent grid
	UPROPERTY()
	UPuzzleGridComponent* _grid;

	// The Lane's Index i the grid
	UPROPERTY()
	int _indexInGrid;

	// The vector from the lane position to the recycler one
	UPROPERTY()
	FVector _directionToRecycler;

#pragma endregion

#pragma region Public functions

public:
	//Get the lane's Parent Grid
	UFUNCTION(BlueprintCallable, Category="Query")
	FORCEINLINE UPuzzleGridComponent* GetParentGrid() { return _grid; }

	//Get the node at the required index
	UFUNCTION(BlueprintCallable, Category="Query")
	FORCEINLINE UPuzzleNodeComponent* GetNodeAtIndex(int index)
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
	//Clear the lane and delete all nodes
	UFUNCTION(BlueprintCallable, Category="Puzzle Lane|Life Time")
	void InitializeLane(UPuzzleGridComponent* grid, int laneLength, int lane_index, EGridGemNodePopMethod innerGemPopMethod = popAtPosition, bool popMethodForAll = false);

	//Clear the lane and delete all nodes
	UFUNCTION(BlueprintCallable, Category="Puzzle Lane|Life Time")
	void CLearLane();

	//Get the Lane recycler position
	UFUNCTION(BlueprintCallable, Category="Puzzle Lane|Query")
	FVector GetLaneRecyclerLocation() { return GetComponentLocation() + GetComponentTransform().TransformVector(_directionToRecycler); }

	//Get a gem form the next node or fron the grid
	UFUNCTION(BlueprintCallable, Category="Puzzle Lane|Query")
	APuzzleGem* GetGemCascade(int nodeYindex, float deltaTime, bool& fromGrid);

	//Set gem at index as the gem in grid at that index
	UFUNCTION(BlueprintCallable, Category="Puzzle Lane|Query")
	void SetGemInGrid(int nodeYindex, APuzzleGem* gem);

	//Add force to all nodes
	void AddForce(FVector force);

	//Add radial force to nodes in range. maxIntensity is the force strength at the center.
	void AddRadialForce(FVector center, float radius, float maxIntensity);

#pragma endregion


#pragma region Class FLow

public:
	// Sets default values for this component's properties
	UPuzzleLaneComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};

#pragma endregion

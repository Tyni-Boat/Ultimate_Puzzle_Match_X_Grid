// Copyright © 2023 by Tyni Boat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PuzzleEnums.h"
#include "PuzzleGemUIWidget.h"

#ifndef UILANE
#define UILANE
#include "PuzzleLaneUIWidget.h"
#endif

#include "PuzzleNodeUIWidget.h"
#include "PuzzleStructs.h"
#include "Components/HorizontalBox.h"
#include "PuzzleGridUIWidget.generated.h"

// The Match3PuzzleGrid UI Widget
UCLASS(ClassGroup = (Match3Puzzle), BlueprintType, Blueprintable)
class MATCH3PUZZLE_API UPuzzleGridUIWidget : public UHorizontalBox
{
	GENERATED_BODY()


#pragma region Properties

public:
	//Class Defaults #############################################################################################

	//The Lane's Class
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Grid|Class Defaults")
		TSubclassOf<UPuzzleLaneUIWidget> LaneClass;

	//The Gem's Class
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Grid|Class Defaults")
		TSubclassOf<UPuzzleGemUIWidget> GemClass;

	//Grid Params #############################################################################################
	

	//The grid size. x define the number of lanes and y the lane's length
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Puzzle Grid|Grid Params")
		FVector2D GridSize;

	//The gameplay mode of the grid.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Grid|Grid Params")
		TEnumAsByte<EGridGameplayMode> GameplayMode;

	//The Node size in the grid
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Puzzle Grid|Grid Params")
		FVector2D NodeSize = FVector2D(100, 100);

	//The gem swap speed
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Grid|Grid Params")
		float SwapSpeed = 2;

	//The Minimum Gem count of a match
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Grid|Grid Params")
		int MinMatchCount = 3;


	//Grid Behaviours #############################################################################################

	//The Node size in the grid
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Puzzle Grid|Behavior")
		TEnumAsByte<EGridFillingStrategy> FillingStrategy;
	

#pragma endregion

#pragma region Caches

protected:
	// The Lanes currently in the grid.
	UPROPERTY()
		TArray<UPuzzleLaneUIWidget*> _lanesInGrid;

	// The array of every gems
	UPROPERTY()
		TArray<UPuzzleGemUIWidget*> _gemsAll;

	// The map of gem present in grid.
	UPROPERTY()
		TMap<FVector2D, UPuzzleGemUIWidget*> _gemsInGrid;

	// The array of gems waiting to be destroyed
	UPROPERTY()
		TArray<UPuzzleGemUIWidget*> _gemToBeDestroyed;

	// The array of gems in the recycler bin
	UPROPERTY()
		TArray<UPuzzleGemUIWidget*> _gemsRecyclerBin;

	//The last selected gem.
	UPROPERTY()
		UPuzzleGemUIWidget* _lastSelectedGem;

	//The actual distance of lanes
	UPROPERTY()
		float _laneDistance;

	//The actual distance of nodes
	UPROPERTY()
		float _nodeDistance;

	//The actual time scale of the grid
	UPROPERTY()
		float _gridTimeScale = 1;

	//The currently active swaps on the grid.
	UPROPERTY()
		TArray<FUIGemSwapHandler> _activeSwaps;

	//The position to consider while match making, no matter their gem's state
	UPROPERTY()
		TArray<FVector2D> _swapGridPositionExceptions;

	//The match array for all the grid.
	UPROPERTY()
		TArray<FGridMatch> _allGridMatches;

	//Multi-purpose position buffer. Use only sequentially to avoid errors
	UPROPERTY()
		TArray<FVector2D> _multiPurposePositionBuffer_1;

	//Multi-purpose position buffer. Use only sequentially to avoid errors
	UPROPERTY()
		TArray<FVector2D> _multiPurposePositionBuffer_2;


#pragma endregion

#pragma region LifeSpan functions

public:

	//Get all widgets of type T from a parent UI.
	template<typename T> void GetChildWidget(UPanelWidget* parent, TArray<T*>& children);

	//Get the center point of a widget.
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Tools")
	FVector2D GetWidgetCenterLocation(const UWidget* parent, const UWidget* Widget);


	//Initiaize the grid by getting children lanes and creating gems
	UFUNCTION(BlueprintCallable, Category = "Puzzle Grid|Life Time")
		void InitializeGrid();

	//Reset the grid delting All Lanes and gems
	UFUNCTION(BlueprintCallable, Category = "Puzzle Grid|Life Time")
		void ClearGrid();

	//Event when the grid just got Init.
	UFUNCTION(BlueprintNativeEvent)
		void OnGridInit();


	//Event when the grid just got Init.
	virtual void OnGridInit_Implementation();


	//get the node pop method when blocked, from grid strategy on init.
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Life Time")
		EGridGemNodePopMethod GetNodePopMethodFromGridStrategy(EGridFillingStrategy strategy, bool& forall);

	//Get the grid time scale.
	UFUNCTION(BlueprintGetter)
		float GetTimeScale() const;

	//Get the grid time scale.
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Life Time")
		void SetTimeScale(float timeScale);


#pragma endregion
	
#pragma region Interaction functions
//
//public:
//	//Handle the inputs and return the swap couple
//	UFUNCTION(BlueprintNativeEvent, Category = "Puzzle Grid|Inputs")
//		FUIGemSwapHandler HandleInputs();
//
//
//	//Handle the inputs and return the swap couple (c++)
//	virtual FUIGemSwapHandler HandleInputs_Implementation();
//
//	//Handle swaps on the grid and update their states. returns match positions.
//	void HandleSwapsOnGrid(FUIGemSwapHandler newSwap, TArray<FVector2D>& swapMatchPositions, float delta);
//
//	//Add force to all nodes on the grid
//	UFUNCTION(BlueprintCallable, Category = "Puzzle Grid|Inputs")
//		void AddForce(FVector2D force);
//
//	//Add radial force to nodes in range. maxIntensity is the force strength at the center.
//	UFUNCTION(BlueprintCallable, Category = "Puzzle Grid|Inputs")
//		void AddRadialForce(FVector2D center, float radius, float maxIntensity);

#pragma endregion

#pragma region Gem Query functions
//
//public:
//	//Get a gem on the grid by it's grid index.
//	UFUNCTION(BlueprintCallable, Category = "Puzzle Grid|Query")
//		UPuzzleGemUIWidget* GetGemAt(FVector2D grid_index);
//
//	//Set a gem on the grid at an grid index.
//	UFUNCTION(BlueprintCallable, Category = "Puzzle Grid|Query")
//		void SetGemAt(FVector2D grid_index, UPuzzleGemUIWidget* gem);
//
//	//Retrieve a gem from the recycler bin
//	UFUNCTION(BlueprintCallable, Category = "Puzzle Grid|Query")
//		UPuzzleGemUIWidget* GetRecycledGem(float deltaTime);
//
//	//Handle the update for gem thet need to be deleted.
//	void HandleGemToDelete(float delta);
//
//	//Mark a gem as pending delete
//	UFUNCTION(BlueprintCallable, Category = "Puzzle Grid|Gem Action")
//		void DeleteGem(UPuzzleGemUIWidget* gem);
//
//	//internaly delete gem and send it to the recycler bin
//	bool DeleteGem_Internal(UPuzzleGemUIWidget* gem);
//
//	//Check if a gem is waiting to be deleted by the grid
//	UFUNCTION(BlueprintCallable, Category = "Puzzle Grid|Query")
//		bool IsGemPendingDeletion(UPuzzleGemUIWidget* gem);
//
//	//Called when a gem got destroyed on the grid
//	UFUNCTION(BlueprintNativeEvent, Category = "Puzzle Grid|Gem Action")
//		void OnGemDeleted(UPuzzleGemUIWidget* gem);
//
//	//Called when a gem got destroyed on the grid
//	virtual void OnGemDeleted_Implementation(UPuzzleGemUIWidget* gem);
//
//	//Called when a gem got Spawn on the grid
//	UFUNCTION(BlueprintNativeEvent, Category = "Puzzle Grid|Gem Action")
//		void OnGemSpawned(UPuzzleGemUIWidget* gem, bool gridInitialization = false);
//
//	//Called when a gem got destroyed on the grid
//	virtual void OnGemSpawned_Implementation(UPuzzleGemUIWidget* gem, bool gridInitialization = false);
//
//	//Handle the gem selection events
//	void HandleSelected(FUIGemSwapHandler swap);
//
//	//Override this event to customize the condition to spawn a new gem into the grid
//	UFUNCTION(BlueprintNativeEvent)
//		bool SpawnGemCondition(UPuzzleGemUIWidget* gemRequested);
//
//	//Override this event to customize the condition to spawn a new gem into the grid
//	virtual  bool SpawnGemCondition_Implementation(UPuzzleGemUIWidget* gemRequested);

#pragma endregion

#pragma region Node Query functions
//
//public:
//	//Get a node on the grid by it's grid index.
//	UFUNCTION(BlueprintCallable, Category = "Puzzle Grid|Query")
//		UPuzzleNodeUIWidget* GetNodeAt(FVector2D grid_index);

#pragma endregion

#pragma region Spatial functions
//
//public:
//	//Get the direction in which node where spawn. include distance from node to node
//	UFUNCTION(BlueprintGetter, Category = "Puzzle Grid|Query")
//		FVector2D GetNodeDirection() const;
//
//	//Get the direction in which Lane where spawn. include distance from lane to lane
//	UFUNCTION(BlueprintGetter, Category = "Puzzle Grid|Query")
//		FVector2D GetLaneDirection() const;

#pragma endregion

#pragma region Class Flow

protected:
	// Called when the game starts
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|LifeSpan")
	virtual void BeginPlay();

public:
	// Called every frame
	UFUNCTION(BlueprintCallable, Category = "Puzzle Grid|LifeSpan")
	virtual void Tick(float DeltaTime);

#pragma endregion

};

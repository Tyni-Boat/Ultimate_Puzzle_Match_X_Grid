// Copyright © 2023 by Tyni Boat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PuzzleGem.h"
#include "PuzzleLaneComponent.h"
#include "Components/SceneComponent.h"
#include "PuzzleStructs.h"
#include "PuzzleLaneComponent.h"
#include "PuzzleGridComponent.generated.h"


// The Match3PuzzleGrid component
UCLASS(ClassGroup = (Match3Puzzle), BlueprintType, Blueprintable, Abstract
	, hidecategories = (Object, LOD, Lighting, TextureStreaming, Velocity, PlanarMovement, MovementComponent, Tags,
		Activation, Cooking, AssetUserData, Collision, Physics)
	, meta = (BlueprintSpawnableComponent))
class MATCH3PUZZLE_API UPuzzleGridComponent : public USceneComponent
{
	GENERATED_BODY()


#pragma region Properties

public:
	//Class Defaults #############################################################################################

	//The Lane's Class
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Grid|Class Defaults")
	TSubclassOf<UPuzzleLaneComponent> LaneClass;

	//The Gem's Class
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Grid|Class Defaults")
	TSubclassOf<APuzzleGem> GemClass;

	//The socket of the spawned gems
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Grid|Class Defaults")
	FName GemSocket;


	//Grid Params #############################################################################################


	//Initialize the grid on begin Play?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Grid|Grid Params")
	bool AutoInitGrid = true;

	//The grid size. x define the number of lanes and y the lane's length
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Grid|Grid Params")
	FVector2D GridSize;

	//The gameplay mode of the grid.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Grid|Grid Params")
	TEnumAsByte<EGridGameplayMode> GameplayMode;

	//The Node size in the grid
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Grid|Grid Params")
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


	//Inputs #############################################################################################

	//The Default trace channel
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Grid|Inputs")
	TEnumAsByte<ECollisionChannel> InputTraceChannel;

	//The DIstance for the Input trace
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Grid|Inputs")
	float InputTraceDistance;

#pragma endregion

#pragma region Caches

protected:
	// The Lanes currently in the grid.
	UPROPERTY()
	TArray<UPuzzleLaneComponent*> _lanesInGrid;

	// The array of every gems
	UPROPERTY()
	TArray<APuzzleGem*> _gemsAll;

	// The map of gem present in grid.
	UPROPERTY()
	TMap<FVector2D, APuzzleGem*> _gemsInGrid;

	// The array of gems waiting to be destroyed
	UPROPERTY()
	TArray<APuzzleGem*> _gemToBeDestroyed;

	// The array of gems in the recycler bin
	UPROPERTY()
	TArray<APuzzleGem*> _gemsRecyclerBin;

	//The last selected gem.
	UPROPERTY()
	APuzzleGem* _lastSelectedGem;

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
	TArray<FGemSwapHandler> _activeSwaps;

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
	
	//The history of swapped gems positions.
	UPROPERTY()
	TArray<FVector2D> _swapHistory;
	

#pragma endregion

#pragma region LifeSpan functions

public:
	//Initiaize the grid
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Life Time")
	void InitializeGrid(FVector2D grid_size, FVector2D node_size, TSubclassOf<UPuzzleLaneComponent> lane_class);

	//Reset the grid delting All Lanes and gems
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Life Time")
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

public:
	//Handle the inputs and return the swap couple
	UFUNCTION(BlueprintNativeEvent, Category="Puzzle Grid|Inputs")
	FGemSwapHandler HandleInputs();


	//Handle the inputs and return the swap couple (c++)
	virtual FGemSwapHandler HandleInputs_Implementation();

	//Handle swaps on the grid and update their states. returns match positions.
	void HandleSwapsOnGrid(FGemSwapHandler newSwap, TArray<FVector2D>& swapMatchPositions, float delta);

	//Add force to all nodes on the grid
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Inputs")
	void AddForce(FVector force);

	//Add radial force to nodes in range. maxIntensity is the force strength at the center.
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Inputs")
	void AddRadialForce(FVector center, float radius, float maxIntensity);
	
#pragma endregion


#pragma region Gem Query functions

public:
	//Get a gem on the grid by it's grid index.
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Query")
	APuzzleGem* GetGemAt(FVector2D grid_index);

	//Set a gem on the grid at an grid index.
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Query")
	void SetGemAt(FVector2D grid_index, APuzzleGem* gem);

	//Retrieve a gem from the recycler bin
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Query")
	APuzzleGem* GetRecycledGem(float deltaTime);

	//Handle the update for gem thet need to be deleted.
	void HandleGemToDelete(float delta);

	//Mark a gem as pending delete
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Gem Action")
	void DeleteGem(APuzzleGem* gem);

	//internaly delete gem and send it to the recycler bin
	bool DeleteGem_Internal(APuzzleGem* gem);

	//Check if a gem is waiting to be deleted by the grid
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Query")
	bool IsGemPendingDeletion(APuzzleGem* gem);

	//Called when a gem got destroyed on the grid
	UFUNCTION(BlueprintNativeEvent, Category="Puzzle Grid|Gem Action")
	void OnGemDeleted(APuzzleGem* gem);

	//Called when a gem got destroyed on the grid
	virtual void OnGemDeleted_Implementation(APuzzleGem* gem);

	//Called when a gem got Spawn on the grid
	UFUNCTION(BlueprintNativeEvent, Category="Puzzle Grid|Gem Action")
	void OnGemSpawned(APuzzleGem* gem, bool gridInitialization = false);

	//Called when a gem got destroyed on the grid
	virtual void OnGemSpawned_Implementation(APuzzleGem* gem, bool gridInitialization = false);

	//Handle the gem selection events
	void HandleSelected(FGemSwapHandler swap);

	//Override this event to customize the condition to spawn a new gem into the grid
	UFUNCTION(BlueprintNativeEvent)
	bool SpawnGemCondition(APuzzleGem* gemRequested);
	
	//Override this event to customize the condition to spawn a new gem into the grid
	virtual  bool SpawnGemCondition_Implementation(APuzzleGem* gemRequested);

#pragma endregion


#pragma region Node Query functions

public:
	//Get a node on the grid by it's grid index.
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Query")
	UPuzzleNodeComponent* GetNodeAt(FVector2D grid_index);

#pragma endregion

#pragma region Spatial functions

public:
	//Get the direction in which node where spawn. include distance from node to node
	UFUNCTION(BlueprintGetter, Category="Puzzle Grid|Query")
	FVector GetNodeDirection() const;

	//Get the direction in which Lane where spawn. include distance from lane to lane
	UFUNCTION(BlueprintGetter, Category="Puzzle Grid|Query")
	FVector GetLaneDirection() const;

#pragma endregion

	
#pragma region Matching Functions

public:

	//Check for a match around a position. returns true if a match was found with positions of that match.
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Match Making")
	bool CheckMatchAroundPosition(FVector2D position, TArray<FVector2D>& matchPositions);

	//Check matches in an aligned line. provide a temps position buffer to avoid GC and a Min Match Count for the minimum of gem aligned for a match to be valid.
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Match Making")
	bool CheckMatchesInLine(TArray<FVector2D> positions,TArray<FVector2D>& tempPositionBuffer, TArray<FGridMatch>& resultingMatches, int minPositionsCountForMatch = 3);

	//Compact horizontal and vertical matches into contigue matches
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Match Making")
	void CompactMatchesOnIntersections(TArray<FGridMatch>& resultingMatches);

	//Merge two matches and return the result with intersection position at the end of the match. true if Merging happens and false if not
	UFUNCTION(BlueprintCallable, Category="Puzzle Grid|Match Making")
	bool MergeMatches(FGridMatch& match_A, FGridMatch& match_B, FGridMatch& result);

	//Check if a match can be destroyed.
	UFUNCTION(BlueprintCallable,  Category="Puzzle Grid|Match Making")
	bool CanDestroyMatch(FGridMatch& match, TArray<FVector2D> exceptionList);
	
	//Called when a gem swap finnished. return true if the swap ended with a match.
	UFUNCTION(BlueprintNativeEvent, Category="Puzzle Grid|Match Making")
	void OnSwapEnded(bool swapResult, FVector2D fromPosition, FVector2D toPosition);
	
	//Called when a gem swap finnished. return true if the swap ended with a match. C++
	virtual void OnSwapEnded_Implementation(bool swapResult,FVector2D fromPosition, FVector2D toPosition);

	//Handle priorityMatches on the grid. responsible of gem destruction and transformation (LV up) if applicable.
	void HandleGridMatches(TArray<FVector2D>& exceptionPositions);

	//Update the gem swap history
	void UpdateSwapHistory(FVector2D gemPosition, bool removeOperation = false);
	
#pragma endregion
	

#pragma region Class Flow

public:
	// Sets default values for this component's properties
	UPuzzleGridComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

#pragma endregion
};

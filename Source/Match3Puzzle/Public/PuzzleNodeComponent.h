// Copyright © 2023 by Tyni Boat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#ifndef LANES
#define LANES
#include "PuzzleLaneComponent.h"
#endif

#include "PuzzleNodeComponent.generated.h"


//Puzzle Node, representing the place where the Gem must be located at. Cannot operate outside of a Puzzle Lane.
UCLASS(ClassGroup = (Match3Puzzle), BlueprintType, Blueprintable
	, hidecategories = (Object, LOD, Lighting, TextureStreaming, Velocity, PlanarMovement, MovementComponent, Tags,
		Activation, Cooking, AssetUserData, Collision, Physics, ComponentTick)
	, meta = (BlueprintSpawnableComponent))
class MATCH3PUZZLE_API UPuzzleNodeComponent : public USceneComponent
{
	GENERATED_BODY()


#pragma region Properties

public:
	//The node's grid position.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Puzzle Node|Node Params")
	FVector2D GridIndex = FVector2D(-1, -1);

	//Forces gem to be retrieve only from the grid.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Node|Node Params")
	bool IsGemFromGridOnly = false;

	//The delay to request a gem directly from grid if the node above refuses to let go it's gem
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Node|Node Params")
	float DelayGridDirectRequest = 3;

	//The node's grid position.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Node|Node Params")
	TEnumAsByte<EGridGemNodePopMethod> RequestGridGemMethod = popAtPosition;

	//The gem's movement easing function.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Node|Node Params")
	TEnumAsByte<EMoveEasingType> MoveEasingType;

#pragma endregion


#pragma region Cached

	//The location the gem was when movement started
	UPROPERTY()
	FVector _movementStartLocation;

	//The gem movement completion amount. from 0 to 1
	UPROPERTY()
	float _movementAmount;

	//The chrono to request a gem directly from grid if the node above refuses to let go it's gem
	UPROPERTY()
	float _chronoGridDirectRequest;

	//the time elapsed since the gem landed. used to delay OnLanding Call on ther gems.
	UPROPERTY()
	float _timeSinceLanding = -99;

	//The landing force.
	UPROPERTY()
	FVector _landingForce;
	
#pragma endregion


#pragma region internal functions

public:
	//Clear the lane and delete all nodes
	UFUNCTION(BlueprintCallable, Category="Puzzle Node|Life Time")
	void InitializeNode(UPuzzleLaneComponent* owner_lane, FVector2D node_index);

	//Clear the node and reinitialize values
	UFUNCTION(BlueprintCallable, Category="Puzzle Node|Life Time")
	void CLearNode();

	//Get the Parent Lane
	UFUNCTION(BlueprintCallable, Category="Puzzle Node|Query")
	UPuzzleLaneComponent* GetParentLane() const;

	//Moves the current gem to the node position
	UFUNCTION(BlueprintCallable, Category="Puzzle Node|Movement")
	void MoveGemToNode(float delta);

#pragma endregion

#pragma region Public functions

public:
	//Attach a gem to the node
	UFUNCTION(BlueprintCallable, Category="Puzzle Node|Life Time")
	void AttachGem(APuzzleGem* gem, bool fromGrid = false);

	//Detach a gem from a node
	UFUNCTION(BlueprintCallable, Category="Puzzle Node|Life Time")
	APuzzleGem* DetachGem(bool forced = false);

	//Request gem from lane
	UFUNCTION(BlueprintCallable, Category="Puzzle Node|Query")
	void RequestGemFromLane(float deltaTime);

	//Get the right easing function end it's result to ease a gem movement
	static float MoveByEasing(TEnumAsByte<EMoveEasingType> type, float inputValue);

	//Add an external force to the current gem.
	UFUNCTION(BlueprintCallable, Category="Puzzle Node|Movement")
	void AddImpulseForceToGem(FVector force);


	//Get get node index in a global direction. if direction is invalid, returns node self Index.
	UFUNCTION(BlueprintCallable, Category="Puzzle Node|Query")
	FVector2D GetNodeIndexInDirection(FVector direction);

	//Get custom gem spawn location relative to the node
	UFUNCTION(BlueprintCallable, Category="Puzzle Node|Query")
	FVector GetCustomGemSpawnLocation(FVector baseLocation);


#pragma endregion

#pragma region Caches

protected:
	// The Parent lane
	UPROPERTY()
	UPuzzleLaneComponent* _parentLane;

	// The current Gem
	UPROPERTY()
	APuzzleGem* _currentGem;

	// The last easing value from movement
	UPROPERTY()
	float _lastMovementEasingValue;

	// The current external push force
	UPROPERTY()
	FVector _externalPushForce;

#pragma endregion


#pragma region CLass Flow

public:
	// Sets default values for this component's properties
	UPuzzleNodeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

#pragma endregion


#pragma region Easing Move Types

public:
	//Linear easing function.
	static float LinearEase(float input);

	//ExpoOut easing function.
	static float ExpoOutEase(float input, float amount);

	//SineOut easing function.
	static float SineOutEase(float input);

	//BackOut easing function.
	static float BackOutEase(float input, float amount);

	//BounceOut easing function.
	static float BounceOutEase(float input);


#pragma endregion
};

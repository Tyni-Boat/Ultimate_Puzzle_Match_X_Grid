// Copyright © 2023 by Tyni Boat. All Rights Reserved.


#include "PuzzleLaneComponent.h"


#pragma region Internal Functions


void UPuzzleLaneComponent::CLearLane()
{
	//Delete Lanes
	for (int i = _nodesInLane.Num() - 1; i >= 0; i--)
	{
		if (_nodesInLane[i])
		{
			_nodesInLane[i]->CLearNode();
			_nodesInLane[i]->DestroyComponent();
		}
	}
	_nodesInLane.Empty();
}

void UPuzzleLaneComponent::InitializeLane(UPuzzleGridComponent* grid, int laneLength, int lane_index,
                                          EGridGemNodePopMethod innerGemPopMethod, bool popMethodForAll)
{
	if (!grid)
		return;
	_indexInGrid = lane_index;
	_directionToRecycler = grid->GetNodeDirection() * (laneLength + RecyclingZoneDistance);
	_grid = grid;
	for (int i = 0; i < laneLength; i++)
	{
		UPuzzleNodeComponent* instance = NewObject<UPuzzleNodeComponent>(this, NodeClass);
		instance->SetupAttachment(this);
		instance->RegisterComponent();
		instance->SetWorldLocation(GetComponentLocation() + grid->GetNodeDirection() * i);
		instance->InitializeNode(this, FVector2D(lane_index, i));
		if (i == 0)
			instance->RequestGridGemMethod = popMethodForAll? innerGemPopMethod : fromBeginOfLane;
		else if (i == (laneLength - 1))
			instance->RequestGridGemMethod = popMethodForAll? innerGemPopMethod : fromEndOfLane;
		else
			instance->RequestGridGemMethod = innerGemPopMethod;
		_nodesInLane.Add(instance);
	}
}

APuzzleGem* UPuzzleLaneComponent::GetGemCascade(int nodeYindex, float deltaTime, bool& fromGrid)
{
	int nextIndex = nodeYindex + 1;
	if (_nodesInLane.IsValidIndex(nextIndex))
	{
		fromGrid = false;
		return _nodesInLane[nextIndex]->DetachGem();
	}

	if (!_grid)
		return nullptr;

	auto gem = _grid->GetRecycledGem(deltaTime);
	if (!gem)
		return nullptr;
	fromGrid = true;
	gem->SetActorLocation(GetLaneRecyclerLocation());
	return gem;
}

void UPuzzleLaneComponent::SetGemInGrid(int nodeYindex, APuzzleGem* gem)
{
	if (!GetParentGrid())
		return;
	FVector2d index = FVector2d(_indexInGrid, nodeYindex);
	GetParentGrid()->SetGemAt(index, gem);
}

void UPuzzleLaneComponent::AddForce(FVector force)
{
	if(_nodesInLane.Num() <= 0)
		return;
	for(auto node:_nodesInLane)
	{
		if(!node)
			continue;
		node->AddImpulseForceToGem(force);
	}
}

void UPuzzleLaneComponent::AddRadialForce(FVector center, float radius, float maxIntensity)
{
	if(_nodesInLane.Num() <= 0)
		return;
	for(auto node:_nodesInLane)
	{
		if(!node)
			continue;
		FVector distance = node->GetComponentLocation() - center;
		if(distance.Length() >= radius)
			continue;
		FVector direction = distance.GetSafeNormal();
		auto inRange = TRange<float>(0, radius);
		auto outRange = TRange<float>(0, 1);
		float intensity = (1 - FMath::GetMappedRangeValueClamped(inRange, outRange, (float)direction.Length())) * maxIntensity;
		node->AddImpulseForceToGem(direction * intensity);
	}
}


#pragma endregion


#pragma region Class Flow

// Sets default values for this component's properties
UPuzzleLaneComponent::UPuzzleLaneComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPuzzleLaneComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UPuzzleLaneComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

#pragma endregion

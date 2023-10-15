// Copyright © 2023 by Tyni Boat. All Rights Reserved.


#include "../Public/PuzzleGridComponent.h"

#include "Kismet/KismetSystemLibrary.h"


#pragma region LifeSpan functions


void UPuzzleGridComponent::InitializeGrid(FVector2D grid_size, FVector2D node_size,
                                          TSubclassOf<UPuzzleLaneComponent> lane_class)
{
	//Clear the grid first
	ClearGrid();

	//Create Lanes
	{
		_nodeDistance = node_size.Y;
		_laneDistance = node_size.X;
		const FVector laneOffsetUnit = GetLaneDirection();
		const FVector nodeOffsetUnit = GetNodeDirection();
		const FVector firstLanePosition = GetComponentLocation()
			- (FMath::Clamp(grid_size.X - 1, 0, FMath::Abs(grid_size.X)) * 0.5f) * laneOffsetUnit
			- (FMath::Clamp(grid_size.Y - 1, 0, FMath::Abs(grid_size.Y)) * 0.5f) * nodeOffsetUnit;

		for (int i = 0; i < grid_size.X; i++)
		{
			UPuzzleLaneComponent* instance = NewObject<UPuzzleLaneComponent>(this, lane_class);
			instance->SetupAttachment(this);
			instance->RegisterComponent();
			instance->SetWorldLocation(firstLanePosition + laneOffsetUnit * i);
			bool popMethodAll = false;
			EGridGemNodePopMethod popMethod = GetNodePopMethodFromGridStrategy(FillingStrategy, popMethodAll);
			//Set inner gem fill method
			if (i == 0)
				popMethod = popMethodAll ? popMethod : fromLastLaneDirection;
			if (i == (grid_size.X - 1))
				popMethod = popMethodAll ? popMethod : fromNextLaneDirection;
			instance->InitializeLane(this, grid_size.Y, i, popMethod, popMethodAll);
			_lanesInGrid.Add(instance);
		}
	}

	//create Gems
	{
		_gemsInGrid.Empty();
		for (int i = 0; i < grid_size.X; i++)
		{
			for (int j = 0; j < grid_size.Y; j++)
			{
				//Create Gem
				APuzzleGem* Gem = GetWorld()->SpawnActor<APuzzleGem>(GemClass);
				if (!Gem)
					continue;
				Gem->parentGrid = this;
				Gem->SetActorLocation(GetComponentLocation());
				Gem->AttachToActor(GetOwner(), FAttachmentTransformRules::KeepWorldTransform, GemSocket);
				_gemsAll.AddUnique(Gem);
				OnGemSpawned(Gem, true);
				DeleteGem_Internal(Gem);


				//Fit gem map
				_gemsInGrid.Add(FVector2D(i, j), nullptr);
			}
		}
	}

	//Emit Init event
	OnGridInit();
}

void UPuzzleGridComponent::ClearGrid()
{
	//Delete Lanes
	for (int i = _lanesInGrid.Num() - 1; i >= 0; i--)
	{
		if (_lanesInGrid[i])
		{
			_lanesInGrid[i]->CLearLane();
			_lanesInGrid[i]->DestroyComponent();
		}
	}
	_lanesInGrid.Empty();

	//Delete Gems
	for (int i = _gemsAll.Num() - 1; i >= 0; i--)
	{
		if (_gemsAll[i])
		{
			_gemsAll[i]->Destroy();
		}
	}
	_gemsAll.Empty();
	_gemsInGrid.Empty();
	_gemsRecyclerBin.Empty();
}

void UPuzzleGridComponent::OnGridInit_Implementation()
{
}

EGridGemNodePopMethod UPuzzleGridComponent::GetNodePopMethodFromGridStrategy(
	EGridFillingStrategy strategy, bool& forall)
{
	switch (strategy)
	{
	case NoStrategy:
		forall = false;
		return fromGridInverseNormal;
	case EveryOnePop:
		forall = true;
		return popAtPosition;
	case InnerOnlyPop:
		forall = false;
		return popAtPosition;
	case EveryOneNormal:
		forall = true;
		return fromGridNormal;
	case InnerOnlyNormal:
		forall = false;
		return fromGridNormal;
	case EveryOneNormalInverse:
		forall = true;
		return fromGridInverseNormal;
	case InnerOnlyNormalInverse:
		forall = false;
		return fromGridInverseNormal;
	default:
		forall = false;
		return fromGridInverseNormal;
	}
}

float UPuzzleGridComponent::GetTimeScale() const
{
	return _gridTimeScale;
}

void UPuzzleGridComponent::SetTimeScale(float timeScale)
{
	_gridTimeScale = timeScale;
	if (_gemsAll.Num() <= 0)
		return;
	for (int i = 0; i < _gemsAll.Num(); i++)
	{
		if (!_gemsAll[i])
			continue;
		_gemsAll[i]->CustomTimeDilation = _gridTimeScale;
	}
}

#pragma endregion


#pragma region Interaction functions


FGemSwapHandler UPuzzleGridComponent::HandleInputs_Implementation()
{
	return FGemSwapHandler();
}

void UPuzzleGridComponent::HandleSwapsOnGrid(FGemSwapHandler newSwap, TArray<FVector2D>& swapMatchPositions,
                                             float delta)
{
	//Add the new swap
	if (newSwap.IsValid())
	{
		const int activeSwapIndex = _activeSwaps.IndexOfByPredicate([newSwap](FGemSwapHandler innerSwap) -> bool
		{
			return innerSwap.GemA == newSwap.GemA || innerSwap.GemB == newSwap.GemA || innerSwap.GemA == newSwap.GemB ||
				innerSwap.GemB == newSwap.GemB;
		});
		if (activeSwapIndex < 0)
		{
			_activeSwaps.Add(newSwap);
			UpdateSwapHistory(newSwap.GemA->GridIndex);
			UpdateSwapHistory(newSwap.GemB->GridIndex);
		}
	}

	if (_activeSwaps.Num() <= 0)
		return;

	for (int i = _activeSwaps.Num() - 1; i >= 0; i--)
	{
		//Handle invalid swaps
		if (!_activeSwaps[i].IsValid())
		{
			if (_activeSwaps[i].GemA)
				_activeSwaps[i].GemA->GemState = falling;
			if (_activeSwaps[i].GemB)
				_activeSwaps[i].GemB->GemState = falling;
			_activeSwaps.RemoveAt(i);
			continue;
		}
		const auto NodeA = GetNodeAt(_activeSwaps[i].GemA->GridIndex);
		const auto NodeB = GetNodeAt(_activeSwaps[i].GemB->GridIndex);
		if (!NodeA || !NodeB)
		{
			if (_activeSwaps[i].GemA)
				_activeSwaps[i].GemA->GemState = falling;
			if (_activeSwaps[i].GemB)
				_activeSwaps[i].GemB->GemState = falling;
			_activeSwaps.RemoveAt(i);
			continue;
		}

		//Handle Ended Swapped
		if (_activeSwaps[i].swapCompletion >= 1)
		{
			_activeSwaps[i].GemA->GemState = falling;
			_activeSwaps[i].GemB->GemState = falling;
			NodeA->_movementStartLocation = _activeSwaps[i].GemA->GetActorLocation();
			NodeA->_movementAmount = 1;
			NodeB->_movementStartLocation = _activeSwaps[i].GemB->GetActorLocation();
			NodeB->_movementAmount = 1;
			TArray<FVector2D> matchesPositions;
			bool A_match = CheckMatchAroundPosition(_activeSwaps[i].GemA->GridIndex, matchesPositions);
			bool B_match = CheckMatchAroundPosition(_activeSwaps[i].GemB->GridIndex, matchesPositions);

			//No match? Swap back
			if (!B_match && !A_match && _activeSwaps[i].isUserMadeSwap)
			{
				_activeSwaps.Add(FGemSwapHandler(_activeSwaps[i].GemB, _activeSwaps[i].GemA));
				UpdateSwapHistory(_activeSwaps[i].GemB->GridIndex, true);
				UpdateSwapHistory(_activeSwaps[i].GemA->GridIndex, true);
			}

			//Harvest the matching positions
			if ((A_match || B_match) && matchesPositions.Num() > 0)
			{
				for (auto pos : matchesPositions)
					swapMatchPositions.AddUnique(pos);
			}
			OnSwapEnded(A_match || B_match, _activeSwaps[i].GemB->GridIndex, _activeSwaps[i].GemA->GridIndex);
			_activeSwaps.RemoveAt(i);
			continue;
		}

		//If swap just began
		if (_activeSwaps[i].swapCompletion <= 0)
		{
			NodeA->DetachGem(true);
			NodeB->DetachGem(true);
			NodeA->AttachGem(_activeSwaps[i].GemB);
			NodeB->AttachGem(_activeSwaps[i].GemA);
			_activeSwaps[i].GemA->GemState = swapping;
			_activeSwaps[i].GemB->GemState = swapping;
			_activeSwaps[i].swapCompletion += delta;
			continue;
		}

		//Handle state
		_activeSwaps[i].GemA->GemState = swapping;
		_activeSwaps[i].GemB->GemState = swapping;

		//Handle movement
		if (_activeSwaps[i].CustomPath)
		{
			//_activeSwaps[i].CustomPath.point
		}
		const FVector positionA = FMath::Lerp(NodeB->GetComponentLocation(), NodeA->GetComponentLocation(),
		                                      NodeA->MoveByEasing(NodeA->MoveEasingType,
		                                                          _activeSwaps[i].swapCompletion));
		_activeSwaps[i].GemA->SetActorLocation(positionA);
		const FVector positionB = FMath::Lerp(NodeA->GetComponentLocation(), NodeB->GetComponentLocation(),
		                                      NodeB->MoveByEasing(NodeB->MoveEasingType,
		                                                          _activeSwaps[i].swapCompletion));
		_activeSwaps[i].GemB->SetActorLocation(positionB);

		_activeSwaps[i].swapCompletion += delta * SwapSpeed * GetTimeScale();
	}
}

void UPuzzleGridComponent::AddForce(FVector force)
{
	if (_lanesInGrid.Num() <= 0)
		return;
	for (auto lane : _lanesInGrid)
	{
		if (!lane)
			continue;
		lane->AddForce(force);
	}
}

void UPuzzleGridComponent::AddRadialForce(FVector center, float radius, float maxIntensity)
{
	if (_lanesInGrid.Num() <= 0)
		return;
	for (auto lane : _lanesInGrid)
	{
		if (!lane)
			continue;
		lane->AddRadialForce(center, radius, maxIntensity);
	}
}

#pragma endregion


#pragma region Gem Query functions


APuzzleGem* UPuzzleGridComponent::GetGemAt(FVector2D grid_index)
{
	if (_gemsInGrid.Contains(grid_index))
		return _gemsInGrid[grid_index];
	return nullptr;
}

void UPuzzleGridComponent::SetGemAt(FVector2D grid_index, APuzzleGem* gem)
{
	if (_gemsInGrid.Contains(grid_index))
		_gemsInGrid[grid_index] = gem;
}

APuzzleGem* UPuzzleGridComponent::GetRecycledGem(float deltaTime)
{
	if (_gemsRecyclerBin.Num() <= 0)
		return nullptr;
	const auto gem = _gemsRecyclerBin[_gemsRecyclerBin.Num() - 1];
	if (!SpawnGemCondition(gem))
		return nullptr;
	gem->UpdateGemVelocity(deltaTime);
	gem->OnGotSpawn_Internal();
	OnGemSpawned(gem, false);
	_gemsRecyclerBin.RemoveAt(_gemsRecyclerBin.Num() - 1);
	gem->GemState = EGemState::none;
	return gem;
}

void UPuzzleGridComponent::HandleGemToDelete(float delta)
{
	if (_gemToBeDestroyed.Num() <= 0)
		return;
	for (int i = _gemToBeDestroyed.Num() - 1; i >= 0; i--)
	{
		if (_gemToBeDestroyed[i]->UpdateGemDeletionChrono(delta * GetTimeScale()))
		{
			if (DeleteGem_Internal(_gemToBeDestroyed[i]))
			{
				_gemToBeDestroyed.RemoveAt(i);
			}
		}
	}
}

void UPuzzleGridComponent::DeleteGem(APuzzleGem* gem)
{
	if (gem && !_gemToBeDestroyed.Contains(gem))
	{
		if (!gem->CanDeleteGem())
			return;
		_gemToBeDestroyed.AddUnique(gem);
		gem->OnMarkedForDestroy();
		gem->GemState = EGemState::pendingDeletion;
	}
}

bool UPuzzleGridComponent::DeleteGem_Internal(APuzzleGem* gem)
{
	if (!gem)
		return false;
	if (_gemsRecyclerBin.Contains(gem))
		return false;

	UpdateSwapHistory(gem->GridIndex, true);
	_gemsRecyclerBin.AddUnique(gem);
	SetGemAt(gem->GridIndex, nullptr);
	OnGemDeleted(gem);
	gem->OnGotDeleted_Internal();
	if (_lanesInGrid.IsValidIndex(gem->GridIndex.X))
	{
		if (const auto node = _lanesInGrid[gem->GridIndex.X]->GetNodeAtIndex(gem->GridIndex.Y))
		{
			node->DetachGem(true);
		}
	}
	return true;
}

bool UPuzzleGridComponent::IsGemPendingDeletion(APuzzleGem* gem)
{
	if (!gem)
		return false;
	if (_gemToBeDestroyed.Num() <= 0)
		return false;
	return _gemToBeDestroyed.Contains(gem);
}

void UPuzzleGridComponent::OnGemDeleted_Implementation(APuzzleGem* gem)
{
}

void UPuzzleGridComponent::OnGemSpawned_Implementation(APuzzleGem* gem, bool gridInitialization)
{
}

void UPuzzleGridComponent::HandleSelected(FGemSwapHandler swap)
{
	if (_lastSelectedGem != swap.GemA)
	{
		if (_lastSelectedGem && _lastSelectedGem->GemState == EGemState::selected)
		{
			_lastSelectedGem->OnGotSelectionReleased();
			_lastSelectedGem->GemState = EGemState::falling;
		}
		if (swap.GemA && swap.GemA->GemState == EGemState::idle && swap.GemA->CanSelectGem())
		{
			swap.GemA->OnGotSelected();
			swap.GemA->GemState = EGemState::selected;
		}

		_lastSelectedGem = swap.GemA;
	}
}

bool UPuzzleGridComponent::SpawnGemCondition_Implementation(APuzzleGem* gemRequested)
{
	return true;
}

#pragma endregion


#pragma region Node Query functions

UPuzzleNodeComponent* UPuzzleGridComponent::GetNodeAt(FVector2D grid_index)
{
	if (!_lanesInGrid.IsValidIndex(grid_index.X))
		return nullptr;
	return _lanesInGrid[grid_index.X]->GetNodeAtIndex(grid_index.Y);
}


#pragma endregion


#pragma region Spatial functions


FVector UPuzzleGridComponent::GetNodeDirection() const
{
	return GetComponentRotation().Quaternion().GetForwardVector() * _nodeDistance;
}

FVector UPuzzleGridComponent::GetLaneDirection() const
{
	return GetComponentRotation().Quaternion().GetRightVector() * _laneDistance;
}


#pragma endregion


#pragma region Matching Functions


bool UPuzzleGridComponent::CheckMatchAroundPosition(FVector2D position, TArray<FVector2D>& matchPositions)
{
	auto positionGem = GetGemAt(position);
	if (!positionGem)
		return false;
	int sizeBefore = matchPositions.Num();
	//Check horizontal
	{
		int countHorizontal = 0;
		//Check right
		for (int i = position.X + 1; i < GridSize.X; i++)
		{
			auto gem = GetGemAt(FVector2D(i, position.Y));
			if (!gem)
				break;
			if (!gem->CanMatchGem())
				break;
			if (!positionGem->CompareGemTo(gem))
				break;
			countHorizontal++;
			matchPositions.AddUnique(FVector2D(i, position.Y));
		}
		//Check Left
		for (int i = position.X - 1; i >= 0; i--)
		{
			auto gem = GetGemAt(FVector2D(i, position.Y));
			if (!gem)
				break;
			if (!gem->CanMatchGem())
				break;
			if (!positionGem->CompareGemTo(gem))
				break;
			countHorizontal++;
			matchPositions.AddUnique(FVector2D(i, position.Y));
		}
		//Not enough matches?
		if (countHorizontal < 2)
		{
			for (int i = countHorizontal - 1; i >= 0; i--)
			{
				if (matchPositions.Num() <= 0)
					break;
				matchPositions.RemoveAt(matchPositions.Num() - 1);
			}
		}
	}

	//Check vertical
	{
		int countVertical = 0;
		//Check up
		for (int i = position.Y + 1; i < GridSize.Y; i++)
		{
			auto gem = GetGemAt(FVector2D(position.X, i));
			if (!gem)
				break;
			if (!gem->CanMatchGem())
				break;
			if (!positionGem->CompareGemTo(gem))
				break;
			countVertical++;
			matchPositions.AddUnique(FVector2D(position.X, i));
		}
		//Check Down
		for (int i = position.Y - 1; i >= 0; i--)
		{
			auto gem = GetGemAt(FVector2D(position.X, i));
			if (!gem)
				break;
			if (!gem->CanMatchGem())
				break;
			if (!positionGem->CompareGemTo(gem))
				break;
			countVertical++;
			matchPositions.AddUnique(FVector2D(position.X, i));
		}
		//Not enough matches?
		if (countVertical < 2)
		{
			for (int i = countVertical - 1; i >= 0; i--)
			{
				if (matchPositions.Num() <= 0)
					break;
				matchPositions.RemoveAt(matchPositions.Num() - 1);
			}
		}
	}

	//Finally
	if (FMath::Abs(sizeBefore - matchPositions.Num()) >= 2)
	{
		matchPositions.AddUnique(position);
		return true;
	}
	return false;
}


bool UPuzzleGridComponent::CheckMatchesInLine(TArray<FVector2D> positions, TArray<FVector2D>& tempPositionBuffer,
                                              TArray<FGridMatch>& resultingMatches, int minPositionsCountForMatch)
{
	if (positions.Num() <= 0)
		return false;
	const int matchesCountOnStart = resultingMatches.Num();
	int startMatchIndex = -1;
	TArray<FVector2D> tempPositionArray;
	for (int i = 1; i < positions.Num(); i++)
	{
		const auto last_gem = GetGemAt(positions[i - 1]);
		const auto gem = GetGemAt(positions[i]);

		//Check last gem could be match
		if (!last_gem || !last_gem->CanMatchGem())
		{
			//Collect - 1
			if (startMatchIndex >= 0 && FMath::Abs(i - startMatchIndex) > (minPositionsCountForMatch - 1))
			{
				tempPositionBuffer.Empty();
				for (int j = startMatchIndex; j < i; j++)
					tempPositionBuffer.Add(positions[j]);
				resultingMatches.Add(FGridMatch(tempPositionBuffer, _swapHistory));
			}
			startMatchIndex = -1;
			continue;
		}

		//Check gem if valid
		if (!gem)
		{
			//Collect - 1
			if (startMatchIndex >= 0 && FMath::Abs(i - startMatchIndex) > (minPositionsCountForMatch - 1))
			{
				tempPositionBuffer.Empty();
				for (int j = startMatchIndex; j < i; j++)
					tempPositionBuffer.Add(positions[j]);
				resultingMatches.Add(FGridMatch(tempPositionBuffer, _swapHistory));
			}
			startMatchIndex = -1;
			continue;
		}


		//Check gem could be match
		if (!gem->CanMatchGem())
		{
			//Collect - 1
			if (startMatchIndex >= 0 && FMath::Abs(i - startMatchIndex) > (minPositionsCountForMatch - 1))
			{
				tempPositionBuffer.Empty();
				for (int j = startMatchIndex; j < i; j++)
					tempPositionBuffer.Add(positions[j]);
				resultingMatches.Add(FGridMatch(tempPositionBuffer, _swapHistory));
			}
			startMatchIndex = -1;
			continue;
		}

		//Check last and current gem's equatables
		if (!last_gem->CompareGemTo(gem))
		{
			//Collect - 1
			if (startMatchIndex >= 0 && FMath::Abs(i - startMatchIndex) > (minPositionsCountForMatch - 1))
			{
				tempPositionBuffer.Empty();
				for (int j = startMatchIndex; j < i; j++)
					tempPositionBuffer.Add(positions[j]);
				resultingMatches.Add(FGridMatch(tempPositionBuffer, _swapHistory));
			}
			startMatchIndex = -1;
			continue;
		}

		//Equatable test passed
		if (startMatchIndex < 0) //Set start index at last gem one if invalid index
			startMatchIndex = i - 1;

		//Collect for the last item
		if (i >= (positions.Num() - 1))
		{
			//Collect full
			if (startMatchIndex >= 0 && FMath::Abs(i - startMatchIndex) >= (minPositionsCountForMatch - 1))
			{
				tempPositionBuffer.Empty();
				for (int j = startMatchIndex; j <= i; j++)
					tempPositionBuffer.Add(positions[j]);
				resultingMatches.Add(FGridMatch(tempPositionBuffer, _swapHistory));
			}
		}
	}

	return FMath::Abs(resultingMatches.Num() - matchesCountOnStart) > 0;
}


void UPuzzleGridComponent::CompactMatchesOnIntersections(TArray<FGridMatch>& resultingMatches)
{
	if (resultingMatches.Num() <= 0)
		return;

	//Check intersection matches
	{
		const int currentLenght = resultingMatches.Num();
		for (int i = currentLenght - 1; i >= 0; i--)
		{
			for (int j = currentLenght - 1; j >= 0; j--)
			{
				if (i == j)
					continue;
				FGridMatch intersectMatch;
				if (MergeMatches(resultingMatches[i], resultingMatches[j], intersectMatch))
				{
					resultingMatches.Add(intersectMatch);
				}
			}
		}
	}

	//Remove empty matches
	{
		for (int i = resultingMatches.Num() - 1; i >= 0; i--)
		{
			if (resultingMatches[i].IsEmpty())
				resultingMatches.RemoveAt(i);
		}
	}
}


bool UPuzzleGridComponent::MergeMatches(FGridMatch& match_A, FGridMatch& match_B, FGridMatch& result)
{
	if (match_A.IsEmpty() || match_B.IsEmpty())
		return false;
	const auto intersection = match_A.Intersect(match_B);
	if (intersection.X < 0 || intersection.Y < 0)
		return false;
	const int index_A = (int)intersection.X;
	const int index_B = (int)intersection.Y;
	if (!match_A.MatchPositions.IsValidIndex(index_A))
		return false;
	if (!match_B.MatchPositions.IsValidIndex(index_B))
		return false;
	result.Clear();
	const auto intersectionPos = match_A.MatchPositions[index_A];
	match_A.MatchPositions.RemoveAt(index_A);
	match_B.MatchPositions.RemoveAt(index_B);
	for (auto pos : match_A.MatchPositions)
		result.MatchPositions.AddUnique(pos);
	for (auto pos : match_B.MatchPositions)
		result.MatchPositions.AddUnique(pos);
	result.MatchPositions.AddUnique(intersectionPos);
	match_A.Clear();
	match_B.Clear();
	return true;
}


bool UPuzzleGridComponent::CanDestroyMatch(FGridMatch& match, TArray<FVector2D> exceptionList)
{
	if (match.MatchPositions.Num() <= 0)
		return false;
	for (auto position : match.MatchPositions)
	{
		auto gem = GetGemAt(position);
		if (!gem)
			continue;
		if (!exceptionList.Contains(position) && gem->GemState != idle)
			return false;
	}

	return true;
}


void UPuzzleGridComponent::OnSwapEnded_Implementation(bool swapResult, FVector2D fromPosition, FVector2D toPosition)
{
}


void UPuzzleGridComponent::HandleGridMatches(TArray<FVector2D>& exceptionPositions)
{
	_allGridMatches.Empty();

	//Search grid for matches

	//Vertical Matches
	{
		for (int i = 0; i < GridSize.X; i++)
		{
			//collect line
			_multiPurposePositionBuffer_1.Empty();
			for (int j = 0; j < GridSize.Y; j++)
			{
				FVector2D position = FVector2D(i, j);
				_multiPurposePositionBuffer_1.AddUnique(position);
			}

			//Check matches in line
			CheckMatchesInLine(_multiPurposePositionBuffer_1, _multiPurposePositionBuffer_2, _allGridMatches,
			                   MinMatchCount);
		}
	}

	//Horizontal Matches
	{
		for (int i = 0; i < GridSize.Y; i++)
		{
			//collect line
			_multiPurposePositionBuffer_1.Empty();
			for (int j = 0; j < GridSize.X; j++)
			{
				FVector2D position = FVector2D(j, i);
				_multiPurposePositionBuffer_1.AddUnique(position);
			}

			//Check matches in line
			CheckMatchesInLine(_multiPurposePositionBuffer_1, _multiPurposePositionBuffer_2, _allGridMatches,
			                   MinMatchCount);
		}
	}

	//Handle intersections
	CompactMatchesOnIntersections(_allGridMatches);

	//Destroy Matches
	{
		exceptionPositions.Empty();
		if (_allGridMatches.Num() <= 0)
			return;
		for (auto match : _allGridMatches)
		{
			if (!CanDestroyMatch(match, exceptionPositions))
				continue;
			for (int i = 0; i < match.MatchPositions.Num(); i++)
			{
				const auto gem = GetGemAt(match.MatchPositions[i]);
				if (!gem)
					continue;
				const int matchCount = match.MatchPositions.Num();
				const bool intersection = i == (match.MatchPositions.Num() - 1) && matchCount > MinMatchCount;
				if (gem->AvoidDestroyOnGemMatching(matchCount, intersection))
					continue;
				DeleteGem(gem);
			}
		}
	}
}


void UPuzzleGridComponent::UpdateSwapHistory(FVector2D gemPosition, bool removeOperation)
{
	int indexInHistory = _swapHistory.IndexOfByPredicate([gemPosition](FVector2D historyItem)-> bool
	{
		return historyItem == gemPosition;
	});

	if (!_swapHistory.IsValidIndex(indexInHistory))
	{
		if (!removeOperation && gemPosition.X >= 0 && gemPosition.Y >= 0)
			_swapHistory.Add(gemPosition);
		return;
	}

	if (!removeOperation)
		return;

	_swapHistory.RemoveAt(indexInHistory);
}

#pragma endregion


#pragma region CLass Flow

// Sets default values for this component's properties
UPuzzleGridComponent::UPuzzleGridComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPuzzleGridComponent::BeginPlay()
{
	Super::BeginPlay();

	// CLear and Initialize the grid
	if (AutoInitGrid)
	{
		InitializeGrid(GridSize, NodeSize, LaneClass);
	}
}


// Called every frame
void UPuzzleGridComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	auto gemSwap = HandleInputs();
	if (gemSwap.IsValid())
		gemSwap.isUserMadeSwap = true;
	HandleSelected(gemSwap);
	switch (GameplayMode)
	{
	case ClickAndDestroy:
		{
			DeleteGem(gemSwap.GemA);
		}
		break;
	case SwapGemAndMatch:
		{
			HandleSwapsOnGrid(gemSwap, _swapGridPositionExceptions, DeltaTime);
			HandleGridMatches(_swapGridPositionExceptions);
		}
		break;
	default:
		{
			DeleteGem(gemSwap.GemA);
		}
		break;
	}
	HandleGemToDelete(DeltaTime);
}

#pragma endregion

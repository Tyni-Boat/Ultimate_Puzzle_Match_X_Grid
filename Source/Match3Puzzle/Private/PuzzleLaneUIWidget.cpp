// Copyright © 2023 by Tyni Boat. All Rights Reserved.


#include "PuzzleLaneUIWidget.h"


#pragma region internal functions


float UPuzzleLaneUIWidget::InitializeLane(UPuzzleGridUIWidget* grid, int lane_index,
                                         EGridGemNodePopMethod innerGemPopMethod,
                                         bool popMethodForAll)
{
	if (!grid)
		return 0;
	_indexInGrid = lane_index;
	_grid = grid;
	_laneCenter = GetParentGrid()->GetWidgetCenterLocation(GetParentGrid(), this);
	FVector2D lastNodeLocation = _laneCenter;
	float nodeDistance = 0;
	GetParentGrid()->GetChildWidget(this, _nodesInLane);
	if(_nodesInLane.Num() > 0)
	{
		if(_nodesInLane.Num() > 1)
		{
			auto pos0 = GetParentGrid()->GetWidgetCenterLocation(this, _nodesInLane[0]);
			auto pos1 = GetParentGrid()->GetWidgetCenterLocation(this, _nodesInLane[1]);
			nodeDistance = (pos1 - pos0).Length();
		}
		
		for (int i = 0; i < _nodesInLane.Num(); i++)
		{
			// _nodesInLane[i]->InitializeNode(this, FVector2D(lane_index, i));
			if (i == 0)
			{
				// _nodesInLane[i]->RequestGridGemMethod = popMethodForAll ? innerGemPopMethod : fromBeginOfLane;
			}
			else if (i == (_nodesInLane.Num() - 1))
			{
				// _nodesInLane[i]->RequestGridGemMethod = popMethodForAll ? innerGemPopMethod : fromEndOfLane;
				lastNodeLocation = GetParentGrid()->GetWidgetCenterLocation(this, _nodesInLane[i]);
			}
			else
			{
				// _nodesInLane[i]->RequestGridGemMethod = innerGemPopMethod;
			}
		}
	}

	_directionToRecycler = (lastNodeLocation - _laneCenter) + FVector2D(0, 1) * RecyclingZoneDistance;
	return nodeDistance;
}

void UPuzzleLaneUIWidget::CLearLane()
{
	_nodesInLane.Empty();
}

UPuzzleGemUIWidget* UPuzzleLaneUIWidget::GetGemCascade(int nodeYindex, float deltaTime, bool& fromGrid)
{
	return nullptr;
}

void UPuzzleLaneUIWidget::SetGemInGrid(int nodeYindex, APuzzleGem* gem)
{
}

void UPuzzleLaneUIWidget::AddForce(FVector2D force)
{
}

void UPuzzleLaneUIWidget::AddRadialForce(FVector2D center, float radius, float maxIntensity)
{
}

#pragma endregion


#pragma region Class FLow

void UPuzzleLaneUIWidget::BeginPlay()
{
}

void UPuzzleLaneUIWidget::TickComponent(float DeltaTime)
{
}

#pragma endregion

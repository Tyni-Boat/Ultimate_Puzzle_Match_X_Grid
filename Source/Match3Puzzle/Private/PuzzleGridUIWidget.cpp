// Copyright © 2023 by Tyni Boat. All Rights Reserved.


#include "PuzzleGridUIWidget.h"

#include "Kismet/KismetSystemLibrary.h"


#pragma region LifeSpan functions


template <typename T>
void UPuzzleGridUIWidget::GetChildWidget(UPanelWidget* parent, TArray<T*>& children)
{
	if (!parent)
		return;
	auto widgets = parent->GetAllChildren();
	for (int i = 0; i < widgets.Num(); i++)
	{
		T* child = Cast<T>(widgets[i]);
		if (child)
		{
			children.Add(child);
		}
	}
}


FVector2D UPuzzleGridUIWidget::GetWidgetCenterLocation(const UWidget* parent, const UWidget* Widget)
{
	if (!parent || !Widget)
		return FVector2D::Zero();

	const FGeometry Geometry = GetCachedGeometry();// parent->GetCachedGeometry();
	const auto absolutePos = Widget->GetCachedGeometry().GetAbsolutePosition();
	const FVector2D Position = Geometry.AbsoluteToLocal(absolutePos);// + Widget->
	                                                                                                         //GetCachedGeometry()
	                                                                                                         //.GetLocalSize()
																											//	/ 2.0f;
	return Position;
}


void UPuzzleGridUIWidget::InitializeGrid()
{
	//Clear the grid first
	ClearGrid();

	//Create Lanes
	{
		//Get children widgets of type lane
		GetChildWidget(this, _lanesInGrid);
		if (_lanesInGrid.Num() <= 0)
			return;

		//Lane distance is the distance between the two last lanes. in case of one lane, is the width of the lane.
		_laneDistance = _lanesInGrid.Num() > 1
			                     ? (GetWidgetCenterLocation(this, Cast<UWidget>(_lanesInGrid[1])).X -
				                     GetWidgetCenterLocation(
					                     this, Cast<UWidget>(_lanesInGrid[0])).X)
			                     : 0;

		//Node distance is the reported distance between two nodes on the first lane
		for (int i = 0; i < _lanesInGrid.Num(); i++)
		{
			bool popMethodAll = false;
			EGridGemNodePopMethod popMethod = GetNodePopMethodFromGridStrategy(FillingStrategy, popMethodAll);
			//Set inner gem fill method
			if (i == 0)
			{
				popMethod = popMethodAll ? popMethod : fromLastLaneDirection;
			}
			if (i == (_lanesInGrid.Num() - 1))
			{
				popMethod = popMethodAll ? popMethod : fromNextLaneDirection;
			}
			float nodeDistance = _lanesInGrid[i]->InitializeLane(this, i);
			if (i == 0)
			{
				_nodeDistance = nodeDistance;
			}
		}
	}

	////create Gems
	//{
	//	//Node count gems must be created
	//	//Gems must not be children of the grid cuz it's an horizontal layout. add them directly to the viewport

	//	_gemsInGrid.Empty();
	//	for (int i = 0; i < grid_size.X; i++)
	//	{
	//		for (int j = 0; j < grid_size.Y; j++)
	//		{
	//			//Create Gem
	//			UPuzzleGemUIWidget* Gem = GetWorld()->SpawnActor<UPuzzleGemUIWidget>(GemClass);
	//			if (!Gem)
	//				continue;
	//			Gem->parentGrid = this;
	//			Gem->SetActorLocation(GetComponentLocation());
	//			Gem->AttachToActor(GetOwner(), FAttachmentTransformRules::KeepWorldTransform, GemSocket);
	//			_gemsAll.AddUnique(Gem);
	//			OnGemSpawned(Gem, true);
	//			DeleteGem_Internal(Gem);


	//			//Fit gem map
	//			_gemsInGrid.Add(FVector2D(i, j), nullptr);
	//		}
	//	}
	//}

	//Emit Init event
	OnGridInit();
}

void UPuzzleGridUIWidget::ClearGrid()
{
	////Delete Lanes
	//for (int i = _lanesInGrid.Num() - 1; i >= 0; i--)
	//{
	//	if (_lanesInGrid[i])
	//	{
	//		_lanesInGrid[i]->CLearLane();
	//	}
	//}
	//_lanesInGrid.Empty();

	////Delete Gems
	//for (int i = _gemsAll.Num() - 1; i >= 0; i--)
	//{
	//	if (_gemsAll[i])
	//	{
	//		_gemsAll[i]->Destroy();
	//	}
	//}
	//_gemsAll.Empty();
	//_gemsInGrid.Empty();
	//_gemsRecyclerBin.Empty();
}

void UPuzzleGridUIWidget::OnGridInit_Implementation()
{
}

EGridGemNodePopMethod UPuzzleGridUIWidget::GetNodePopMethodFromGridStrategy(
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

float UPuzzleGridUIWidget::GetTimeScale() const
{
	return _gridTimeScale;
}

void UPuzzleGridUIWidget::SetTimeScale(float timeScale)
{
	//_gridTimeScale = timeScale;
	//if (_gemsAll.Num() <= 0)
	//	return;
	//for (int i = 0; i < _gemsAll.Num(); i++)
	//{
	//	if (!_gemsAll[i])
	//		continue;
	//	_gemsAll[i]->CustomTimeDilation = _gridTimeScale;
	//}
}


#pragma endregion


#pragma region Class Flow


void UPuzzleGridUIWidget::BeginPlay()
{
	InitializeGrid();
}

void UPuzzleGridUIWidget::Tick(float DeltaTime)
{
}


#pragma endregion

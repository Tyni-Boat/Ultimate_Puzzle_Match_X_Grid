// Copyright © 2023 by Tyni Boat. All Rights Reserved.


#include "PuzzleNodeComponent.h"


#pragma region internal functions


void UPuzzleNodeComponent::InitializeNode(UPuzzleLaneComponent* owner_lane, FVector2D node_index)
{
	_parentLane = owner_lane;
	GridIndex = node_index;
}

void UPuzzleNodeComponent::CLearNode()
{
	GridIndex = FVector2D(-1, -1);
	_parentLane = nullptr;
}

UPuzzleLaneComponent* UPuzzleNodeComponent::GetParentLane() const
{
	return _parentLane;
}

#pragma endregion

#pragma region Public functions

void UPuzzleNodeComponent::AttachGem(APuzzleGem* gem, bool fromGrid)
{
	if (!gem)
		return;
	if (!_parentLane)
		return;
	_currentGem = gem;
	_currentGem->SetGridIndex(GridIndex);
	_parentLane->SetGemInGrid(GridIndex.Y, _currentGem);
	_currentGem->GemState = EGemState::falling;
	//Set custom gem position depending on RequestGridGemMethod, when gem came from grid
	if (fromGrid)
	{
		FVector newPosition = GetCustomGemSpawnLocation(_currentGem->GetActorLocation());
		_currentGem->SetActorLocation(newPosition);
	}
	_movementStartLocation = _currentGem->GetActorLocation();
	_movementAmount = 0;
	_timeSinceLanding = -99;
}

APuzzleGem* UPuzzleNodeComponent::DetachGem(bool forced)
{
	if (!_parentLane)
		return nullptr;
	if (!_currentGem)
		return nullptr;
	if (!((_currentGem->GemState == EGemState::idle || _currentGem->GemState == EGemState::falling) && _currentGem->
		CanMoveGem()) && !forced)
		return nullptr;
	const auto gem = _currentGem;
	_parentLane->SetGemInGrid(GridIndex.Y, nullptr);
	_currentGem->SetGridIndex(FVector2D(-1, -1));
	_currentGem->GemState = EGemState::none;
	_currentGem = nullptr;
	return gem;
}

void UPuzzleNodeComponent::RequestGemFromLane(float deltaTime)
{
	if (!_parentLane)
		return;
	if (_currentGem)
		return;
	bool fromGrid = false;
	const auto gem = _parentLane->GetGemCascade(
		(_chronoGridDirectRequest >= DelayGridDirectRequest || IsGemFromGridOnly) ? -2 : GridIndex.Y,
		deltaTime, fromGrid);
	if (_chronoGridDirectRequest >= DelayGridDirectRequest)
		_chronoGridDirectRequest = 0;
	if (!gem)
	{
		if (_chronoGridDirectRequest < DelayGridDirectRequest && !IsGemFromGridOnly)
			_chronoGridDirectRequest += deltaTime;
		return;
	}

	_chronoGridDirectRequest = 0;
	AttachGem(gem, fromGrid);
}

void UPuzzleNodeComponent::MoveGemToNode(float delta)
{
	if (!_currentGem)
		return;

	if (!_currentGem->CanMoveGem())
	{
		if (_currentGem->GemState == EGemState::falling)
			_currentGem->GemState = EGemState::idle;
		_currentGem->SetActorLocation(GetComponentLocation());
		return;
	}

	//Move
	if (_currentGem->GemState == EGemState::falling)
	{
		_movementAmount += delta * _currentGem->GemSpeed * (_externalPushForce.Length() > 0 ? 4 : 1);
		float movementEasing = _movementAmount;
		if (_externalPushForce.Length() <= 0)
		{
			movementEasing = MoveByEasing(MoveEasingType, _movementAmount);
			if (movementEasing >= 0.95f && _lastMovementEasingValue < 0.95f)
			{
				_landingForce = _currentGem->GemVelocity * 5 * delta;
				_timeSinceLanding = _currentGem->LandingDelay;
			}
		}
		else
		{
			movementEasing = SineOutEase(_movementAmount);
			if (movementEasing > 0 && _lastMovementEasingValue <= 0)
			{
				_landingForce = _externalPushForce;
				_timeSinceLanding = _currentGem->LandingDelay;
			}
		}

		_lastMovementEasingValue = movementEasing;
		const FVector position = FMath::Lerp(_movementStartLocation
		                                     , GetComponentLocation() + _externalPushForce, movementEasing);
		_currentGem->SetActorLocation(position);
		if (_movementAmount >= 1)
		{
			_movementAmount = 1;
			_currentGem->SetActorLocation(GetComponentLocation() + _externalPushForce);
			_currentGem->GemState = EGemState::idle;
			_lastMovementEasingValue = 0;
			if (_externalPushForce.Length() > 0)
				_externalPushForce = FVector::ZeroVector;
		}
	}
	else if (_currentGem->GemState == EGemState::idle)
	{
		if ((_currentGem->GetActorLocation() - GetComponentLocation()).SquaredLength() > 1)
		{
			_currentGem->GemState = EGemState::falling;
			_movementStartLocation = _currentGem->GetActorLocation();
			_lastMovementEasingValue = 0;
			_movementAmount = 0;
		}
	}

	//Gem landing and delay
	if (_currentGem && _timeSinceLanding <= 0 && (int)_timeSinceLanding != -99)
	{
		_currentGem->OnGemLanded(_parentLane->GetParentGrid()
		                         , GetNodeIndexInDirection(_landingForce)
		                         , _landingForce);
		_timeSinceLanding = -99;
	}
	else if (_timeSinceLanding > 0)
	{
		_timeSinceLanding -= delta;
	}
}

float UPuzzleNodeComponent::MoveByEasing(TEnumAsByte<EMoveEasingType> type, float inputValue)
{
	switch (type)
	{
	case EMoveEasingType::linear:
		return LinearEase(inputValue);
	case EMoveEasingType::outExpo:
		return ExpoOutEase(inputValue, 3);
	case EMoveEasingType::outSine:
		return SineOutEase(inputValue);
	case EMoveEasingType::outBack:
		return BackOutEase(inputValue, 3);
	case EMoveEasingType::outBounce:
		return BounceOutEase(inputValue);
	default:
		return LinearEase(inputValue);
	}
}

void UPuzzleNodeComponent::AddImpulseForceToGem(FVector force)
{
	if (!_currentGem)
		return;
	if (_currentGem->GemState == EGemState::idle)
	{
		_movementStartLocation = _currentGem->GetActorLocation();
		_lastMovementEasingValue = 0;
		_currentGem->GemState = EGemState::falling;
		_externalPushForce += force * _currentGem->ExternalForceTransferScale;
		_movementAmount = 0;
	}
}

FVector2D UPuzzleNodeComponent::GetNodeIndexInDirection(FVector direction)
{
	FVector dir = direction;
	if (!dir.Normalize() || !_parentLane || !_parentLane->GetParentGrid())
		return GridIndex;
	float Ycompound = GridIndex.Y;
	float Xcompound = GridIndex.X;

	float verticalDot = FVector::DotProduct(dir, _parentLane->GetParentGrid()->GetNodeDirection().GetSafeNormal());
	float horizontalDot = FVector::DotProduct(dir, _parentLane->GetParentGrid()->GetLaneDirection().GetSafeNormal());
	Ycompound = verticalDot > 0.5f ? GridIndex.Y + 1 : (verticalDot < -0.5f ? GridIndex.Y - 1 : Ycompound);
	Xcompound = horizontalDot > 0.5f ? GridIndex.X + 1 : (horizontalDot < -0.5f ? GridIndex.X - 1 : Xcompound);

	return FVector2D(Xcompound, Ycompound);
}

FVector UPuzzleNodeComponent::GetCustomGemSpawnLocation(FVector baseLocation)
{
	switch (RequestGridGemMethod)
	{
	default:
		return baseLocation;
	//normal
	case fromGridNormal:
		if (_parentLane && _parentLane->GetParentGrid())
			return GetComponentLocation() + FVector::CrossProduct(_parentLane->GetParentGrid()->GetNodeDirection(),
			                                                      _parentLane->GetParentGrid()->GetLaneDirection()).
				GetSafeNormal() * (_parentLane
				                   ->GetParentGrid()->GetLaneDirection().Length() + _parentLane->RecyclingZoneDistance);
		return baseLocation;
	//inverse normal
	case fromGridInverseNormal:
		if (_parentLane && _parentLane->GetParentGrid())
			return GetComponentLocation() - FVector::CrossProduct(_parentLane->GetParentGrid()->GetNodeDirection(),
			                                                      _parentLane->GetParentGrid()->GetLaneDirection()).
				GetSafeNormal() * (_parentLane
				                   ->GetParentGrid()->GetLaneDirection().Length() + _parentLane->RecyclingZoneDistance);
		return baseLocation;
	//begin lane
	case fromBeginOfLane:
		if (_parentLane && _parentLane->GetParentGrid())
		{
			auto startNode = _parentLane->GetParentGrid()->GetNodeAt(FVector2D(GridIndex.X, 0));
			if (startNode)
			{
				return startNode->GetComponentLocation() - (_parentLane->GetParentGrid()->GetNodeDirection() *
					_parentLane->RecyclingZoneDistance);
			}
			return baseLocation;
		}
		return baseLocation;
	//Pop
	case popAtPosition:
		return GetComponentLocation();
	//Last lane
	case fromLastLaneDirection:
		if (_parentLane && _parentLane->GetParentGrid())
			return GetComponentLocation() - (_parentLane->GetParentGrid()->GetLaneDirection() * _parentLane->
				RecyclingZoneDistance);
		return baseLocation;
	//Next lane
	case fromNextLaneDirection:
		if (_parentLane && _parentLane->GetParentGrid())
			return GetComponentLocation() + (_parentLane->GetParentGrid()->GetLaneDirection() * _parentLane->
				RecyclingZoneDistance);
		return baseLocation;
	}
}


#pragma endregion

#pragma region CLass Flow

// Sets default values for this component's properties
UPuzzleNodeComponent::UPuzzleNodeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPuzzleNodeComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UPuzzleNodeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	float delta = DeltaTime;
	//Grid scaled time
	if (_parentLane && _parentLane->GetParentGrid())
		delta = _parentLane->GetParentGrid()->GetTimeScale() * DeltaTime;
	RequestGemFromLane(delta);
	MoveGemToNode(delta);
}

#pragma endregion


#pragma region Easing Move Types


float UPuzzleNodeComponent::LinearEase(float input)
{
	return input;
}

float UPuzzleNodeComponent::ExpoOutEase(float input, float amount)
{
	return input == 1 ? 1 : 1 - FMath::Pow(2, -10 * input);
}

float UPuzzleNodeComponent::SineOutEase(float input)
{
	return FMath::Sin((input * 3.14f) * 0.5f);
}

float UPuzzleNodeComponent::BackOutEase(float input, float amount)
{
	const float c1 = 1.70158;
	const float c3 = c1 + 1;
	const float x = input - 1;
	return 1 + c3 * (x * x * x) + c1 * (x * x);
}

float UPuzzleNodeComponent::BounceOutEase(float input)
{
	const float n1 = 7.5625;
	const float d1 = 2.75;

	if (input < 1 / d1)
	{
		return n1 * input * input;
	}
	else if (input < 2 / d1)
	{
		float input_local = input;
		input_local -= 1.5 / d1;
		return n1 * (input_local) * input_local + 0.75;
	}
	else if (input < 2.5 / d1)
	{
		float input_local = input;
		input_local -= 2.25 / d1;
		return n1 * (input_local) * input_local + 0.9375;
	}
	else
	{
		float input_local = input;
		input_local -= 2.625 / d1;
		return n1 * (input_local) * input_local + 0.984375;
	}
}

#pragma endregion

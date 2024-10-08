// Copyright © 2023 by Tyni Boat. All Rights Reserved.


#include "PuzzleGem.h"
#include "UObject/Object.h"


#pragma region Properties Handling Functions

//Gem Indexing ################################################################################

void APuzzleGem::SetGridIndex(FVector2D grid_index)
{
	GridIndex = grid_index;
}


//Gem Spawning ################################################################################


void APuzzleGem::OnGotSpawn_Internal()
{
	_gemDeletionCountDownChrono = GemDeletionDelay > 0 ? GemDeletionDelay : 0.5f;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);
	OnGotSpawn();
}


void APuzzleGem::OnGotSpawn_Implementation()
{
}


//Gem Deletion ################################################################################


void APuzzleGem::OnMarkedForDestroy_Implementation()
{
}

void APuzzleGem::OnGotDeleted_Implementation()
{
}

void APuzzleGem::OnGotDeleted_Internal()
{
	OnGotDeleted();
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
	//Detach all attachment
	if (_attachmentList.Num() > 0)
	{
		for (int i = _attachmentList.Num() - 1; i >= 0; i--)
		{
			DetachFromGem(_attachmentList[i]);
		}
	}
}

bool APuzzleGem::UpdateGemDeletionChrono(float delta)
{
	_gemDeletionCountDownChrono -= delta;
	return _gemDeletionCountDownChrono <= 0;
}


//Gem Selection ################################################################################

void APuzzleGem::OnGotSelected_Implementation()
{
}

void APuzzleGem::OnGotSelectionReleased_Implementation()
{
}


//Gem inter-action #############################################################################


void APuzzleGem::OnGemLanded_Implementation(USceneComponent* onGrid, FVector2D indexGemLandedOn, FVector landingForce)
{
}


void APuzzleGem::UpdateGemVelocity(float delta)
{
	//Update velocity
	GemVelocity = (GetActorLocation() - _lastGemLocation) / delta;
	_lastGemLocation = GetActorLocation();
}


//Gem attachments #######################################################################


void APuzzleGem::AttachToGem(TScriptInterface<IPuzzleGemAttachment> attachment)
{
	if (!attachment.GetObject())
		return;
	attachment.SetInterface(reinterpret_cast<IPuzzleGemAttachment*>(attachment.GetObject()));
	if (!attachment.GetInterface())
		return;

	if (_attachmentList.Num() > 0)
	{
		const int cannotAddIndex = _attachmentList.IndexOfByPredicate([&](
			TScriptInterface<IPuzzleGemAttachment> attachment)
			{
				return !attachment->Execute_CanAttachToGem(attachment.GetObject(), this, attachment.GetObject());
			});
		if (cannotAddIndex >= 0)
		{
			attachment->Execute_OnDetach(attachment.GetObject(), this, false);
			return;
		}
	}

	if (!_attachmentList.Contains(attachment))
	{
		_attachmentList.AddUnique(attachment);
	}
	attachment->Execute_OnAttach(attachment.GetObject(), this, false);
	OnAttachToGem(attachment);
}

void APuzzleGem::DetachFromGem(TScriptInterface<IPuzzleGemAttachment> attachment)
{
	if (!attachment)
		return;
	if (_attachmentList.Contains(attachment))
		_attachmentList.Remove(attachment);
	attachment->Execute_OnDetach(attachment.GetObject(), this, false);
	OnDetachFromGem(attachment);
}

void APuzzleGem::OnAttachToGem_Implementation(const TScriptInterface<IPuzzleGemAttachment>& attachment)
{
}

void APuzzleGem::OnDetachFromGem_Implementation(const TScriptInterface<IPuzzleGemAttachment>& attachment)
{
}

void APuzzleGem::HandleAttachments()
{
	if (_attachmentList.Num() <= 0)
		return;
	for (int i = _attachmentList.Num() - 1; i >= 0; i--)
	{
		if (_attachmentList[i])
			continue;
		_attachmentList.RemoveAt(i);
	}
}

bool APuzzleGem::CanSelectGem()
{
	if (_attachmentList.Num() <= 0)
		return true;
	const int cannotSelectIndex = _attachmentList.IndexOfByPredicate([&](
		TScriptInterface<IPuzzleGemAttachment> attachment)
		{
			return !attachment->Execute_CanSelectGem(attachment.GetObject(), this);
		});
	return cannotSelectIndex < 0;
}

bool APuzzleGem::CanSwapGem()
{
	if (_attachmentList.Num() <= 0)
		return true;
	const int cannotSwapIndex = _attachmentList.IndexOfByPredicate([&](
		TScriptInterface<IPuzzleGemAttachment> attachment)
		{
			return !attachment->Execute_CanSwapGem(attachment.GetObject(), this);
		});
	return cannotSwapIndex < 0;
}

bool APuzzleGem::CanMoveGem()
{
	if (_attachmentList.Num() <= 0)
		return true;
	const int cannotMoveIndex = _attachmentList.IndexOfByPredicate([&](
		TScriptInterface<IPuzzleGemAttachment> attachment)
		{
			return !attachment->Execute_CanMoveGem(attachment.GetObject(), this);
		});
	return cannotMoveIndex < 0;
}

bool APuzzleGem::CanDeleteGem()
{
	if (_attachmentList.Num() <= 0)
		return true;
	const int cannotDeleteIndex = _attachmentList.IndexOfByPredicate([&](
		TScriptInterface<IPuzzleGemAttachment> attachment)
		{
			return !attachment->Execute_CanDeleteGem(attachment.GetObject(), this);
		});
	return cannotDeleteIndex < 0;
}

bool APuzzleGem::CanMatchGem()
{
	if (_attachmentList.Num() <= 0)
		return true;
	const int cannotMatchIndex = _attachmentList.IndexOfByPredicate([&](
		TScriptInterface<IPuzzleGemAttachment> attachment)
		{
			return !attachment->Execute_CanMatchGem(attachment.GetObject(), this);
		});
	return cannotMatchIndex < 0;
}


//Gem Comparison and Matching #######################################################################


TScriptInterface<IPuzzleGemEquatable> APuzzleGem::GetGemEquatable()
{
	return _gemEquatable;
}

void APuzzleGem::SetGemEquatable(TScriptInterface<IPuzzleGemEquatable> equatable)
{
	if(equatable.GetObject())
	{
		equatable.SetInterface(reinterpret_cast<IPuzzleGemEquatable*>(equatable.GetObject()));
	}
	OnGemEquatableChanged(_gemEquatable, equatable);
	if(_gemEquatable && _gemEquatable.GetObject())
	{
		auto object = _gemEquatable.GetObject();
		object->ConditionalBeginDestroy();
	}
	_gemEquatable = equatable;
}

bool APuzzleGem::CompareGemTo(APuzzleGem* other)
{
	if(!other)
		return false;
	if(!_gemEquatable && !other->_gemEquatable)
		return true;
	if(!_gemEquatable ^ !other->_gemEquatable)
		return true;
	return _gemEquatable->Execute_GemEquals(_gemEquatable.GetObject(), other->_gemEquatable);
}

void APuzzleGem::OnGemEquatableChanged_Implementation(const TScriptInterface<IPuzzleGemEquatable>& lastEquatable,
	const TScriptInterface<IPuzzleGemEquatable>& newEquatable)
{
}


bool APuzzleGem::AvoidDestroyOnGemMatching_Implementation(int matchGemCount, bool asIntersectionGem)
{
	return false;
}


#pragma endregion


#pragma region CLass Flow

// Sets default values
APuzzleGem::APuzzleGem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APuzzleGem::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APuzzleGem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HandleAttachments();
	UpdateGemVelocity(DeltaTime);
}


#pragma endregion

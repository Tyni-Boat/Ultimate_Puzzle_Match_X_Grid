// Copyright © 2023 by Tyni Boat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include <PuzzleEnums.h>

#include "IPuzzleGemAttachment.h"
#include "PuzzleGemEquatable.h"
#include "GameFramework/Actor.h"
#include "PuzzleGem.generated.h"


class UPuzzleGridComponent;

UCLASS(ClassGroup = (Match3Puzzle), BlueprintType, Blueprintable)
class MATCH3PUZZLE_API APuzzleGem : public AActor
{
	GENERATED_BODY()


#pragma region Properties

public:
	//The gems's parent grid
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Puzzle Gem|Gem Params")
	UPuzzleGridComponent* parentGrid;

	//The gems's speed
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Gem|Gem Params")
	float GemSpeed = 5;

	//The gems's delete delay
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Gem|Gem Params")
	float GemDeletionDelay = 0.5f;

	//The gems's grid position.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Puzzle Gem|Gem Params")
	FVector2D GridIndex = FVector2D(-1, -1);

	//The gems's grid position.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Puzzle Gem|Gem Params")
	TEnumAsByte<EGemState> GemState;

	//The resistance scale to external force transfert
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Gem|Gem Params", meta=(UIMin = 0.0f, UIMax = 1.0f))
	float ExternalForceTransferScale = 0.9f;

	//The landing delay, use to delay the Onlanding call on other gems. use carefully!!
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Puzzle Gem|Gem Params", meta=(UIMin = 0.0f, UIMax = 1.0f))
	float LandingDelay = 0;
	
	//The current gem velocity
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Puzzle Gem|Gem Params")
	FVector GemVelocity;

protected:
	//The deletion count down. delete the gem when it reaches zero
	UPROPERTY()
	float _gemDeletionCountDownChrono = 0.5f;

	//The position of the gem on the last frame
	UPROPERTY()
	FVector _lastGemLocation;

	//The gem's attachments list
	UPROPERTY()
	TArray<TScriptInterface<IPuzzleGemAttachment>> _attachmentList;

	//The gem's equatable. it's used to compare gem when matching in the grid.
	UPROPERTY()
	TScriptInterface<IPuzzleGemEquatable> _gemEquatable;

#pragma endregion


#pragma region Properties Handling Functions

public:
	//Gem Indexing ################################################################################

	//Set the gem's grid index
	UFUNCTION(BlueprintCallable, Category="Puzzle Gem|Grid Indexing")
	void SetGridIndex(FVector2D grid_index);

	//Gem Spawning ################################################################################

	//Called when the gem get spawn on grid
	void OnGotSpawn_Internal();

	//Called when the gem got spawn on grid
	UFUNCTION(BlueprintNativeEvent, Category="Puzzle Gem|Life Time")
	void OnGotSpawn();

	//Called when the gem got spawn on grid, in C++
	virtual void OnGotSpawn_Implementation();


	//Gem Deletion ################################################################################

	//Called when the gem is marked as waiting to be destroyed
	UFUNCTION(BlueprintNativeEvent, Category="Puzzle Gem|Life Time")
	void OnMarkedForDestroy();

	//Called when the gem got deleted on grid
	UFUNCTION(BlueprintNativeEvent, Category="Puzzle Gem|Life Time")
	void OnGotDeleted();

	//Internally delete gem, and trigger event
	void OnGotDeleted_Internal();

	//Called when the gem is marked as waiting to be destroyed, c++ implementation
	virtual void OnMarkedForDestroy_Implementation();

	//Called when the gem got deleted on grid, c++ implementation
	virtual void OnGotDeleted_Implementation();

	//Update the gem deletion chrono and return wether or not the chrono reached zero
	bool UpdateGemDeletionChrono(float delta);


	//Gem Selection ################################################################################

	//Called when the gem got selected on grid
	UFUNCTION(BlueprintNativeEvent, Category="Puzzle Gem|Interraction")
	void OnGotSelected();

	//Called when the gem got selected on grid ,c++
	virtual void OnGotSelected_Implementation();

	//Called when the gem got released from selection on grid
	UFUNCTION(BlueprintNativeEvent, Category="Puzzle Gem|Interraction")
	void OnGotSelectionReleased();

	//Called when the gem got released from selection on grid. c++
	virtual void OnGotSelectionReleased_Implementation();


	//Gem inter-action #######################################################################

	//Called when the gem lands on another one when falling
	UFUNCTION(BlueprintNativeEvent, Category="Puzzle Gem|Interraction")
	void OnGemLanded(USceneComponent* onGrid, FVector2D indexGemLandedOn, FVector landingForce);

	//Called when the gem lands on another one when falling, c++
	void OnGemLanded_Implementation(USceneComponent* onGrid, FVector2D indexGemLandedOn, FVector landingForce);

	//Update gem's velocity
	void UpdateGemVelocity(float delta);

	//Gem attachments #######################################################################

	//Add an attachment to gem
	UFUNCTION(BlueprintCallable, Category="Puzzle Gem|Attachment")
	void AttachToGem(TScriptInterface<IPuzzleGemAttachment> attachment);

	//Remove an attachment from gem
	UFUNCTION(BlueprintCallable, Category="Puzzle Gem|Attachment")
	void DetachFromGem(TScriptInterface<IPuzzleGemAttachment> attachment);


	//Called when an attachment is added to gem
	UFUNCTION(BlueprintNativeEvent, Category="Puzzle Gem|Attachment")
	void OnAttachToGem(const TScriptInterface<IPuzzleGemAttachment>& attachment);

	//Called when an attachment is removed from gem
	UFUNCTION(BlueprintNativeEvent, Category="Puzzle Gem|Attachment")
	void OnDetachFromGem(const TScriptInterface<IPuzzleGemAttachment>& attachment);

	//Called when an attachment is added to gem, c++
	virtual void OnAttachToGem_Implementation(const TScriptInterface<IPuzzleGemAttachment>& attachment);

	//Called when an attachment is removed from gem, c++
	virtual void OnDetachFromGem_Implementation(const TScriptInterface<IPuzzleGemAttachment>& attachment);

	//Update the list of attachments
	void HandleAttachments();

	//Check attachment condition for Selection
	bool CanSelectGem();

	//Check attachment condition for Swap
	bool CanSwapGem();

	//Check attachment condition for Movement
	bool CanMoveGem();

	//Check attachment condition for Deletion
	bool CanDeleteGem();

	//Check attachment condition for Matching
	bool CanMatchGem();

	

	//Gem Comparison and Matching #######################################################################

	//Get the equatable of the gem 
	UFUNCTION(BlueprintCallable, Category="Puzzle Gem|Grid Matching")
	TScriptInterface<IPuzzleGemEquatable> GetGemEquatable();
	
	//Set an equatable to the gem 
	UFUNCTION(BlueprintCallable, Category="Puzzle Gem|Grid Matching")
	void SetGemEquatable(TScriptInterface<IPuzzleGemEquatable> equatable);

	//Compare this gem to another, and returns true if theirs equatables are matching
	UFUNCTION(BlueprintCallable, Category="Puzzle Gem|Grid Matching")
	bool CompareGemTo(APuzzleGem* other);

	
	//Called when the gem's Equatable got changed.
	UFUNCTION(BlueprintNativeEvent)
	void OnGemEquatableChanged(const TScriptInterface<IPuzzleGemEquatable>& lastEquatable, const TScriptInterface<IPuzzleGemEquatable>& newEquatable);

	
	//Called when the gem's Equatable got changed.
	void OnGemEquatableChanged_Implementation(const TScriptInterface<IPuzzleGemEquatable>& lastEquatable, const TScriptInterface<IPuzzleGemEquatable>& newEquatable);


	//Called when a gem is about to be destroyed by a match. can be useful to LvUp gem. 
	UFUNCTION(BlueprintNativeEvent)
	bool AvoidDestroyOnGemMatching(int matchGemCount, bool asIntersectionGem);

	//Called when a gem is about to be destroyed by a match. can be useful to LvUp gem. C++
	virtual bool AvoidDestroyOnGemMatching_Implementation(int matchGemCount, bool asIntersectionGem);
	
#pragma endregion


#pragma region CLass Flow

public:
	// Sets default values for this actor's properties
	APuzzleGem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


#pragma endregion
};

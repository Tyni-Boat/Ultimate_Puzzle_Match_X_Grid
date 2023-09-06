// Copyright © 2023 by Tyni Boat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IPuzzleGemAttachment.generated.h"


class APuzzleGem;

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UPuzzleGemAttachment : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implement this interface for any attachment for gem that can affects it's behaviour
 */
class MATCH3PUZZLE_API IPuzzleGemAttachment
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	//Called when the attachment gets attached to a gem. asTransfer is true on attachment transfers to another gem
	UFUNCTION(BlueprintNativeEvent)
	void OnAttach(APuzzleGem* gem, bool asTransfer = false);

	//Called when the attachment gets detached from a gem. asTransfer is true on attachment transfers to another gem
	UFUNCTION(BlueprintNativeEvent)
	void OnDetach(APuzzleGem* gem, bool asTransfer = false);

	//Called when a gem collider with another one "other"
	UFUNCTION(BlueprintNativeEvent)
	void OnGemHitAnotherOne(APuzzleGem* other);

	//Transfer an attachment from one gem to another
	UFUNCTION(BlueprintNativeEvent) 
	void TransferFromTo(APuzzleGem* lastGem, APuzzleGem* newGem);

	//return true if the gem with this attachment can still be selected
	UFUNCTION(BlueprintNativeEvent)
	bool CanSelectGem(APuzzleGem* gem);

	//return true if the gem with this attachment can still be swapped
	UFUNCTION(BlueprintNativeEvent)
	bool CanSwapGem(APuzzleGem* gem);

	//return true if the gem with this attachment can still be moved
	UFUNCTION(BlueprintNativeEvent)
	bool CanMoveGem(APuzzleGem* gem);

	//return true if the gem with this attachment can still be deleted
	UFUNCTION(BlueprintNativeEvent)
	bool CanDeleteGem(APuzzleGem* gem);

	//return true if the gem with this attachment can still receive attachment
	UFUNCTION(BlueprintNativeEvent)
	bool CanAttachToGem(APuzzleGem* gem, UObject* Attachment);

	//return true if the gem with this attachment can still be matched, even if it's not destroyed.
	UFUNCTION(BlueprintNativeEvent)
	bool CanMatchGem(APuzzleGem* gem);


	//C++ ###################################################################################################################################


	
	//Called when the attachment gets attached to a gem. asTransfer is true on attachment transfers to another gem
	void OnAttach_Implementation(APuzzleGem* gem, bool asTransfer = false);

	//Called when the attachment gets detached from a gem. asTransfer is true on attachment transfers to another gem
	void OnDetach_Implementation(APuzzleGem* gem, bool asTransfer = false);

	//Called when a gem collider with another one "other"
	void OnGemHitAnotherOne_Implementation(APuzzleGem* other);

	//Transfer an attachment from one gem to another
	void TransferFromTo_Implementation(APuzzleGem* lastGem, APuzzleGem* newGem);

	//return true if the gem with this attachment can still be selected
	bool CanSelectGem_Implementation(APuzzleGem* gem);

	//return true if the gem with this attachment can still be swapped
	bool CanSwapGem_Implementation(APuzzleGem* gem);

	//return true if the gem with this attachment can still be moved
	bool CanMoveGem_Implementation(APuzzleGem* gem);

	//return true if the gem with this attachment can still be deleted
	bool CanDeleteGem_Implementation(APuzzleGem* gem);

	//return true if the gem with this attachment can still receive attachment
	bool CanAttachToGem_Implementation(APuzzleGem* gem, UObject* Attachment);
	
	//return true if the gem with this attachment can still be matched, even if it's not destroyed. c++
	bool CanMatchGem_Implementation(APuzzleGem* gem);
};

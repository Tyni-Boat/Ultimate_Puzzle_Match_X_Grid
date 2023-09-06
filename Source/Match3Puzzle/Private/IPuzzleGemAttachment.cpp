// Copyright © 2023 by Tyni Boat. All Rights Reserved.


#include "IPuzzleGemAttachment.h"


// Add default functionality here for any IPuzzleGemAttachment functions that are not pure virtual.


void IPuzzleGemAttachment::OnAttach_Implementation(APuzzleGem* gem, bool asTransfer)
{
}

void IPuzzleGemAttachment::OnDetach_Implementation(APuzzleGem* gem, bool asTransfer)
{
}

void IPuzzleGemAttachment::OnGemHitAnotherOne_Implementation(APuzzleGem* other)
{
}

void IPuzzleGemAttachment::TransferFromTo_Implementation(APuzzleGem* lastGem, APuzzleGem* newGem)
{
}

bool IPuzzleGemAttachment::CanSelectGem_Implementation(APuzzleGem* gem)
{
	return true;
}

bool IPuzzleGemAttachment::CanSwapGem_Implementation(APuzzleGem* gem)
{
	return true;
}

bool IPuzzleGemAttachment::CanMoveGem_Implementation(APuzzleGem* gem)
{
	return true;
}

bool IPuzzleGemAttachment::CanDeleteGem_Implementation(APuzzleGem* gem)
{
	return true;
}

bool IPuzzleGemAttachment::CanAttachToGem_Implementation(APuzzleGem* gem, UObject* Attachment)
{
	return true;
}

bool IPuzzleGemAttachment::CanMatchGem_Implementation(APuzzleGem* gem)
{
	return true;
}

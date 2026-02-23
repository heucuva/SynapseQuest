// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SQDialogueTypes.generated.h"


/**
 * @brief ESQDialogueTone categorizes a dialogue option by its emotional tone,
 * similar to Mass Effect's Paragon / Neutral / Renegade system.
 *
 * Use this in UMG to drive option coloring and wheel placement.
 */
UENUM(BlueprintType)
enum class ESQDialogueTone : uint8
{
	/** Friendly, heroic, compassionate response (top-right in Mass Effect) */
	Paragon		UMETA(DisplayName = "Paragon"),

	/** Neutral, investigative, or informational response (middle) */
	Neutral		UMETA(DisplayName = "Neutral"),

	/** Aggressive, intimidating, or ruthless response (bottom-right in Mass Effect) */
	Renegade	UMETA(DisplayName = "Renegade"),
};


/**
 * @brief FSQDialogueOption represents a single selectable response
 * in the dialogue wheel.
 */
USTRUCT(BlueprintType)
struct SYNAPSEQUEST_API FSQDialogueOption
{
	GENERATED_BODY()

	/** Short label shown on the dialogue wheel (e.g., "I'll help you.") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString Text;

	/** The emotional tone of this option, used for UI coloring and placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	ESQDialogueTone Tone = ESQDialogueTone::Neutral;

	/** The full message to send to the LLM when this option is selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString FullResponse;
};


/**
 * @brief ESQDialogueState tracks the current state of the dialogue system.
 */
UENUM(BlueprintType)
enum class ESQDialogueState : uint8
{
	/** No dialogue active */
	Inactive	UMETA(DisplayName = "Inactive"),

	/** Waiting for LLM response */
	WaitingForNPC	UMETA(DisplayName = "Waiting for NPC"),

	/** NPC text displayed, player choosing a response */
	PlayerChoosing	UMETA(DisplayName = "Player Choosing"),

	/** Dialogue is ending (goodbye selected) */
	Ending		UMETA(DisplayName = "Ending"),
};


/**
 * @brief FSQDialogueLine represents a complete NPC dialogue turn:
 * the NPC's spoken text plus the player's available response options.
 */
USTRUCT(BlueprintType)
struct SYNAPSEQUEST_API FSQDialogueLine
{
	GENERATED_BODY()

	/** The NPC's spoken dialogue text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString NPCText;

	/** Available response options for the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FSQDialogueOption> Options;

	/** True if the NPC has indicated the conversation is over */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bIsGoodbye = false;
};

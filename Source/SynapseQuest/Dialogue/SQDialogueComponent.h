// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dialogue/SQDialogueTypes.h"
#include "Synapse.h"
#include "SQDialogueComponent.generated.h"


class USynapseComponent;


/**
 * @brief FOnDialogueLineReady fires when the LLM response has been parsed
 * into a structured dialogue line with NPC text and player options.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnDialogueLineReady,
	USQDialogueComponent*, DialogueComponent,
	const FSQDialogueLine&, Line);

/**
 * @brief FOnDialogueStateChanged fires when the dialogue state changes.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnDialogueStateChanged,
	USQDialogueComponent*, DialogueComponent,
	ESQDialogueState, NewState);

/**
 * @brief FOnDialogueEnded fires when the dialogue conversation ends.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnDialogueEnded,
	USQDialogueComponent*, DialogueComponent);


/**
 * @brief USQDialogueComponent manages Mass Effect-style dialogue flow
 * between a player and an NPC powered by an LLM via USynapseComponent.
 *
 * Attach this component to an NPC actor alongside a USynapseComponent.
 * It sends structured prompts to the LLM and parses the responses into
 * FSQDialogueLine structs (NPC text + response options with tones).
 *
 * Usage:
 * 1. Add both USynapseComponent and USQDialogueComponent to your NPC.
 * 2. Configure the SynapseComponent's Personality with a system prompt
 *    (the DialogueComponent provides a default if none is set).
 * 3. Call StartDialogue() when the player interacts with the NPC.
 * 4. Bind to OnDialogueLineReady to display NPC text and options.
 * 5. Call SelectOption() when the player picks a response.
 * 6. Call EndDialogue() to close the conversation.
 */
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class SYNAPSEQUEST_API USQDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USQDialogueComponent();

	// ============================================================
	// UActorComponent Interface
	// ============================================================

	virtual void BeginPlay() override;

	// ============================================================
	// Configuration
	// ============================================================

	/**
	 * @brief NPC display name shown in the dialogue UI.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString NPCName = TEXT("NPC");

	/**
	 * @brief The opening message sent to the LLM when dialogue starts.
	 * Use {NPCName} and {PlayerName} as template variables.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (MultiLine = true))
	FString OpeningPrompt = TEXT("The player has approached {NPCName} and initiated conversation. Greet them.");

	/**
	 * @brief Additional template variables passed to the SynapseComponent
	 * for every request (merged with built-in variables like NPCName).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TMap<FString, FString> ExtraTemplateVariables;

	// ============================================================
	// Dialogue Flow
	// ============================================================

	/**
	 * @brief Starts a dialogue conversation with the NPC.
	 * Sends the opening prompt to the LLM and transitions to WaitingForNPC.
	 * @param PlayerName The player's display name for template substitution.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(const FString& PlayerName = TEXT("Player"));

	/**
	 * @brief Selects a dialogue option by index, sending it to the LLM.
	 * @param OptionIndex Index into the current FSQDialogueLine::Options array.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SelectOption(int32 OptionIndex);

	/**
	 * @brief Ends the dialogue, clearing conversation state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

	/**
	 * @brief Returns the current dialogue state.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	ESQDialogueState GetDialogueState() const { return DialogueState; }

	/**
	 * @brief Returns true if a dialogue is currently active.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	bool IsDialogueActive() const { return DialogueState != ESQDialogueState::Inactive; }

	/**
	 * @brief Returns the most recent dialogue line from the NPC.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	const FSQDialogueLine& GetCurrentLine() const { return CurrentLine; }

	/**
	 * @brief Returns the NPC display name.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	const FString& GetNPCName() const { return NPCName; }

	// ============================================================
	// Events
	// ============================================================

	/** Fires when a parsed dialogue line is ready to display. */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueLineReady OnDialogueLineReady;

	/** Fires when the dialogue state changes. */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueStateChanged OnDialogueStateChanged;

	/** Fires when the dialogue ends. */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueEnded OnDialogueEnded;

protected:

	/**
	 * @brief Finds or caches the sibling USynapseComponent on the same actor.
	 */
	USynapseComponent* GetSynapseComponent() const;

	/**
	 * @brief Builds the template variables map for a request.
	 */
	TMap<FString, FString> BuildTemplateVariables() const;

	/**
	 * @brief Handles the raw LLM response and parses it into a dialogue line.
	 */
	UFUNCTION()
	void HandleLLMResponse(USynapseComponent* Component, const FSynapseResponse& Response);

	/**
	 * @brief Parses an LLM response string into an FSQDialogueLine.
	 *
	 * Expected format from LLM:
	 * @code
	 * NPC dialogue text here...
	 *
	 * [OPTIONS]
	 * [PARAGON] Short label | Full response text
	 * [NEUTRAL] Short label | Full response text
	 * [RENEGADE] Short label | Full response text
	 * [GOODBYE] Short label
	 * @endcode
	 */
	FSQDialogueLine ParseResponse(const FString& ResponseText) const;

	/**
	 * @brief Sets the dialogue state and broadcasts the change.
	 */
	void SetDialogueState(ESQDialogueState NewState);

	/**
	 * @brief Constructs the system prompt that instructs the LLM
	 * how to format dialogue responses.
	 */
	static FString GetDialogueSystemPrompt();

	/** Current dialogue state */
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	ESQDialogueState DialogueState = ESQDialogueState::Inactive;

	/** Most recent parsed dialogue line */
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FSQDialogueLine CurrentLine;

	/** Player name for the current conversation */
	FString CurrentPlayerName;

	/** Cached SynapseComponent reference */
	UPROPERTY()
	mutable TObjectPtr<USynapseComponent> CachedSynapseComponent;
};

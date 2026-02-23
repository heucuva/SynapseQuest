// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Dialogue/SQDialogueTypes.h"
#include "SQDialogueWidget.generated.h"


class USQDialogueComponent;


/**
 * @brief USQDialogueWidget is the base UMG widget for displaying a
 * Mass Effect-style dialogue UI.
 *
 * This class provides the C++ scaffold; the visual layout, animations,
 * and styling are all done in a Blueprint subclass using UMG Designer.
 *
 * Typical Blueprint subclass layout:
 * - A full-screen Canvas Panel (semi-transparent background)
 * - A text block or Rich Text Block for the NPC name and dialogue
 * - A radial arrangement of USQDialogueOptionWidget entries (the "wheel")
 * - A loading/thinking indicator for the WaitingForNPC state
 *
 * Wire this widget from your Player Controller:
 * 1. Create the widget and call SetDialogueComponent().
 * 2. Add it to the viewport.
 * 3. Call Show/Hide from the dialogue component's state change events.
 */
UCLASS(abstract)
class SYNAPSEQUEST_API USQDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// ============================================================
	// Setup
	// ============================================================

	/**
	 * @brief Associates this widget with a dialogue component and binds events.
	 * Call this once after creating the widget.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|UI")
	void SetDialogueComponent(USQDialogueComponent* InComponent);

	/**
	 * @brief Returns the associated dialogue component.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue|UI")
	USQDialogueComponent* GetDialogueComponent() const { return DialogueComponent; }

	// ============================================================
	// Option Selection (call from Blueprint button events)
	// ============================================================

	/**
	 * @brief Called by option button widgets when the player clicks one.
	 * Forwards the selection to the dialogue component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|UI")
	void OnOptionSelected(int32 OptionIndex);

protected:

	// ============================================================
	// Blueprint Implementable Events
	// ============================================================

	/**
	 * @brief Called when a new dialogue line is ready.
	 * Implement in Blueprint to populate NPC text and option buttons.
	 *
	 * @param NPCName     The NPC's display name.
	 * @param Line        The parsed dialogue line with NPC text and options.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue|UI",
		meta = (DisplayName = "On Dialogue Line Ready"))
	void BP_OnDialogueLineReady(const FString& NPCName, const FSQDialogueLine& Line);

	/**
	 * @brief Called when the dialogue state changes.
	 * Implement in Blueprint to show/hide loading indicators, enable/disable input, etc.
	 *
	 * @param NewState The new dialogue state.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue|UI",
		meta = (DisplayName = "On Dialogue State Changed"))
	void BP_OnDialogueStateChanged(ESQDialogueState NewState);

	/**
	 * @brief Called when the dialogue ends.
	 * Implement in Blueprint to remove the widget, restore input, etc.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue|UI",
		meta = (DisplayName = "On Dialogue Ended"))
	void BP_OnDialogueEnded();

	/**
	 * @brief Returns the color associated with a dialogue tone.
	 * Override in Blueprint to customize per-tone styling.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue|UI")
	FLinearColor GetToneColor(ESQDialogueTone Tone) const;

private:

	/** Callback bound to DialogueComponent::OnDialogueLineReady */
	UFUNCTION()
	void HandleDialogueLineReady(USQDialogueComponent* Component, const FSQDialogueLine& Line);

	/** Callback bound to DialogueComponent::OnDialogueStateChanged */
	UFUNCTION()
	void HandleDialogueStateChanged(USQDialogueComponent* Component, ESQDialogueState NewState);

	/** Callback bound to DialogueComponent::OnDialogueEnded */
	UFUNCTION()
	void HandleDialogueEnded(USQDialogueComponent* Component);

	/** The dialogue component driving this UI */
	UPROPERTY()
	TObjectPtr<USQDialogueComponent> DialogueComponent;
};

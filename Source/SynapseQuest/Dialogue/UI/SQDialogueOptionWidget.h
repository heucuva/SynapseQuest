// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Dialogue/SQDialogueTypes.h"
#include "SQDialogueOptionWidget.generated.h"


/**
 * @brief USQDialogueOptionWidget represents a single selectable option
 * in the Mass Effect-style dialogue wheel.
 *
 * Create a Blueprint subclass (e.g., WBP_DialogueOption) with:
 * - A Button for click handling
 * - A Text Block for the option label
 * - Optional tone-colored accent (border, icon, etc.)
 *
 * The parent USQDialogueWidget creates one of these per option and calls
 * SetOption() to configure it. When the player clicks, the widget calls
 * back to the parent with its OptionIndex.
 */
UCLASS(abstract)
class SYNAPSEQUEST_API USQDialogueOptionWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/**
	 * @brief Configures this option widget with data and its index.
	 * @param InOption   The dialogue option data (text, tone, full response).
	 * @param InIndex    The index into the parent's options array.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|UI")
	void SetOption(const FSQDialogueOption& InOption, int32 InIndex);

	/**
	 * @brief Returns the option data.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue|UI")
	const FSQDialogueOption& GetOption() const { return Option; }

	/**
	 * @brief Returns this option's index in the dialogue line.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue|UI")
	int32 GetOptionIndex() const { return OptionIndex; }

protected:

	/**
	 * @brief Called when the option data is set.
	 * Implement in Blueprint to update text, color, and layout.
	 *
	 * @param InOption  The dialogue option data.
	 * @param InIndex   The option index (0 = top of wheel, going clockwise).
	 * @param ToneColor The suggested color for this option's tone.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue|UI",
		meta = (DisplayName = "On Option Set"))
	void BP_OnOptionSet(const FSQDialogueOption& InOption, int32 InIndex, FLinearColor ToneColor);

	/** The option data */
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue|UI")
	FSQDialogueOption Option;

	/** Index into the parent's option array */
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue|UI")
	int32 OptionIndex = 0;
};

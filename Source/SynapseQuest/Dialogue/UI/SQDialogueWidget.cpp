// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dialogue/UI/SQDialogueWidget.h"
#include "Dialogue/SQDialogueComponent.h"
#include "SynapseQuest.h"


void USQDialogueWidget::SetDialogueComponent(USQDialogueComponent* InComponent)
{
	// Unbind from previous component
	if (IsValid(DialogueComponent))
	{
		DialogueComponent->OnDialogueLineReady.RemoveDynamic(this, &USQDialogueWidget::HandleDialogueLineReady);
		DialogueComponent->OnDialogueStateChanged.RemoveDynamic(this, &USQDialogueWidget::HandleDialogueStateChanged);
		DialogueComponent->OnDialogueEnded.RemoveDynamic(this, &USQDialogueWidget::HandleDialogueEnded);
	}

	DialogueComponent = InComponent;

	// Bind to new component
	if (IsValid(DialogueComponent))
	{
		DialogueComponent->OnDialogueLineReady.AddDynamic(this, &USQDialogueWidget::HandleDialogueLineReady);
		DialogueComponent->OnDialogueStateChanged.AddDynamic(this, &USQDialogueWidget::HandleDialogueStateChanged);
		DialogueComponent->OnDialogueEnded.AddDynamic(this, &USQDialogueWidget::HandleDialogueEnded);
	}
}

void USQDialogueWidget::OnOptionSelected(int32 OptionIndex)
{
	if (IsValid(DialogueComponent))
	{
		DialogueComponent->SelectOption(OptionIndex);
	}
}

FLinearColor USQDialogueWidget::GetToneColor(ESQDialogueTone Tone) const
{
	switch (Tone)
	{
	case ESQDialogueTone::Paragon:
		// Blue — heroic, compassionate
		return FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);

	case ESQDialogueTone::Renegade:
		// Red — aggressive, ruthless
		return FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

	case ESQDialogueTone::Neutral:
	default:
		// White/light gray — neutral, investigative
		return FLinearColor(0.85f, 0.85f, 0.85f, 1.0f);
	}
}

// ============================================================
// Internal Callbacks
// ============================================================

void USQDialogueWidget::HandleDialogueLineReady(
	USQDialogueComponent* Component,
	const FSQDialogueLine& Line)
{
	BP_OnDialogueLineReady(Component->GetNPCName(), Line);
}

void USQDialogueWidget::HandleDialogueStateChanged(
	USQDialogueComponent* Component,
	ESQDialogueState NewState)
{
	BP_OnDialogueStateChanged(NewState);
}

void USQDialogueWidget::HandleDialogueEnded(USQDialogueComponent* Component)
{
	BP_OnDialogueEnded();
}

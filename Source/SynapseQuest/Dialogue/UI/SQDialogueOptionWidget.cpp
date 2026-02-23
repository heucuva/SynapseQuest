// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dialogue/UI/SQDialogueOptionWidget.h"


void USQDialogueOptionWidget::SetOption(const FSQDialogueOption& InOption, int32 InIndex)
{
	Option = InOption;
	OptionIndex = InIndex;

	// Compute tone color
	FLinearColor ToneColor;
	switch (Option.Tone)
	{
	case ESQDialogueTone::Paragon:
		ToneColor = FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);
		break;
	case ESQDialogueTone::Renegade:
		ToneColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);
		break;
	case ESQDialogueTone::Neutral:
	default:
		ToneColor = FLinearColor(0.85f, 0.85f, 0.85f, 1.0f);
		break;
	}

	// Forward to Blueprint for visual setup
	BP_OnOptionSet(Option, OptionIndex, ToneColor);
}

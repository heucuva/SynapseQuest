// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dialogue/SQDialogueComponent.h"
#include "Component/SynapseComponent.h"
#include "SynapseQuest.h"


USQDialogueComponent::USQDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USQDialogueComponent::BeginPlay()
{
	Super::BeginPlay();

	// Bind to the SynapseComponent's response event
	if (USynapseComponent* Synapse = GetSynapseComponent();
		IsValid(Synapse))
	{
		Synapse->OnResponse.AddDynamic(this, &USQDialogueComponent::HandleLLMResponse);
	}
	else
	{
		UE_LOG(LogSynapseQuest, Warning,
			TEXT("USQDialogueComponent on '%s': No sibling USynapseComponent found. "
				 "Add a USynapseComponent to this actor for dialogue to work."),
			*GetOwner()->GetName());
	}
}

USynapseComponent* USQDialogueComponent::GetSynapseComponent() const
{
	if (IsValid(CachedSynapseComponent))
	{
		return CachedSynapseComponent;
	}

	if (AActor* Owner = GetOwner();
		IsValid(Owner))
	{
		CachedSynapseComponent = Owner->FindComponentByClass<USynapseComponent>();
	}

	return CachedSynapseComponent;
}

TMap<FString, FString> USQDialogueComponent::BuildTemplateVariables() const
{
	TMap<FString, FString> Vars = ExtraTemplateVariables;
	Vars.Add(TEXT("NPCName"), NPCName);
	Vars.Add(TEXT("PlayerName"), CurrentPlayerName);
	return Vars;
}

// ============================================================
// Dialogue Flow
// ============================================================

void USQDialogueComponent::StartDialogue(const FString& PlayerName)
{
	if (DialogueState != ESQDialogueState::Inactive)
	{
		UE_LOG(LogSynapseQuest, Warning,
			TEXT("USQDialogueComponent::StartDialogue: Dialogue already active on '%s'"),
			*GetOwner()->GetName());
		return;
	}

	USynapseComponent* Synapse = GetSynapseComponent();
	if (!IsValid(Synapse))
	{
		UE_LOG(LogSynapseQuest, Error,
			TEXT("USQDialogueComponent::StartDialogue: No USynapseComponent on '%s'"),
			*GetOwner()->GetName());
		return;
	}

	CurrentPlayerName = PlayerName;

	// Clear any previous conversation history so each dialogue is fresh
	Synapse->ClearHistory();

	// Enable conversation history for multi-turn dialogue
	Synapse->bUseConversationHistory = true;

	SetDialogueState(ESQDialogueState::WaitingForNPC);

	// Send the opening prompt with our dialogue system prompt injected
	// The system prompt teaches the LLM the response format
	Synapse->ChatWithSystem(
		GetDialogueSystemPrompt(),
		OpeningPrompt,
		BuildTemplateVariables());
}

void USQDialogueComponent::SelectOption(int32 OptionIndex)
{
	if (DialogueState != ESQDialogueState::PlayerChoosing)
	{
		UE_LOG(LogSynapseQuest, Warning,
			TEXT("USQDialogueComponent::SelectOption: Not in PlayerChoosing state"));
		return;
	}

	if (!CurrentLine.Options.IsValidIndex(OptionIndex))
	{
		UE_LOG(LogSynapseQuest, Warning,
			TEXT("USQDialogueComponent::SelectOption: Invalid option index %d (have %d options)"),
			OptionIndex, CurrentLine.Options.Num());
		return;
	}

	const FSQDialogueOption& Option = CurrentLine.Options[OptionIndex];

	// If this was a goodbye option, end the dialogue
	if (CurrentLine.bIsGoodbye && OptionIndex == CurrentLine.Options.Num() - 1)
	{
		EndDialogue();
		return;
	}

	USynapseComponent* Synapse = GetSynapseComponent();
	if (!IsValid(Synapse))
	{
		return;
	}

	SetDialogueState(ESQDialogueState::WaitingForNPC);

	// Send the full response text (or the short text if no full response)
	const FString& MessageToSend = Option.FullResponse.IsEmpty()
		? Option.Text
		: Option.FullResponse;

	Synapse->Chat(MessageToSend, BuildTemplateVariables());
}

void USQDialogueComponent::EndDialogue()
{
	if (DialogueState == ESQDialogueState::Inactive)
	{
		return;
	}

	// Cancel any pending LLM requests
	if (USynapseComponent* Synapse = GetSynapseComponent();
		IsValid(Synapse))
	{
		Synapse->CancelAllRequests();
	}

	CurrentLine = FSQDialogueLine();
	CurrentPlayerName.Empty();

	SetDialogueState(ESQDialogueState::Inactive);
	OnDialogueEnded.Broadcast(this);
}

// ============================================================
// Response Handling
// ============================================================

void USQDialogueComponent::HandleLLMResponse(
	USynapseComponent* Component,
	const FSynapseResponse& Response)
{
	// Ignore responses when we're not expecting them
	if (DialogueState != ESQDialogueState::WaitingForNPC)
	{
		return;
	}

	if (!Response.IsSuccess())
	{
		UE_LOG(LogSynapseQuest, Warning,
			TEXT("USQDialogueComponent: LLM error: %s"), *Response.ErrorMessage);

		// Create a fallback line so the UI can display the error
		CurrentLine = FSQDialogueLine();
		CurrentLine.NPCText = FString::Printf(
			TEXT("(I seem to have lost my train of thought... [%s])"),
			*Response.ErrorMessage);
		CurrentLine.bIsGoodbye = true;

		FSQDialogueOption GoodbyeOption;
		GoodbyeOption.Text = TEXT("Leave");
		GoodbyeOption.Tone = ESQDialogueTone::Neutral;
		CurrentLine.Options.Add(GoodbyeOption);

		SetDialogueState(ESQDialogueState::PlayerChoosing);
		OnDialogueLineReady.Broadcast(this, CurrentLine);
		return;
	}

	// Parse the structured response
	CurrentLine = ParseResponse(Response.Content);

	SetDialogueState(ESQDialogueState::PlayerChoosing);
	OnDialogueLineReady.Broadcast(this, CurrentLine);
}

// ============================================================
// Response Parsing
// ============================================================

FSQDialogueLine USQDialogueComponent::ParseResponse(const FString& ResponseText) const
{
	FSQDialogueLine Line;

	// Split on [OPTIONS] marker
	FString NPCPart;
	FString OptionsPart;

	if (int32 OptionsIdx = INDEX_NONE;
		ResponseText.FindLastChar('[', OptionsIdx)
		&& ResponseText.Mid(OptionsIdx).StartsWith(TEXT("[OPTIONS]")))
	{
		// Find the actual [OPTIONS] tag
		int32 TagPos = ResponseText.Find(TEXT("[OPTIONS]"), ESearchCase::IgnoreCase);
		if (TagPos != INDEX_NONE)
		{
			NPCPart = ResponseText.Left(TagPos).TrimStartAndEnd();
			OptionsPart = ResponseText.Mid(TagPos + 9).TrimStartAndEnd(); // 9 = len("[OPTIONS]")
		}
		else
		{
			NPCPart = ResponseText.TrimStartAndEnd();
		}
	}
	else
	{
		// Try to find [OPTIONS] anywhere
		int32 TagPos = ResponseText.Find(TEXT("[OPTIONS]"), ESearchCase::IgnoreCase);
		if (TagPos != INDEX_NONE)
		{
			NPCPart = ResponseText.Left(TagPos).TrimStartAndEnd();
			OptionsPart = ResponseText.Mid(TagPos + 9).TrimStartAndEnd();
		}
		else
		{
			// No options marker — treat entire response as NPC text and
			// provide a default "Continue" and "Goodbye" option
			NPCPart = ResponseText.TrimStartAndEnd();
		}
	}

	Line.NPCText = NPCPart;

	// Parse individual option lines
	if (!OptionsPart.IsEmpty())
	{
		TArray<FString> OptionLines;
		OptionsPart.ParseIntoArrayLines(OptionLines, true);

		for (const FString& RawLine : OptionLines)
		{
			FString Trimmed = RawLine.TrimStartAndEnd();
			if (Trimmed.IsEmpty())
			{
				continue;
			}

			FSQDialogueOption Option;

			// Detect tone tag: [PARAGON], [NEUTRAL], [RENEGADE], [GOODBYE]
			if (Trimmed.StartsWith(TEXT("[PARAGON]"), ESearchCase::IgnoreCase))
			{
				Option.Tone = ESQDialogueTone::Paragon;
				Trimmed = Trimmed.Mid(9).TrimStartAndEnd();
			}
			else if (Trimmed.StartsWith(TEXT("[NEUTRAL]"), ESearchCase::IgnoreCase))
			{
				Option.Tone = ESQDialogueTone::Neutral;
				Trimmed = Trimmed.Mid(9).TrimStartAndEnd();
			}
			else if (Trimmed.StartsWith(TEXT("[RENEGADE]"), ESearchCase::IgnoreCase))
			{
				Option.Tone = ESQDialogueTone::Renegade;
				Trimmed = Trimmed.Mid(10).TrimStartAndEnd();
			}
			else if (Trimmed.StartsWith(TEXT("[GOODBYE]"), ESearchCase::IgnoreCase))
			{
				Option.Tone = ESQDialogueTone::Neutral;
				Trimmed = Trimmed.Mid(9).TrimStartAndEnd();
				Line.bIsGoodbye = true;

				if (Trimmed.IsEmpty())
				{
					Trimmed = TEXT("Goodbye");
				}

				Option.Text = Trimmed;
				Option.FullResponse = Trimmed;
				Line.Options.Add(Option);
				continue;
			}

			// Split on pipe: "Short label | Full response"
			int32 PipeIdx = INDEX_NONE;
			if (Trimmed.FindChar('|', PipeIdx))
			{
				Option.Text = Trimmed.Left(PipeIdx).TrimStartAndEnd();
				Option.FullResponse = Trimmed.Mid(PipeIdx + 1).TrimStartAndEnd();
			}
			else
			{
				Option.Text = Trimmed;
				Option.FullResponse = Trimmed;
			}

			if (!Option.Text.IsEmpty())
			{
				Line.Options.Add(Option);
			}
		}
	}

	// Ensure there's always at least a "Continue" and "Goodbye" option
	if (Line.Options.Num() == 0)
	{
		FSQDialogueOption ContinueOption;
		ContinueOption.Text = TEXT("Continue...");
		ContinueOption.Tone = ESQDialogueTone::Neutral;
		ContinueOption.FullResponse = TEXT("Continue the conversation.");
		Line.Options.Add(ContinueOption);

		FSQDialogueOption GoodbyeOption;
		GoodbyeOption.Text = TEXT("Goodbye");
		GoodbyeOption.Tone = ESQDialogueTone::Neutral;
		GoodbyeOption.FullResponse = TEXT("Goodbye");
		Line.Options.Add(GoodbyeOption);
		Line.bIsGoodbye = true;
	}

	return Line;
}

// ============================================================
// State Management
// ============================================================

void USQDialogueComponent::SetDialogueState(ESQDialogueState NewState)
{
	if (DialogueState != NewState)
	{
		DialogueState = NewState;
		OnDialogueStateChanged.Broadcast(this, NewState);
	}
}

// ============================================================
// System Prompt
// ============================================================

FString USQDialogueComponent::GetDialogueSystemPrompt()
{
	return TEXT(
		"You are an NPC character named {NPCName} in an interactive video game. "
		"You are having a conversation with a player named {PlayerName}.\n"
		"\n"
		"IMPORTANT: You must format EVERY response exactly as follows:\n"
		"\n"
		"First, write your character's dialogue — what {NPCName} says out loud. "
		"Keep it natural, in-character, and 1-3 paragraphs.\n"
		"\n"
		"Then, after your dialogue, provide EXACTLY 3-4 response options for the "
		"player using this EXACT format:\n"
		"\n"
		"[OPTIONS]\n"
		"[PARAGON] Short friendly label | The full friendly response the player would say\n"
		"[NEUTRAL] Short neutral label | The full neutral/investigative response the player would say\n"
		"[RENEGADE] Short aggressive label | The full aggressive/rude response the player would say\n"
		"[GOODBYE] Leave\n"
		"\n"
		"Rules:\n"
		"- The short label (before the |) should be 2-6 words summarizing the tone.\n"
		"- The full response (after the |) is what the player actually says.\n"
		"- Always include [PARAGON], [NEUTRAL], [RENEGADE], and [GOODBYE] options.\n"
		"- The [GOODBYE] option ends the conversation.\n"
		"- Stay in character at all times.\n"
		"- React appropriately to the player's chosen tone.\n"
		"- Do NOT break the fourth wall or mention that you are an AI.\n"
	);
}

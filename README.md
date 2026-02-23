# SynapseQuest

A showcase project demonstrating the [Synapse](https://github.com/heucuva) LLM integration plugin for **Unreal Engine 5.7**. It contains multiple gameplay variants — first-person exploration, horror, and shooter — each illustrating different ways to wire AI-powered features into Blueprints and C++.

## What This Project Shows

| Variant | Level | Highlights |
|---------|-------|------------|
| **First Person** | `Lvl_FirstPerson` | Base template with NPC interaction via `OnUseOther`, Enhanced Input, first-person camera |
| **Horror** | `Lvl_Horror` | Sprint/stamina system, flashlight, HUD driven by `BlueprintImplementableEvent` delegates |
| **Shooter** | `Lvl_Shooter` | Weapon inventory, projectiles, AI NPCs with StateTree behaviors, EQS, scoreboard |
| **Dialogue** | *(any variant)* | Mass Effect-style LLM dialogue wheel — `USQDialogueComponent` + UMG widgets using `USynapseComponent` |

The **Dialogue** system is the primary Synapse integration demo. It shows how to:

- Attach a `USynapseComponent` and `USQDialogueComponent` to an NPC
- Use a structured system prompt so the LLM returns Paragon / Neutral / Renegade / Goodbye options
- Parse LLM responses into typed `FSQDialogueLine` structs
- Drive a UMG dialogue widget entirely from Blueprint `ImplementableEvent` callbacks
- Leverage Synapse's cascading personality system (Settings → DataTable → Asset → Inline)

## Prerequisites

- **Unreal Engine 5.7** (source or launcher build)
- **Synapse plugin** (not included — see installation below)
- An LLM provider: [LM Studio](https://lmstudio.ai/), [Ollama](https://ollama.com/), OpenAI API key, Anthropic API key, Docker Model Runner, or vLLM

## Installing the Synapse Plugin

The `Plugins/` directory is excluded from this codebase, so you must install Synapse separately.

### Option A — Copy from a release

1. Obtain the **Synapse** plugin folder (from Fab, a release archive, or another project).
2. Copy the entire `Synapse` folder into this project's `Plugins/` directory:
   ```
   SynapseQuest/
   └── Plugins/
       └── Synapse/
           ├── Synapse.uplugin
           ├── Source/
           ├── Content/
           ├── Docs/
           └── ...
   ```
3. Regenerate project files:
   - **Windows**: right-click `SynapseQuest.uproject` → *Generate Visual Studio project files*
4. Open `SynapseQuest.sln` and build, or open `SynapseQuest.uproject` directly in the editor.

### Option B — Clone as a Git submodule

If you have access to the Synapse repository:

```bash
git submodule add <synapse-repo-url> Plugins/Synapse
git submodule update --init --recursive
```

Then regenerate project files as above.

## Configuring an LLM Provider

Once the plugin is installed:

1. Open the project in Unreal Editor.
2. Go to **Project Settings → Plugins → Synapse**.
3. In the **Providers** array, add an entry:
   - **Provider Name**: e.g. `MyProvider`
   - **Provider Type**: `LM Studio`, `Ollama`, `OpenAI Compatible`, `Claude (Anthropic)`, etc.
   - **Base URL**: your provider's endpoint (e.g. `http://localhost:1234/v1` for LM Studio)
   - **Default Model**: the model to use (e.g. `qwen/qwen3-14b`)
   - **API Key**: if required by your provider
4. Set the **Default Provider Name** to match your entry.

The project includes a `DT_LLMProviders` DataTable at `Content/LLM/DT_LLMProviders` for additional provider configuration.

## Project Structure

```
SynapseQuest/
├── Config/                         # Engine, game, input configuration
├── Content/
│   ├── FirstPerson/                # Base first-person level and blueprints
│   ├── Variant_Horror/             # Horror variant level, blueprints, UI
│   ├── Variant_Shooter/            # Shooter variant level, AI, weapons, UI
│   ├── LLM/                        # LLM data assets (DA_MassEffect, DT_LLMProviders)
│   ├── Characters/                 # Mannequin meshes and animations
│   ├── Input/                      # Enhanced Input mapping contexts and actions
│   └── Weapons/                    # Weapon meshes and materials
├── Plugins/
│   └── Synapse/                    # LLM integration plugin (not included — see installation instructions)
└── Source/
    └── SynapseQuest/
        ├── SynapseQuest.Build.cs   # Module dependencies (includes Synapse)
        ├── SynapseQuestCharacter.*  # Base first-person character with Use interaction
        ├── Dialogue/               # Mass Effect-style dialogue system
        │   ├── SQDialogueTypes.h   # Enums and structs (tone, options, lines, state)
        │   ├── SQDialogueComponent.*  # LLM dialogue flow manager
        │   └── UI/
        │       ├── SQDialogueWidget.*       # Base dialogue HUD widget
        │       └── SQDialogueOptionWidget.* # Individual option button widget
        ├── Variant_Horror/         # Horror character, controller, sprint UI
        └── Variant_Shooter/        # Shooter character, AI, weapons, scoreboard
```

## Building

1. Install the Synapse plugin (see above).
2. Open `SynapseQuest.sln` in Visual Studio 2022, or run `GenerateProjectFiles.bat` first.
3. Set the build configuration to **Development Editor** / **Win64**.
4. Build and run.

Alternatively, open `SynapseQuest.uproject` directly in UE 5.7 — the editor will compile on launch.

## License

This sample project is provided as-is for demonstration purposes.  
The **Synapse plugin** is © 2026 Jason Crawford — see the plugin's own `LICENSE` file for terms.

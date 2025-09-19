# EOS Login Sample for Unreal Engine 5.3

This repository contains a lightweight Unreal Engine 5.3 C++ project scaffold that demonstrates how to wire Epic Online Services (EOS) into a UMG widget. The widget exposes login, lobby creation, lobby discovery, and lobby joining flows through Blueprint-callable functions, making it easy to build a UI that authenticates with Epic credentials and manages EOS Lobbies/Sessions.

> **Note**
> This project ships without compiled assets (`Binaries`, `Intermediate`, `Saved`, etc.). Generate project files and build the project with Unreal Engine 5.3 or newer on your machine. EOS credentials must be supplied by the developer through the Epic Developer Portal.

## Features

- EOS authentication using the `AccountPortal` login flow.
- Lobby creation backed by the EOS Session/Lobbies interface.
- Session discovery with presence-enabled lobby searches.
- Joining any discovered lobby by index from a widget.
- Delegates exposed to Blueprints so UI widgets can react to login/session events.

## Project layout

```
EOSLoginSample.uproject
Config/
  DefaultEngine.ini
  DefaultGame.ini
Source/
  EOSLoginSample.Target.cs
  EOSLoginSampleEditor.Target.cs
  EOSLoginSample/
    EOSLoginSample.Build.cs
    Public/
      EOSLobbySubsystem.h
      EOSLoginWidget.h
      EOSLoginGameInstance.h
    Private/
      EOSLobbySubsystem.cpp
      EOSLoginWidget.cpp
      EOSLoginGameInstance.cpp
```

## Getting started

1. **Install prerequisites**
   - Unreal Engine 5.3 (or newer).
   - Visual Studio 2022 (Windows) or Xcode 14 (macOS).
   - EOS SDK components installed via the Epic Launcher (the OnlineSubsystemEOS plugin ships with UE).

2. **Clone and generate project files**
   ```bash
   git clone <this repo>
   cd EOS
   ./GenerateProjectFiles.bat   # or .sh on macOS/Linux
   ```

3. **Configure EOS credentials**
   - Open `Config/DefaultEngine.ini` and fill in the placeholders in the `[OnlineSubsystemEOS]` section using your product's values from the [Epic Developer Portal](https://dev.epicgames.com/portal/).
   - Make sure the Client Policy is set to allow the Account Portal login flow.

4. **Build and run**
   - Launch the project with Unreal Editor.
   - Set `EOSLoginGameInstance` as the default GameInstance class in Project Settings → Maps & Modes.
   - Create a UMG widget blueprint that inherits from `UEOSLoginWidget` (named e.g. `WBP_EOSLobby`). Bind the exposed delegates to update your UI.
   - Add the widget to your HUD (e.g. from your PlayerController) or use the sample `EOSLoginGameInstance` to spawn it at startup.

5. **Testing**
   - In the editor, play-in-editor (PIE). Use the widget buttons to log in via the Account Portal, create a lobby, discover existing lobbies, and join one of the returned sessions.

## Implementing the widget

The `UEOSLoginWidget` class is designed to be extended in UMG. Override `NativeConstruct` and bind the provided `BlueprintImplementableEvent` hooks if you want to react to login/session states directly in Blueprint.

Example Blueprint flow:

1. Create a Widget Blueprint inheriting from `EOSLoginWidget`.
2. Add buttons/text fields.
3. On the Login button, call `RequestEpicLogin`.
4. On the Create Session button, call `CreateLobby` (expose a text box for max players).
5. Call `SearchForLobbies` and populate a scroll list using the `OnSessionsUpdated` event.
6. Use `JoinLobbyByIndex` with the selected entry's index.

## Additional notes

- The code favors clarity over advanced error handling. You can extend `UEOSLobbySubsystem` to push richer status/error messages to the UI.
- When packaging, be sure to include the EOS overlay prerequisites (Install Bundled EOS Redistributables and ensure the EOS overlay is enabled in Project Settings).
- Refer to Epic's documentation for crossplay requirements and platform-specific credential setup.

Enjoy building with EOS!

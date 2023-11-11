# ggxrd_freecam

![Cannot display image](screen.png)

## Description

Allows to move camera freely in Guilty Gear Xrd Rev2 version 2211 (as of 11'th November 2023).  
No controller support (I don't own a controller).

## Credits

Thanks to WorseThanYou for reworking the mouse controls so that they work with the Raw input API.

## System requirements

Intel processor architecture x86 (32-bit) or x64 (64-bit) (AMD will work). Windows operating system.

## Quickstart

1. Launch the game. Or you can do the next step at any point while the game is running.

2. Launch `ggxrd_freecam_injector.exe`.

3. Start a match. Freecam only works within matches, in training room, etc (also works while the game is paused).  
   Left-click anywhere on the Guilty Gear's window and the camera should freeze. You can now move the camera freely (see `Controls` section). Left-click again and the camera should unfreeze. More in `Controls` section.

To turn off the mod you can launch `ggxrd_freecam_injector.exe` again.  
If trying to use the mod with a game version that it doesn't fit, the game may crash. The mod should be possible to combine with other mods, but there might be some mods that can't be combined with this one (if they hook or sigscan the same functions).

The mod may show up as a virus. I swear this is not a virus, check the source code, compile this yourself if still doubting. Check commit history of this repo to see no one else but me modified this. Add this to whatever antivirus exceptions necessary and run as administrator if necessary.

### Controls

Default controls are:

* Left-click anywhere on Guilty Gear's window: toggle freecam mode;
* Right-click anywhere on Guilty Gear's window: same as Left-click, except when exiting freecam mode the camera will remain frozen;
* F1: toggle (show/hide) HUD. Doesn't work on all of HUD elements, you may hide the remaining HUD elements using the game's built-in menus and ways to hide HUD;

While the freecam mode is on:

* Move mouse: look around;
* TFGH: move camera around the space;
* VBN: move camera up in space;
* Space: move camera down in space;
* PageDown: roll the camera to the right;
* PageUp: roll the camera to the left;
* Home: reset roll and FOV to their default values;
* Mouse wheel up: decrease FOV (field of view). This makes everything bigger;
* Mouse wheel down: increase FOV (field of view). This makes everything smaller;

Controls can be configured: read on.

### Controls configuration

If you wish to configure key and/or mouse combinations for the controls, create a text file named `ggxrd_freecam.ini` and place it in the directory where the game executable is. For example, for me my Steam version of the game is located at `...\SteamLibrary\steamapps\common\GUILTY GEAR Xrd -REVELATOR-\Binaries\Win32`.  
Here's an example of the `.ini` file:

```ini
; To configure camera hotkeys place this file in the folder with the game executable
; For example it could be C:\Something\SteamLibrary\steamapps\common\GUILTY GEAR Xrd -REVELATOR-\Binaries\Win32
; Available keys:
; Backspace, Tab, Enter, PauseBreak, CapsLock, Escape, Space, PageUp, PageDown, End, Home, ArrowLeft, ArrowUp, ArrowRight, ArrowDown, PrintScreen, Insert Delete, Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, NumMultiply, NumAdd, NumSubtract, NumDecimal, NumDivide, F1, F2, F3, F4, F, F6, F7, F8, F9, F10, F11, F12, NumLock, ScrollLock, Colon, Plus, Minus, Comma, Period, Slash, Tilde, OpenSquareBracket, Backslash, CloseSquareBracket, Quote, Backslash2, LeftClick, RightClick, MiddleClick, MouseMoveLeft, MouseMoveUp, MouseMoveRight, MouseMoveDown, MouseWheelUp, MouseWheelDown, 0123456789, ABCDEFGHIJKLMNOPQRSTUVWXYZ, Shift, Ctrl, Alt
; You can specify key and key + mouse combinations by separating key names with the '+' sign

; Toggle freecam on/off
toggleFreecam = LeftClick

; Toggle freecam on/off, but when toggling off, free onnly the cursor and leave the camera position unchanged
toggleFreecamButLeaveLocked = RightClick

; Moves camera across space
moveCameraForward = T
moveCameraBackwards = G
moveCameraLeft = F
moveCameraRight = H
moveCameraUp = V
moveCameraUp = B
moveCameraUp = N
moveCameraDown = Space

; Rotates camera up/down/left/right
lookUp = MouseMoveUp
lookDown = MouseMoveDown
lookRight = MouseMoveRight
lookLeft = MouseMoveLeft

; Rolls camera left or right (think Lean in FPS games)
rollRight = PageDown
rollLeft = PageUp

; Resets roll back and FOV to their default values
resetRoll = Home

; Increase/decrease field of view. The smaller the FOV, the bigger everything is
fovDecrease = MouseWheelUp
fovIncrease = MouseWheelDown

; Hide/show HUD
toggleHud = F1

; Multiplier for camera rotation speed up/down/left/right. Active always while looking around
lookaroundSpeedMultiplier = 1.0

; Multiplier for camera movement speed across space. Active always while moving
movementSpeedMultiplier = 1.0

; Multiplier for camera rolling speed (using rollRight/rollLeft). Active always while rolling
rollMultiplier = 1.0

; Multiplier for FOV change speed (using fovIncrease/fovDecrease). Active always while changing FOV
fovChangeSpeedMultiplier = 1.0
```

You can specify a combination of keys and/or mouse buttons, separated by `+` sign.  
Only the following key names are allowed: Backspace, Tab, Enter, PauseBreak, CapsLock, Escape, Space, PageUp, PageDown, End, Home, ArrowLeft, ArrowUp, ArrowRight, ArrowDown, PrintScreen, Insert Delete, Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, NumMultiply, NumAdd, NumSubtract, NumDecimal, NumDivide, F1, F2, F3, F4, F, F6, F7, F8, F9, F10, F11, F12, NumLock, ScrollLock, Colon, Plus, Minus, Comma, Period, Slash, Tilde, OpenSquareBracket, Backslash, CloseSquareBracket, Quote, Backslash2, LeftClick, RightClick, MiddleClick, MouseMoveLeft, MouseMoveUp, MouseMoveRight, MouseMoveDown, MouseWheelUp, MouseWheelDown, 0123456789, ABCDEFGHIJKLMNOPQRSTUVWXYZ, Shift, Ctrl, Alt.  
If the mod is already running you need to reload it in order to apply the new controls. To reload the mod you can run the injector again.

## Build instructions

There are two separate projects in the solution.

The `ggxrd_freecam_injector` project builds an application that will inject a dll into the process and exit. The main action will then take place in the dll, in the target process' address space.

The `ggxrd_freecam_dll` project builds the dll that's responsible for the freecam functionality.

Both applications need to be compiled as 32-bit (x86) projects. Open the `.sln` file, which is located in the root of the repository, using Visual Studio (I'm using 2022 Community Edition). In the top bar select `Release` and `x86`.

The dll project must be compiled with UNICODE unset. Right-click the `ggxrd_freecam_dll` project in `Solution Explorer` and go to `Properties` -> `Advanced` -> `Character Set`. Select `Use Multi-Byte Character Set`. This setting is set separately for each `Release` + `x86` combination so it might change after changing the target platform.

The injector project must be compiled with UNICODE to support multi-language directory paths. Right-click the `ggxrd_freecam_injector` project in `Solution Explorer` and go to `Properties` -> `Advanced` -> `Character Set`. Select `Use Unicode Character Set`.

Add dependencies to the dll project: read below.

## Development dependencies

The dll part of the project depends on:

* Microsoft Detours library: <https://github.com/microsoft/Detours> Follow their instructions on how to build the `.lib` static library. You need to build the 32-bit (x86) version.  
  In order to link the static library you must right click the project in `Solution Explorer`. Go to `Properties` -> `VC++ Directories`. Add the full path to the `Detours-main\include` folder in the `Include Directories` field.  
  Add the full path to the `Detours-main\lib.X86` directory to the `Library Directories` field.  
  In the project settings property pages go to `Linker` -> `Input` and add `detours.lib` into the `Additional Dependencies` field.  
  This project uses version 4.0.1 of Detours dated Apr 16, 2018.

After setting UNICODE support correctly and adding dependencies you can build the project and the resulting `.exe` and `.dll` will appear in the `REPO_ROOT\Release` folder.

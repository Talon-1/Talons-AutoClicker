# clicker
Auto-clicker made in modern C++, easy to use and does NOT take over your mouse!  
Originally intended for automation tasks in Minecraft.
![image](https://raw.githubusercontent.com/Talon-1/Talons-AutoClicker/main/clicker.png)

## How to use
Set your desired time intervals in the provided fields, click "Set Target Application" and click on the application you wish to send mouse clicks to.
This WILL find the handle of whatever is directly under the mouse, If you're ontop of some windows control, that handle will be used. If you wish to send 
clicks directly to the main application, click on it's titlebar. Normal event bubbling must be considered depending on the task required.

For instance, if you're trying to click on a list item in some dialogs listbox, and you've set the target based on the main application (IE clicking it's titlebar)
then the mouse clicks will be sent to the main application itself, IE last stop of event bubbling. Events bubble backwards, not forwards. You should instead set the 
target to the listbox, which then using a defined location would click the proper item inside said listbox.

You can manually define a location, or press the "Get Location" button and click your intended area, and it will auto-fill the coordinates with the relative mouse 
coordinates of it's location. If you're not using the defined location, it will send the mouse coordinates of it's current position relative to the desktop!

To assign a hotkey to turn on/off: click Assign Key and press any single key, and it will register that key as a toggle even if the app is not the active application.

## Bugs or suggestions
Bugs or suggestions should be reported at the issues section.

## Download
Download the latest release at the [releases section](https://github.com/Talon-1/Talons-AutoClicker/releases/latest).

## Compiling
### Prerequisites
Microsoft Visual Studio (Preferably the latest version) with C++ installed.

### Compiling from the source
Open the solution file `Talons auto clicker.sln`, then select `Release | x86` on the Build configuration and press Build solution.

## Errors
### vcruntime140.dll and msvcp140.dll errors
That happens because you don't have the runtime and libraries required for this to run.  
Please install preferably both `x86` and `x64` packages of the VC++ 2019 runtime linked below.
- https://aka.ms/vs/16/release/vc_redist.x86.exe
- https://aka.ms/vs/16/release/vc_redist.x64.exe

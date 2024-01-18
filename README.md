# THIS IS A COMMUNITY PROJECT, ALL UPDATES FROM YOU GUYS AND US, THIS IS OURS SO HELP BUILD UP AND SUPPORT!

# BetterOsu

BetterOsu - internal osu! mod, realtime difficulty changer for any beatmap, works online.  

![preview](https://cdn.discordapp.com/attachments/1179510539894919299/1196456844051370075/static.png?ex=65b7b244&is=65a53d44&hm=661cf2f85dd3bcc4bd21155a40e9742f279d0dbff7a561b94b9a1841f76ea1bc&)  

## Download

[GitHub Actions Artifact](https://github.com/botsolver/bettermintosu/actions) - most recent build.  
[Releases](https://github.com/botsolver/bettermintosu/releases/latest) - probably outdated build.  

## Usage

run osu!, then run your favorite injector, you may use the supplied one.
mod ui should appear in top left corner of the osu! window, see [controls](#controls).  

### Controls

|    Keys     |   Description  |
|:-----------:|:--------------:|
| Right Click |    Settings    |
| F11         |  Hide Mod Menu |

## Features

- Difficulty Changer:
    * Approach Rate (AR)
    * Circle Size (CS)
    * Overall Difficulty (OD)

- Timewarp:
    * Scale

- Replay Copy (*a bit buggy!*):
    * Add/Remove Hard Rock (HR)
    * Replay Keys Only
    * Replay Aim Only
    * Leaderboard Replay Download

- Aimbot:
    * Cursor Speed
    * Spins Per Minute

- Relax (*unstable rate beta!*):
    * SingleTap
    * Alternate
    * Variable Unstable Rate

- Mods:
    * Score Multiplier Changer
    * Unmod Flashlight
    * Unmod Hidden

- Misc:
    * Set Font Size
    * Set Discord RPC Status Text
    * Unload DLL

## Build

It shouldn't be necessary to build the project unless you're debugging or modifying the source code  

### Requirements

* MSVC x64/x86 build tools
* Windows SDK
* .NET Framework SDK

### Execute nobuild

    cmd.exe
    nobuild.exe

Alternatively, bootstrap nobuild

    cmd.exe
    vcvarsall x86
    cl nobuild.c && nobuild.exe

### Optional nobuild flags

Build and run standalone BetterOsu with debug symbols:

    nobuild.exe debug standalone run

Standalone BetterOsu is currently used for ui debugging as an alternative to unloading the dll.  
It doesn't function as an external, [features](#features) will not work.

|    Flag     |          Description          |
|:-----------:|:-----------------------------:|
| all         | Build All                     |
| No Flags    | Freedom Only                  |
| standalone  | Standalone Only               |
| run         | Run standalone after build    |
| rebuild     | Update headers / Rebuild all  |
| debug       | Symbols, Disable Optimizations|

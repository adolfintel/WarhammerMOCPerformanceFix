# Warhammer: Mark of Chaos Performance Fix

[Warhammer: Mark of Chaos](https://www.gog.com/game/warhammer_mark_of_chaos_gold_edition) is a real time strategy game released in 2006.

Despite its age, the game remains CPU bottlenecked to this day due to the fact that it only uses 1 CPU core and there's some leftover debug code that spams millions of system calls to `GetThreadTimes` and `QueryPerformanceCounter`. This causes long load times and irregular performance on Windows and makes the game virtually unplayable on Wine, where these system calls are very slow.

This patch removes the leftover debug code, improving __loading times by 40-50% on Windows__ and by nearly 3000% on Linux (that's not a typo), __game performance will also improve by about 10-15%__ in CPU-bound scenarios (such as large hordes of rats), and __stuttering is reduced__ in general.

## Installation
* Make sure you're using the GOG version of the game, it should say Version 2.14 in the menu
* Open the game's installation folder
* Rename `binkw32.dll` to `binkw23.dll`
* Extract `binkw32.dll` to the game's folder
* Run the game

If you run into issues while playing the game, report them here or send an email to [info@fdossena.com](mailto:info@fdossena.com)

__Please note that your antivirus software might classify this patch as malware since it overwrites the game code, you can safely ignore it.__

## Compatibility
|**Version**|**Source**|**Compatibility**|
|-----------|----------|-----------------|
| 2.14 | [GOG](https://www.gog.com/game/warhammer_mark_of_chaos_gold_edition) | **Yes** |
| 2.14 (EU) | Retail | **Yes** <sup>1</sup> |
| 2.14 (US) | Retail | Probably <sup>1</sup> |
| 2.14 (RU) | Retail | Probably <sup>1</sup> |
| <= 1.74 | Retail | **No** |

<sup>1</sup>. The game's SafeDisc DRM is incompatible with modern systems, a crack is required.

## ASI mods
This patch contains a simple ASI loader. To load ASI mods into the game, make a folder called `asi` in the game's installation folder and put your ASI mods into it, they'll be loaded automatically when the game is launched.

## How it works
This article explains how this fix was made and how it works: [link](https://fdossena.com/?p=whmocfix/i.md)

## How to build
Want to improve the patch? Great! You'll need Visual Studio 2019. Load the solution in VS, make sure the Release x86 build is selected, and build it.

## Credits
* WarrantyVoider for his super useful [Proxy DLL Maker](https://github.com/zeroKilo/ProxyDllMaker)
* Erik-JS for his awesome [Mass Effect ASI loader](https://github.com/Erik-JS/masseffect-binkw32/tree/master/ME1) that served as a base for the ASI loader in this patch

## License
Copyright (C) 2022 Federico Dossena

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this program. If not, see https://www.gnu.org/licenses/lgpl.

This program and its author are in no way affiliated to Deep Silver, Bandai Namco, Black Hole Entertainment or Games Workshop.  
Warhammer is a registered trademark of Games Workshop.

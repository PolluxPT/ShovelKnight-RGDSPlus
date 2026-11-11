# Shovel Knight: Treasure Trove — RGDS+

**Beta 1 · GOG Linux 4.1 · Anbernic RGDS+ stock Linux · standalone, without PortMaster.**

The owner confirmed that the 64-bit version works on the RGDS+. The supplied test log shows gameplay loading, a game exit code of 0, and restoration of volume and lower-screen settings. This is the first public beta; extended testing across every campaign and mode has not been completed.

## Install

1. Download **ShovelKnight_RGDSPlus_Beta1_Install.zip** from [Releases](https://github.com/PolluxPT/ShovelKnight-RGDSPlus/releases). Extract it on your computer.
2. Copy its **APPS** folder into **Roms** on your memory card. Merge the existing APPS folder if asked.
3. Put your own **shovel_knight_treasure_trove_4_1_36252.sh** Linux installer into **Roms/APPS/ShovelKnight_RGDSPlus/gamedata/**.
4. Launch **ShovelKnight.sh** from the handheld's Apps menu. The first launch imports your files automatically and may take several minutes.

Buy and provide your own DRM-free GOG Linux game. The PC installer is read as an archive and is never executed on the handheld. No terminal commands or PortMaster installation are needed. See [INSTALL.txt](APPS/ShovelKnight_RGDSPlus/INSTALL.txt) for the extracted-game-folder alternative.

**Already using the working Alpha 1.4?** Merge this APPS folder over the existing installation. Keep game/ and savedata/. If the supported game files are present, no installer copy or fresh import is required.

## Controls and device handling

| Physical control | Behavior |
| --- | --- |
| D-pad / left stick | Movement and menu navigation |
| A, B, X, Y | Game face-button actions, with the established Chasm physical calibration |
| L1 / R1 / L2 / R2 | Game shoulder and trigger inputs |
| START | Start / pause input |
| SELECT | Back / select input |
| Hold START + SELECT for 1.5 seconds | Exit the game |
| Volume buttons | Live volume adjustment |

The port applies **0.35 × (level / 10)²** to final audio output, including startup sounds. It saves the incoming system volume and restores it on exit. The lower screen is powered off during play and its previous brightness/power state is restored afterward. Temporary CPU/GPU performance settings are restored too.

## Compatibility

- Tested device: RGDS+ with RK3568, 1 GB RAM, stock ARM64 Linux 6.1.141.
- Supported game: GOG DRM-free Linux **Treasure Trove 4.1**, installer **shovel_knight_treasure_trove_4_1_36252.sh**.
- Windows/Steam/other game versions and other firmware builds have not been validated.
- The bundled Box64 and graphics libraries are part of this port. They do not replace firmware files or require PortMaster.

## Saves and support

Keep **ShovelKnight_RGDSPlus/savedata/** when updating. To uninstall, remove ShovelKnight.sh and its ShovelKnight_RGDSPlus folder after backing up savedata/.

If startup fails, attach **ShovelKnight_RGDSPlus/ShovelKnight_TestLogs.zip** to an issue. Do not attach your installer, game files, or saved games. Full logs are kept in logs/.

Known beta observations: the runtime can report a teardown segfault after the game's normal shutdown; in the supplied working session the launcher still reported exit code 0 and restored the device settings. The audio log also recorded a small number of bounded queue overruns/underruns during the session. The owner reported that gameplay works fine; this beta does not claim exhaustive testing or a specific frame rate.

## What is included

This repository and the public installation ZIP contain the launcher, device integration, runtime libraries, original integration source, installation scripts and license notices. **No Shovel Knight executable, FMOD, Box2D, GLEW, game data, music, artwork, installer, logs or saves are distributed.** Required game components are imported locally from the user's own copy.

The game remains the property of Yacht Club Games and its respective rights holders. This is an unofficial community project and is not endorsed by the game's developer or Anbernic.

Integration code: [MIT](LICENSE). Third-party components retain their own licenses; see [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) and [THIRD_PARTY_SOURCES.md](THIRD_PARTY_SOURCES.md).

## Developer details

The original x86_64 game runs through ARM64 Box64. Native SDL2 and GL4ES supply the handheld graphics/audio path. The tested Alpha 1.4 runtime binaries are retained unchanged for Beta 1. Only packaging, version labeling, game import and documentation are updated. See [Source/BUILD.md](Source/BUILD.md).

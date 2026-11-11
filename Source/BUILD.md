# Beta 1 — implementation and build notes

The Alpha 1.3 target log confirms native SDL output initialized but FMOD EventSystem::getEventBySystemID returned 33 for ID 52, mode 1, triggering the unmodified game assertion. This is an internal FMOD failure, not a missing-file assertion or a memory exhaustion report. The precise defect inside the translated 32-bit FMOD path is not established.

This candidate uses the original owner's x86_64 game, Box2D, GLEW and FMOD Ex 4.44.33. The x86_64 event library dependency filename is normalized to the single main FMOD library, as in the previous 32-bit normalization. No game code or asserts are patched. The shared data and save directories are retained.

The existing dual-architecture Box64/Box32 core supplies the x86_64 translator. Original Braid GLES hooks are only exported for wrapped32 SDL; the ARM64 native_video.c shim supplies equivalent GLES2 attribute forcing, shown-window flags, gl4es_rgds_attach_context(), desktop GL procedure resolution, first-frame show and GL flush for the 64-bit SDL route.

The core exports an internal native dlsym dispatcher that bypasses ordinary symbol interposition. This exact core uses a writable native lookup cache at 0x36032ef8; the bridge installs its forwarding callback there. Before writing, it checks the main executable maps and the first 16 bytes of the dispatcher at 0x35379050. The launcher additionally verifies the full core SHA-256 below. No executable code or installed firmware file is modified. The callback resolves actual libc/libdl entry points directly from their ELF symbol tables to avoid recursive binding to the core's own dlopen/dlsym exports. It is active only in the game subprocess. LD_PRELOAD is removed from the environment after loading to avoid leaking the ARM64 shim into guest preload lists or auxiliary programs.

Pinned Box64 core SHA-256: 5a1601588f55e5f9c87452d60b4b20441787ca32bd6630d1443ec296b52c363a

Do not substitute another core without rebuilding the bridge and updating these checks.

Native audio uses the Alpha 1.3 realtime FMOD DSP -> bounded stereo ring -> native SDL callback route, with signed 16-bit output and the exact 0.35*(level/10)^2 curve applied at the final PCM stage. The SDL callback request is increased from 1024 to 2048 frames to give startup more scheduling margin. No FMOD error is suppressed.

Build guest helper:
`zig cc -target x86_64-linux-gnu.2.28 -shared -fPIC -O2 -fno-builtin shovel_rgds.c -ldl -pthread -o libshovel_rgds.so`

Build native video helper:
`zig cc -target aarch64-linux-gnu.2.28 -shared -fPIC -O2 native_video.c -ldl -o librgds_video.so`

GCC 12.2 x86_64 support libraries are from Debian 12 libstdc++6/libgcc-s1 12.2.0-14+deb12u1. The existing GCC runtime license notice applies; Debian package copyright files are retained here. These are guest libraries, not replacements for ARM64 firmware libraries.

Native-source behavior is covered by tests for active/inactive video lookup, GLES context attachment, GL-procedure fallback, swap/flush/show forwarding, controls, preferences, final PCM gain, mute, invalid samples and bounded queues. A guest x86_64 -> real ARM64 Box64 -> ARM64 SDL/GL mock integration test checks the pinned cache hook and graphics lookup chain under QEMU.

An isolated real FMOD event-52 probe succeeds with the 64-bit libraries through ARM64 Box64 interpreter mode under QEMU. The 32-bit interpreter probe also succeeded, so this alone does not establish the cause of the handheld failure. Nested QEMU + Box64 dynamic-recompiler probes timed out; no performance or dynarec success is claimed from those runs. The candidate on the handheld keeps dynamic recompilation enabled.

The developer host uses desktop software rendering and a dummy audio sink. It requires a host-only EGL GLEW build and desktop context/window setup for the headless test; those host-only graphics components are not shipped. The subsequent owner-supplied Alpha 1.4 log confirms the physical graphics/audio route reached gameplay and exited with code 0; the owner reported that the game works fine. No universal frame-rate or all-mode claim is made.

This public Beta 1 excludes all commercial inputs and supplies the user-owned-file importer.


## Beta 1 packaging

All shipped executable/runtime bytes match the working Alpha 1.4. Beta 1 replaces the private installation manifest with a public-runtime manifest plus user-game metadata, removes the obsolete 32-bit normalizer from the startup path, and adds import-game.py. The importer validates all 450 user-owned files on first import, normalizes only the x86_64 FMOD dependency filename, then commits the prepared files. Startup subsequently checks file sizes plus runtime/critical game checksums.

The public release contains no game files. The older personal-update discussion above describes development history only. Do not publish the previous PERSONAL ZIP.

Build and test the integration with a C compiler and Zig as shown above. Source/Tests/test_import.py covers importer rollback, checksums, idempotence, path validation and directory/archive inputs. The release packaging script only packages an empty public application tree; it rejects commercial file paths and private state.

# witcher3-dumper

Dump a C++ SDK of The Witcher 3 (next-gen, REDengine 3) from the running game: classes, structs, enums, script functions, natives, config variables and console commands.

The engine builds its reflection database at startup: every class, property, function and enum is registered at run time, and the script classes only exist once the compiled scripts are loaded. witcher3-dumper is a DLL that reads that database from inside the game and writes headers that compile, with every size and offset checked.

```
witcher3-dumper attached
anchor rtti: rva 0x31E800 (1 match)
anchor as_char: rva 0x26B72E0 (1 match)
anchor config: rva 0x32B890 (1 match)
anchor global_ctor: rva 0x26FDDA0 (1 match)
anchor class_ctor: rva 0x26FDA50 (1 match)
version: 5.0.1048522 P4CL: 13413172 Stream: //Red_engine/Main.Lava.Release
collected 6664 classes, 684 enums, 17 bitfields, 3160 other types, 2226 global functions (868 exec), 1747 cvars
wrote dump to C:\Users\<user>\Documents\witcher3-dumper\5.0.1048522
```

## Features

- Single DLL, no dependencies; it unloads itself after the dump
- No hardcoded addresses: engine entry points are found by byte pattern
- Native and script properties at their reflected offsets, with padding, `alignas` and `#pragma pack` only where they are needed
- Script variables of script objects in a typed `ScriptData` struct, reached through `script()`
- Every function declared with real C++ types, with the native's RVA and script flags
- `Asserts.hpp` with a size, alignment and offset check for every type and member
- Config variables as an INI file with values, defaults, ranges and the address of each value, ready for a memory editor
- Console commands as a searchable HTML reference with example calls

## Usage

1. Start the game (`bin\x64_dx12\witcher3.exe`).
2. Inject `witcher3-dumper.dll` with any LoadLibrary injector.
3. Wait for the main menu. The dump starts on its own once the scripts are loaded; press **F8** to dump right away.

The output goes to `Documents\witcher3-dumper\<build>\`, and the log to `Documents\witcher3-dumper\log.txt`.

| File | Contents |
| --- | --- |
| `Classes.hpp` | Every reflected class and struct, script variables (`ScriptData`) and functions. Global script functions are static members of `Globals`. |
| `Enums.hpp` | Every reflected enum and bitfield. |
| `Asserts.hpp` | Size, alignment and member offset checks for `Classes.hpp`. Include it in one translation unit. |
| `CVars.ini` | Config variables grouped by section, like the game's own ini files, with type, default, range and the RVA of the value the game reads (`"witcher3.exe"+RVA`). |
| `ExecCommands.html` | Console commands by category, with arguments, script source location and example calls. |

### Example

```cpp
// Class CExplorationMover
// 0x0000 (0x0058 - 0x0058)
class CExplorationMover : public CObject
{
public:
	struct ScriptData
	{
		THandle<class CExplorationStateManager> m_ExplorationO; // +0x0000 (0x0008)
		THandle<class CExplorationInput> m_InputO; // +0x0008 (0x0008)
		struct Vector m_WorldPositionV; // +0x0010 (0x0010)
		...
	};

	ScriptData *script()
	{
		return *(ScriptData **)((uint8_t *)this + 0x20);
	}

	// Functions
	void SetVelocity(const struct Vector& _NewVelocityV);
	float GetMovementSpeedF();
	...
};
```

```cpp
float speed = mover->script()->m_SpeedF;
```

### Console

`ExecCommands.html` lists the commands of the game's debug console. To open it, add these lines to `Documents\The Witcher 3\dx12user.settings` while the game is closed:

```ini
[General]
DBGConsoleOn=true

[Cheat]
CheatLevel=2
```

Then press the key with virtual-key code 192 (`~` on a US layout, `ò` on an Italian one).

## Building

Requirements: Visual Studio 2026 with the **Desktop development with C++** workload (v145 toolset, Windows 10 SDK).

Open `witcher3-dumper.sln`, select **Production | x64** and build, or from a developer command prompt:

```
msbuild witcher3-dumper.sln /p:Configuration=Production /p:Platform=x64
```

The DLL is written to `bin\witcher3-dumper.dll`.

The two configurations are **Development** and **Production**. Both keep runtime checks and standard library hardening enabled.

## How it works

1. **Anchors.** Five byte patterns locate the RTTI system, the name pool lookup, the config system and the two native function tables. Each pattern matches exactly once in the game's `.text`.
2. **Waiting.** The type and global function counts are polled until they are stable for five seconds and the console commands exist, which means the compiled scripts are loaded.
3. **Collection.** The RTTI system's type map yields every class (base, properties, functions, flags), enum (options and values), bitfield and simple type. The global function map yields script functions, natives and console commands. The config variable registry yields every cvar; its validation range is read from the variable's MSVC RTTI type name.
4. **Layout.** Each type is laid out at its reflected offsets. The dumper computes the alignment C++ would give each type and the largest alignment every place that embeds it allows, and adds `alignas` or `#pragma pack(push, 0x1)` only where C++ would otherwise move a field or change a size. `Asserts.hpp` proves the result.
5. **Script variables.** Script objects keep their script variables in a separate heap buffer whose pointer sits at `IScriptable+0x20`, the same place the script VM reads them from. They are emitted as a nested `ScriptData` struct that extends the parent class's one.
6. **Console commands.** The game's script sources (`content`, `dlc` and `mods`) are scanned for each command's definition, which gives its file, line and category.

## Compatibility

Built and tested against The Witcher 3 next-gen **5.0.1048522** (DX12). Other next-gen builds most likely work as long as the five patterns still match; the log shows each anchor and its match count. The 2018 classic build (3.x) and Cyberpunk 2077 (REDengine 4) use different memory layouts and are not supported.

## License

[MIT](LICENSE)

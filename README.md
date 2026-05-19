# Uiana — Valorant Map Importer

Import Valorant maps into Unreal Engine from game archives—meshes, materials, lighting, and sublevels—without rebuilding levels by hand.

This is a **workflow automation tool** for content pipelines, not a game product. It saves hours of manual export/import when you need map geometry inside Unreal Engine.

## What it does

- Extracts map data from Valorant PAK files and imports into UE5
- Imports meshes, materials, textures, decals, and lighting
- Supports sublevels and bundled C# extractor (`cue4extractor`)
- Integrates with CUE4Parse and UEFormat (`.uemodel`) workflows

## Why it exists

Map porting was too slow to do manually. This plugin automates extraction and import so you can focus on editing instead of repetitive file handling.

## Quick start

1. [Download the latest release](https://github.com/djhaled/Uiana-MapImporter/releases) and extract the ZIP.
2. Install the [UEFormat](https://github.com/h4lfheart/UEFormat) plugin (unreal branch) into your project’s `Plugins/` folder.
3. Copy the `Uiana` folder into your UE project’s `Plugins/` directory (project plugins, not engine plugins).
4. Rebuild the project in Visual Studio if prompted.
5. Open the Uiana toolbar in the editor and follow the import workflow.

Clone with submodules:

```bash
git clone --recurse-submodules https://github.com/djhaled/Uiana-MapImporter.git
```

## Requirements

- Unreal Engine 5.0+
- .NET 6.0 Runtime
- Visual Studio 2019+ with C++ workload
- Valorant game files (PAK)
- UEFormat plugin; mesh assets as `.uemodel` (via FModel or CUE4Parse)

## Project status

Maintained; releases tagged (e.g. v1.05). See commit history and GitHub Releases.

## License

See repository license file. Use responsibly and in line with applicable terms for game assets you do not own.

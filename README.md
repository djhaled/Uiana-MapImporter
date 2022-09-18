<div id="top"></div>

<br />
<div align="center">
  <a href="https://github.com/djhaled/Uiana-MapImporter">
    <img src="HighresScreenshot00019.png" alt="Logo">
  </a>


  <p align="center">
    <br />
  </p>
</div>

# **Uiana**
Unreal Engine plugin for creating VALORANT content. Heavily inspired in Piana by Luviana.


## 📒 Requirements

Before you download the addon, you must download these first.

* [Download & Install .NET 6.0](https://dotnet.microsoft.com/en-us/download/dotnet/thank-you/runtime-6.0.5-windows-x64-installer)
* [Unreal 5.0+](https://www.unrealengine.com/en-US/download) 
* [Visual Studio 2019+](https://docs.unrealengine.com/4.26/en-US/ProductionPipelines/DevelopmentSetup/VisualStudioSetup/)


## 1. 🔧 Installation & Usage
1. **[Download latest](https://github.com/djhaled/Uiana-MapImporter/releases/download/1.03/Uiana-1.03.zip)**
2. Create a Plugins folder on your project if you don't already have
3. Drag "UIANA" and "UnrealPSKPSA" from the .zip file and put it in your **project's** "Plugins" folder (NOT IN ENGINE PLUGINS), create the folder if its not there
4. If asked to, rebuild your Unreal project in Visual Studio
5. Press Uiana button (Valorant Icon) on top of the UI to open the tool
6. Inside Uiana's UI, go to Settings and fill the inputs as follows.
   ```js
   Export Folder    = "Path to save your settings & export the maps"
                    // Pick an empty folder with a lot of space.
   PAKs Folder      = "Path to VALORANT's PAK files."
                    // ..Riot Games\VALORANT\live\ShooterGame\Content\Paks\
   ```
6. Select your preferable settings and hit "Generate Map" on bottom right.

## 2. 💾 Saving Import
1. Make sure to save ValorantContent folder & level after importing successfully

## 3. ⚡ Baking/Fixing lighting
1. Open "Build" tab on the top left
2. Change lighting to desired quality level (peview <-> production)
3. Press "Build Lighting Only"





## Contact 
* Discord
  - bK#6198
  - Zain#1873


## Credits
- Whole Piana team (Luviana,floxay,CoRe | Janik.M,Rata,Drice,Devo,Zertox)
- Zain (Scripting and Shader)
- TheyCallMeSpy (Shader and ideas)
- Patchzy (Shader and ideas)
- BK
- Halfuwu (PSKX importer)

**Uiana-MapImporter was created under Riot Games' "Legal Jibber Jabber" policy using assets owned by Riot Games.  Riot Games does not endorse or sponsor this project.**


<p align="right">(<a href="#top">back to top</a>)</p>



using System.Diagnostics;
using CUE4Parse.Encryption.Aes;
using CUE4Parse.FileProvider;
using CUE4Parse.UE4.Assets.Objects;
using CUE4Parse.UE4.Objects.Core.Misc;
using CUE4Parse.UE4.Objects.Meshes;
using CUE4Parse.UE4.Objects.UObject;
using CUE4Parse.UE4.Versions;
using CUE4Parse.Utils;
using Newtonsoft.Json;

namespace cue4extractor
{
    class Program
    {
        private static string[] GetUmapList(string mapName, string jsonPath)
        {
            var jsonContent = File.ReadAllText(jsonPath);
            var dict = JsonConvert.DeserializeObject<Dictionary<string, string[]>>(jsonContent);

            return dict[mapName];
        }


        /// <param name="gameDirectory">An option whose argument is parsed as an int</param>
        /// <param name="aesKey">An option whose argument is parsed as a bool</param>
        /// <param name="exportDirectory">An option whose argument is parsed as a bool</param>
        /// <param name="mapName">An option whose argument is parsed as a FileInfo</param>
        /// <param name="fileList">An option whose argument is parsed as a FileInfo</param>
        /// <param name="gameUmaps">An option whose argument is parsed as a FileInfo</param>
        /// <param name="GameVersion">An option whose argument is parsed as a FileInfo</param>
        private static void Main(
string allMeshes,string allextracts, string gameDirectory = @"D:\FortOLD\FortniteGame\Content\Paks",
            string aesKey = "0x2CCDFD22AD74FBFEE693A81AC11ACE57E6D10D0B8AC5FA90E793A130BC540ED4",
            string exportDirectory = @"D:\mapsCenat",
            string mapName = "athena_poi_communitypark_001",
            // string fileList = "D:\\__programming\\_github\\valorant-luvi\\export\\_datas\\ascent\\Ascent_Art_A_assets_obj.txt",
            string fileList = "D:\\ExportFortUE\\maps\\athena_poi_communitypark_001\\_assets_actors.txt",
            string gameUmaps = @"C:\Users\BERNA\Documents\Unreal Projects\BLANK\Plugins\Uiana\Content\Python\assets\umaps.json",
            EGame GameVersion = EGame.GAME_UE4_19
            )
        {
            //Console.WriteLine($"CUE4PARSE -  {gameDirectory}");
            //Console.WriteLine($"CUE4PARSE -  {aesKey}");
            //Console.WriteLine($"CUE4PARSE -  {exportDirectory}");
            //Console.WriteLine($"CUE4PARSE -  {mapName}");
            //Console.WriteLine($"CUE4PARSE -  {fileList}");
            //Console.WriteLine($"CUE4PARSE -  {gameUmaps}");
            //Console.WriteLine($"CUE4PARSE -  {GameVersion}");
            //Thread.Sleep(100000);
            var versions = new VersionContainer(GameVersion);
            var provider = new DefaultFileProvider(gameDirectory, SearchOption.AllDirectories, true, versions);
            provider.Initialize();
            provider.SubmitKey(new FGuid(), new FAesKey(aesKey));
            // provider.LoadMappings();
            provider.LoadLocalization(ELanguage.English);

            var selectedMap = mapName.ToLower() switch
            {
                "bind" => "Duality",
                "ascent" => "Ascent",
                "haven" => "Triad",
                "icebox" => "Port",
                "split" => "Bonsai",
                "breeze" => "FoxTrot",
                "fracture" => "Canyon",
                "range" => "Poveglia",
                "pearl" => "Pitt",
                "character select" => "PregameV2",
                _ => "",
            };

            var folder = Path.Combine(exportDirectory);
            if (!Directory.Exists(folder))
                Directory.CreateDirectory(folder);

            var converters = new Dictionary<Type, JsonConverter>()
            {
                { typeof(FColorVertexBuffer), new FColorVertexBufferCustomConverter() }
            };
            var settings = new JsonSerializerSettings { ContractResolver = new CustomResolver(converters) };

            if (fileList.Length > 0)
            {
                var sWatch = Stopwatch.StartNew();
                var currentList = fileList;

                var filename = Path.GetFileNameWithoutExtension(fileList);
                var folderType = filename switch
                {
                    "_assets_objects" => "Objects",
                    "_assets_actors" => "Actors",
                    "_assets_materials" => "Materials",
                    "_assets_materials_ovr" => "Override Materials",
                    _ => "",
                };
                if (File.Exists(currentList))
                    foreach (var line in File.ReadLines(currentList))
                    {
                        var objName = line.Split('\\')[^1];
                        if (folderType == "Actors")
                        {
                            objName = line.Split('/')[^1];
                        }
                        //Console.WriteLine($"INFO - CUE4Parse - Extracting : {objName}");
                        var objExports = provider.LoadObjectExports(line);
                        var objJSON = JsonConvert.SerializeObject(objExports, Formatting.Indented, settings);

                        var objJSONPath = Path.Combine(folder, $"{objName}.json");

                        //Console.WriteLine($"INFO - CUE4Parse - {objName}");
                        File.WriteAllText(objJSONPath, objJSON);
                    }
                sWatch.Stop();
                var swms = sWatch.ElapsedMilliseconds;
                Console.WriteLine($"INFO - CUE4Parse - Extracted MAP : {folderType} / {swms}ms");
            }
            else
            {
                if (gameUmaps == "")
                    return;

                var umapList = GetUmapList(mapName.ToLower(), gameUmaps);

                foreach (var umap in umapList)
                {
                    var sWatch = Stopwatch.StartNew();

                    var umapInternalPath = $"{umap}";
                    var umapExportFull = provider.LoadObjectExports(umapInternalPath);
                    var filename = Path.GetFileNameWithoutExtension(umapInternalPath);
                    var umapJSON = JsonConvert.SerializeObject(umapExportFull, Formatting.Indented, settings);

                    var umapJSONPath = Path.Combine(folder, $"{filename}.json");

                    File.WriteAllText(umapJSONPath, umapJSON);
                    sWatch.Stop();
                    var swms = sWatch.ElapsedMilliseconds;
                    Console.WriteLine($"INFO - CUE4Parse - Extracted MAP : {filename} / {swms}ms");
                }
            }
        }
    }

    class FColorVertexBufferCustomConverter : JsonConverter<FColorVertexBuffer>
    {
        public override void WriteJson(JsonWriter writer, FColorVertexBuffer value, JsonSerializer serializer)
        {
            writer.WriteStartObject();

            writer.WritePropertyName("Data");
            // serializer.Serialize(writer, value.Data); // saving space and time by only writing as hex
            writer.WriteStartArray();
            foreach (var c in value.Data)
                writer.WriteValue(UnsafePrint.BytesToHex(c.R, c.G, c.B, c.A)); // we need alpha even if it's 1 or 0...
            writer.WriteEndArray();

            writer.WritePropertyName("Stride");
            writer.WriteValue(value.Stride);

            writer.WritePropertyName("NumVertices");
            writer.WriteValue(value.NumVertices);

            writer.WriteEndObject();
        }

        public override FColorVertexBuffer ReadJson(JsonReader reader, Type objectType, FColorVertexBuffer existingValue, bool hasExistingValue,
            JsonSerializer serializer)
        {
            throw new NotImplementedException();
        }
    }
}
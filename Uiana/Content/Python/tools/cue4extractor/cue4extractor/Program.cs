using System.Diagnostics;
using CUE4Parse.Encryption.Aes;
using CUE4Parse.FileProvider;
using CUE4Parse.UE4.Objects.UObject;
using CUE4Parse.UE4.Objects.Core.Misc;
using CUE4Parse.UE4.Objects.Meshes;
using CUE4Parse.UE4.Assets.Objects;
using CUE4Parse.UE4.Versions;
using CUE4Parse.Utils;
using Newtonsoft.Json;
using CUE4Parse.UE4.Objects.Engine;
using CUE4Parse.UE4.Assets.Exports;

namespace cue4extractor
{
    class Program
    {
        private static List<string> GetUmapList(string targetMapName, string jsonFilePath, DefaultFileProvider fileProvider)
        {
            var jsonContent = File.ReadAllText(jsonFilePath);
            var mapDictionary = JsonConvert.DeserializeObject<Dictionary<string, string[]>>(jsonContent);
            var targetMapPaths = mapDictionary[targetMapName][0];
            List<string> objectNames = new List<string>();

            UWorld targetMap = (UWorld)fileProvider.LoadObject(targetMapPaths);
            objectNames.Add(targetMap.Owner.Name);

            foreach (var streamingLevel in targetMap.StreamingLevels)
            {
                streamingLevel.TryLoad(out var level);
                UWorld levelWorldAsset = level.Get<UWorld>("WorldAsset");
                objectNames.Add(levelWorldAsset.Owner.Name);
            }

            return objectNames;
        }




        /// <param name="gameDirectory">An option whose argument is parsed as an int</param>
        /// <param name="aesKey">An option whose argument is parsed as a bool</param>
        /// <param name="exportDirectory">An option whose argument is parsed as a bool</param>
        /// <param name="mapName">An option whose argument is parsed as a FileInfo</param>
        /// <param name="fileList">An option whose argument is parsed as a FileInfo</param>
        /// <param name="gameUmaps">An option whose argument is parsed as a FileInfo</param>
        private static void Main(
            string gameDirectory = @"C:\Riot Games\VALORANT\live\ShooterGame\Content\Paks",
            string aesKey = "0x4BE71AF2459CF83899EC9DC2CB60E22AC4B3047E0211034BBABE9D174C069DD6",
            string exportDirectory = @"D:\TEtestze",
            string mapName = "kasbah",
            // string fileList = "D:\\__programming\\_github\\valorant-luvi\\export\\_datas\\ascent\\Ascent_Art_A_assets_obj.txt",
            string fileList = "",
            string gameUmaps = @"E:\Uiana-MapImporter\Uiana\Content\Python\assets\umaps.json"
            )
        {
            var versions = new VersionContainer(EGame.GAME_Valorant);
            var provider = new DefaultFileProvider(gameDirectory, SearchOption.AllDirectories, true, versions);
            provider.Initialize();
            provider.SubmitKey(new FGuid(), new FAesKey(aesKey));
            // provider.LoadMappings();
            provider.LoadLocalization(ELanguage.English);

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
                        var objExports = provider.LoadAllObjects(line);
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

                var umapList = GetUmapList(mapName.ToLower(), gameUmaps,provider);

                foreach (var umap in umapList)
                {
                    var sWatch = Stopwatch.StartNew();
                    var filename = Path.GetFileNameWithoutExtension($"{umap}");
                    var umap_Object = provider.LoadAllObjects(umap);
                    var umapJSON = JsonConvert.SerializeObject(umap_Object, Formatting.Indented, settings);
                    var umapJSONPath = Path.Combine(folder, $"{filename}.json");
                    File.WriteAllText(umapJSONPath, umapJSON);
                    sWatch.Stop();
                    Console.WriteLine($"INFO - CUE4Parse - Extracted MAP : {filename} / {sWatch.ElapsedMilliseconds}ms");
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

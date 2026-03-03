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
using CUE4Parse.UE4.Assets.Exports.Texture;
using CUE4Parse.UE4.Assets.Exports.StaticMesh;
using CUE4Parse.UE4.Assets.Exports.SkeletalMesh;
using CUE4Parse_Conversion;
using CUE4Parse_Conversion.Textures;
using CUE4Parse_Conversion.UEFormat.Enums;

namespace cue4extractor
{
    class Program
    {
        private static string ToProviderPath(string line)
        {
            var path = line.Trim().Replace('\\', '/');
            if (path.StartsWith("Game/", StringComparison.OrdinalIgnoreCase))
                return "ShooterGame/Content/" + path.Substring(5);
            return path;
        }

        private static string ToExportPath(string path)
        {
            path = path.Replace('\\', '/');
            if (path.StartsWith("ShooterGame/Content/", StringComparison.OrdinalIgnoreCase))
                return "Game/" + path.Substring(19);
            return path;
        }

        private static void ExportAssets(DefaultFileProvider provider, string fileList, string exportDirectory)
        {
            var options = new ExporterOptions
            {
                MeshFormat = EMeshFormat.UEFormat,
                TextureFormat = ETextureFormat.Png,
                LodFormat = ELodFormat.FirstLod
            };
            var baseDir = new DirectoryInfo(exportDirectory);
            if (!baseDir.Exists) baseDir.Create();
            int count = 0, skipped = 0;
            var sw = Stopwatch.StartNew();
            foreach (var line in File.ReadLines(fileList))
            {
                var trimmed = line.Trim();
                if (string.IsNullOrEmpty(trimmed)) continue;
                var loadPath = ToProviderPath(trimmed);
                try
                {
                    var exports = provider.LoadAllObjects(loadPath);
                    foreach (var exp in exports)
                    {
                        try
                        {
                            if (exp is UStaticMesh or USkeletalMesh)
                            {
                                var exporter = new Exporter(exp, options);
                                if (exporter.TryWriteToDir(baseDir, out _, out var savedFilePath))
                                {
                                    count++;
                                    var normalized = ToExportPath(savedFilePath);
                                    if (normalized != savedFilePath)
                                    {
                                        var fromFull = Path.Combine(baseDir.FullName, savedFilePath);
                                        var toFull = Path.Combine(baseDir.FullName, normalized);
                                        if (File.Exists(fromFull))
                                        {
                                            var toDir = Path.GetDirectoryName(toFull);
                                            if (!string.IsNullOrEmpty(toDir)) Directory.CreateDirectory(toDir);
                                            File.Move(fromFull, toFull, true);
                                        }
                                    }
                                }
                            }
                            else if (exp is UTexture2D tex)
                            {
                                try
                                {
                                    var decoded = tex.Decode(ETexturePlatform.DesktopMobile);
                                    var pngBytes = decoded.Encode(ETextureFormat.Png, options.ExportHdrTexturesAsHdr, out var ext);
                                    if (pngBytes == null || pngBytes.Length == 0) { skipped++; continue; }
                                    var exportPath = (exp.Owner?.Provider?.FixPath(exp.Owner.Name) ?? exp.GetPathName()).SubstringBeforeLast('.');
                                    if (exportPath.StartsWith('/')) exportPath = exportPath[1..];
                                    exportPath = ToExportPath(exportPath) + "." + ext;
                                    var outPath = Path.Combine(baseDir.FullName, exportPath);
                                    var outDir = Path.GetDirectoryName(outPath);
                                    if (!string.IsNullOrEmpty(outDir)) Directory.CreateDirectory(outDir);
                                    File.WriteAllBytes(outPath, pngBytes);
                                    count++;
                                }
                                catch { skipped++; }
                            }
                        }
                        catch (NotSupportedException) { skipped++; }
                        catch (Exception) { skipped++; }
                    }
                }
                catch (Exception) { /* skip failed loads */ }
            }
            sw.Stop();
            Console.WriteLine($"INFO - CUE4Parse - Exported assets : {count} (skipped {skipped}) / {sw.ElapsedMilliseconds}ms");
        }

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
        /// <param name="exportAssets">When true and fileList is set, export meshes (.uemodel) and textures (.png) instead of JSON</param>
        private static void Main(
            string gameDirectory = @"C:\Riot Games\VALORANT\live\ShooterGame\Content\Paks",
            string aesKey = "0x4BE71AF2459CF83899EC9DC2CB60E22AC4B3047E0211034BBABE9D174C069DD6",
            string exportDirectory = @"D:\TEtestze",
            string mapName = "kasbah",
            string fileList = "",
            string gameUmaps = @"E:\Uiana-MapImporter\Uiana\Content\Importer\assets\umaps.json",
            bool exportAssets = false
            )
        {
            var versions = new VersionContainer(EGame.GAME_Valorant);
            var provider = new DefaultFileProvider(gameDirectory, SearchOption.AllDirectories, true, versions);
            provider.Initialize();
            provider.SubmitKey(new FGuid(), new FAesKey(aesKey));
            provider.LoadLocalization(ELanguage.English);

            var folder = Path.Combine(exportDirectory);
            if (!Directory.Exists(folder))
                Directory.CreateDirectory(folder);

            if (exportAssets && !string.IsNullOrEmpty(fileList) && File.Exists(fileList))
            {
                ExportAssets(provider, fileList, exportDirectory);
                return;
            }

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

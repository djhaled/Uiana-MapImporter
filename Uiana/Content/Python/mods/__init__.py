import argparse
from pathlib import Path

from mods.main import *

parser = argparse.ArgumentParser()
parser.add_argument('ImportBlueprints', type=int, help='ImportBlueprints')
parser.add_argument('ManualLMResMult', type=str, help='Manually Increase Lightmap Resolution')
parser.add_argument('ImportSubLevels', type=int, help='ImportSubLevels')
parser.add_argument('ImportMesh', type=int, help='ImportMesh')
parser.add_argument('ImportMaterial', type=int, help='ImportMaterial')
parser.add_argument('ImportDecal', type=int, help='ImportDecal')
parser.add_argument('ImportLights', type=int, help='ImportLights')
parser.add_argument('MapName', type=str, help='MapName')
parser.add_argument('ExportPath', type=str, help='ExportPath')
parser.add_argument('PakFolder', type=str, help='PakFolder')
parser.add_argument('PluginPath', type=str, help='PluginPath')
args = parser.parse_args()

#def get_valo_path():
    ## open yaml file and parse it

    
class UeSettings:
    bImportMaterial = bool(args.ImportMaterial)
    bImportMesh = bool(args.ImportMesh)
    bImportDecal = bool(args.ImportDecal)
    bImportLights = bool(args.ImportLights)
    bImportBlueprint = bool(args.ImportBlueprints)
    fMapName = args.MapName
    if fMapName == "characterSelect":
        fMapName = "character select"
    vAesKey = "0x4BE71AF2459CF83899EC9DC2CB60E22AC4B3047E0211034BBABE9D174C069DD6"
    iManualLMResMult = float(args.ManualLMResMult)
    bImportSubLevels = bool(args.ImportSubLevels)
    PExportPath = Path(args.ExportPath)
    PakFolha = args.PakFolder
    PPakFolder = Path(PakFolha)
    NewPath = (args.PluginPath) + ('\\Uiana\\Content\\Python')
    FixedPath = NewPath
    PPluginPath = Path(FixedPath)


import_map(UeSettings)
exit()

import argparse
from pathlib import Path
from mods.liana_main import *
parser = argparse.ArgumentParser()
parser.add_argument('ImportSubLevels',type=int,help='ImportSubLevels')
parser.add_argument('ImportMesh',type=int,help='ImportMesh')
parser.add_argument('ImportMaterial',type=int,help='ImportMaterial')
parser.add_argument('ImportDecal',type=int,help='ImportDecal')
parser.add_argument('ImportLights',type=int,help='ImportLights')
parser.add_argument('MapName',type=str,help='MapName')
parser.add_argument('ExportPath',type=str,help='ExportPath')
parser.add_argument('PakFolder',type=str,help='PakFolder')
parser.add_argument('PluginPath',type=str,help='PluginPath')
parser.add_argument('GameVersion',type=str,help='GameVerison')
parser.add_argument('vaAesKey',type=str,help='vaAesKey')
args = parser.parse_args()

class UeSettings:
	bImportMaterial = bool(args.ImportMaterial)
	bImportMesh = bool(args.ImportMesh)
	bImportDecal = bool(args.ImportDecal)
	bImportLights = bool(args.ImportLights)
	fMapName = args.MapName
	if fMapName == "characterSelect":
		fMapName = "character select"
	vAesKey = args.vaAesKey
	bImportSubLevels = bool(args.ImportSubLevels)
	PExportPath = Path(args.ExportPath)
	PakFolha = args.PakFolder
	PPakFolder = Path(PakFolha)
	NewPath = (args.PluginPath) + ('\\Uiana\\Content\\Python')
	FixedPath = NewPath
	PPluginPath = Path(FixedPath)
	GVersion =args.GameVersion
import_map(UeSettings)
exit()

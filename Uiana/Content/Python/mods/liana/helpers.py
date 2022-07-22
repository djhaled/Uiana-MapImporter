
import os
import json
from pathlib import Path
from subprocess import run
from shutil import rmtree
from collections.abc import Iterable
from enum import Enum
import requests
import argparse
import time
start_time = time.time()


FILEBROWSER_PATH = os.path.join(os.getenv('WINDIR'), 'explorer.exe')


# ANCHOR: Functions
# -------------------------- #

def MeasureTime():
	return (time.time() - start_time)
def HowMuchTimeTookFunc(value1, value2):
	took = value2 - value1
	return float(took)
def open_folder(path):
	"""
	Open a file explorer to a path
	:param path: path to folder
	:return:
	"""
	path = os.path.normpath(path)

	if os.path.isdir(path):
		run([FILEBROWSER_PATH, path])


def get_files(path: str, extension: str = "") -> list:
	"""
	Get all files in a directory
	:param path: path to directory
	:param extension: extension of files to get
	:return: list of files
	"""
	files = list()
	for file in os.listdir(path):
		if extension in file:
			files.append(Path(os.path.join(path, file)))
	return files


def remove_file(path: str):
	"""
	Remove a file
	"""
	if os.path.isfile(path) or os.path.islink(path):
		os.remove(path)  # remove the file
	elif os.path.isdir(path):
		rmtree(path)  # remove dir and all contains
	else:
		raise ValueError("file {} is not a file or dir.".format(path))


def open_folder(path):
	"""
	Open a file explorer to a path
	:param path: path to folder
	:return:
	"""
	path = os.path.normpath(path)

	if os.path.isdir(path):
		run([FILEBROWSER_PATH, path])


def save_list(filepath: Path, lines: list):
	"""
	Save a list to a file
	:param filepath: path to file
	:param lines: list of lines
	:return:
	"""

	# Flatten umap objects
	lines = list(flatten_list(lines))

	# Remove Duplicates
	lines = list(dict.fromkeys(lines))

	with open(filepath.__str__(), 'w') as f:
		f.write('\n'.join(lines))
	return filepath.__str__()


def save_json(p: str, d):
	"""
	Save a dictionary to a json file
	:param p: path to file
	:param d: dictionary
	:return:
	"""
	with open(p, 'w') as jsonfile:
		json.dump(d, jsonfile, indent=4)


def read_json(p: str) -> dict:
	"""
	Read a json file and return a dictionary
	:param p: path to file
	:return:
	"""
	with open(p) as json_file:
		return json.load(json_file)

def GetAttachScene(obj,OuterName,umapfile):
	types = ["SceneComponent","BrushComponent", "StaticMeshComponent"]
	if OuterName == 'PersistentLevel':
		OuterName = obj["Name"]
	for j in umapfile:
		tipo = j["Type"]
		outer = j["Outer"]
		if outer == "PersistentLevel":
			outer = j["Name"]
		#print(f'OuterName trying to find is {OuterName} and current outer is {outer} // also tipo is {tipo}')
		if outer == OuterName and tipo in types:
			return j
	#exit()

def IsBlockingVolume(obj,OuterName,umapfile):
	for gama in umapfile:
		if OuterName == gama["Name"]:
			return True
	return False
def GetBlockingMesh(obj,OuterName,umapfile):
	for gama in umapfile:
		if gama["Type"] == "StaticMeshComponent" and obj["Name"] == gama["Outer"]:
			return gama
def shorten_path(file_path, length) -> str:
	"""
	Shorten a path to a given length
	:param file_path: path to shorten
	:param length: length to shorten to
	:return: shortened path
	"""
	return f"..\{os.sep.join(file_path.split(os.sep)[-length:])}"


def flatten_list(collection):
	"""
	Flatten a list of lists
	:param collection: list of lists
	:return: list
	"""

	for x in collection:
		if isinstance(x, Iterable) and not isinstance(x, str):
			yield from flatten_list(x)
		else:
			yield x



def create_folders(self):
	for attr, value in self.__dict__.items():
		if "path" in attr:
			f = Path(value)
			if not f.exists():
				print(f"Creating folder {f}")
				f.mkdir(parents=True)


# ANCHOR: Classes
# -------------------------- #


class BlendMode(Enum):
	OPAQUE = 0
	CLIP = 1
	BLEND = 2
	HASHED = 3


def get_umap_list() -> list:
	a = requests.get("https://gist.githubusercontent.com/luvyana/d5d7b2be0d33f9d213067f06ec681bd8/raw/cd34145908eb2e936065d10f3b9b570c7d5c7353/umaps.json").json()
	return a




class Settings:
	def __init__(self,UESet):
		self.aes = UESet.vAesKey
		self.texture_format = ".png"
		########## have to fix so it gets actual dir
		self.script_root = UESet.PPluginPath
		self.tools_path = self.script_root.joinpath("tools")
		self.importer_assets_path = self.script_root.joinpath("assets")
		self.paks_path = UESet.PPakFolder
		self.import_decals = UESet.bImportDecal
		self.import_lights = UESet.bImportLights
		self.import_Misc = UESet.bImportMisc
		self.import_Mesh = UESet.bImportMesh
		self.import_materials = UESet.bImportMaterial
		self.combine_umaps = False
		self.export_path = UESet.PExportPath
		self.assets_path = self.export_path.joinpath("export")
		self.maps_path = self.export_path.joinpath("maps")
		self.umodel = self.script_root.joinpath("tools", "umodel.exe")
		self.debug = False
		self.cue4extractor = self.script_root.joinpath("tools", "cue4extractor.exe")
		self.log = self.export_path.joinpath("import.log")
		self.umap_list_path = self.importer_assets_path.joinpath("umaps.json")
		self.umap_list = read_json(self.umap_list_path)
		
		self.selected_map = Map(UESet.fMapName, self.maps_path, self.umap_list)

		self.shaders = [
			"VALORANT_Base",
			"VALORANT_Decal",
			"VALORANT_Emissive",
			"VALORANT_Emissive_Scroll",
			"VALORANT_Hologram",
			"VALORANT_Glass",
			"VALORANT_Blend",
			"VALORANT_Decal",
			"VALORANT_MRA_Splitter",
			"VALORANT_Normal_Fix",
			"VALORANT_Screen"
		]

		create_folders(self)


class Map:
	def __init__(self, selected_map_name: str, maps_path: Path, all_umaps: list):

		self.name = selected_map_name
		# print(maps_path, self.name)
		self.folder_path = maps_path.joinpath(self.name)

		self.umaps = all_umaps[self.name]
		# print(self)
		self.materials_path = self.folder_path.joinpath("materials")
		self.materials_ovr_path = self.folder_path.joinpath("materials_ovr")
		self.objects_path = self.folder_path.joinpath("objects")
		self.scenes_path = self.folder_path.joinpath("scenes")
		self.umaps_path = self.folder_path.joinpath("umaps")
		create_folders(self)
		self.import_decals = True
		self.import_lights = False
		self.combine_umaps = False
		self.export_path = Path('D:\XportPsk')
		self.assets_path = self.export_path.joinpath("export")
		self.maps_path = self.export_path.joinpath("maps")
		self.umodel = self.script_root.joinpath("tools", "umodel.exe")
		self.debug = False
		self.cue4extractor = self.script_root.joinpath("tools", "cue4extractor.exe")
		self.log = self.export_path.joinpath("import.log")
		self.umap_list_path = self.importer_assets_path.joinpath("umaps.json")
		self.umap_list = read_json(self.umap_list_path)
		
		self.selected_map = Map('bind', self.maps_path, self.umap_list)

		self.shaders = [
			"VALORANT_Base",
			"VALORANT_Decal",
			"VALORANT_Emissive",
			"VALORANT_Emissive_Scroll",
			"VALORANT_Hologram",
			"VALORANT_Glass",
			"VALORANT_Blend",
			"VALORANT_Decal",
			"VALORANT_MRA_Splitter",
			"VALORANT_Normal_Fix",
			"VALORANT_Screen"
		]

		create_folders(self)


class Map:
	def __init__(self, selected_map_name: str, maps_path: Path, all_umaps: list):

		self.name = selected_map_name
		# print(maps_path, self.name)
		self.folder_path = maps_path.joinpath(self.name)

		self.umaps = all_umaps[self.name]
		# print(self)
		self.materials_path = self.folder_path.joinpath("materials")
		self.materials_ovr_path = self.folder_path.joinpath("materials_ovr")
		self.objects_path = self.folder_path.joinpath("objects")
		self.scenes_path = self.folder_path.joinpath("scenes")
		self.umaps_path = self.folder_path.joinpath("umaps")
		create_folders(self)

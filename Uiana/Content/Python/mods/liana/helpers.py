
import os
import json
from pathlib import Path
from subprocess import run
from collections.abc import Iterable
import time
import unreal
SELECTIVE_OBJECTS = []
projectpath = unreal.Paths.project_plugins_dir()
newpath = projectpath + 'Uiana/Content/Python/assets/umapTYPE.json'
f = open(newpath)
JsonMapTypeData = json.load(f)
FILEBROWSER_PATH = os.path.join(os.getenv('WINDIR'), 'explorer.exe')

# -------------------------- #
def ClearLevel():
	AllActors = unreal.EditorLevelLibrary.get_all_level_actors()
	for j in AllActors:
		unreal.EditorLevelLibrary.destroy_actor(j)

def SetCubeMapTexture(Seting):
	pathCube = Seting["ObjectName"]
	newtext = pathCube.replace("TextureCube ","")
	AssetPath = (f'/Uiana/CubeMaps/{newtext}.{newtext}')
	TextureCubeMap = unreal.load_asset(AssetPath)
	return TextureCubeMap
def SetIesTexture(setting):
	pathIES = setting["ObjectName"]
	StartNewTextureName = pathIES
	NewTextureName = ReturnFormattedString(StartNewTextureName,"_")
	AssetPath = (f'/Uiana/IESProfiles/{NewTextureName}.{NewTextureName}')
	TextureIES = unreal.load_asset(AssetPath)
	return TextureIES
def SetMaterialVectorValue(Mat,ParamName,Value):
	unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(Mat,ParamName,Value)
	unreal.MaterialEditingLibrary.update_material_instance(Mat)
def SetMaterialScalarValue(Mat,ParamName,Value):
	unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(Mat,ParamName,Value)
	unreal.MaterialEditingLibrary.update_material_instance(Mat)
def GetReadableUMapType(mapname):
	for j in JsonMapTypeData:
		NewMapName = j["Name"]
		MapType = j["StreamingType"]
		if mapname == NewMapName:
			return MapType
BLACKLIST = [
	"navmesh",
	"_breakable",
	"_collision",
	"windstreaks_plane",
	"sm_port_snowflakes_boundmesh",
	"sm_barrierduality",
	"M_Pitt_Caustics_Box",
	"box_for_volumes", 
	"supergrid",
	"_col",
	"M_Pitt_Lamps_Glow",
	"for_volumes",
]
def GetUMapType(mapname):
	for j in JsonMapTypeData:
		NewMapName = j["Name"]
		MapType = j["StreamingType"]
		if mapname == NewMapName:
			return eval(f'unreal.{MapType}')
def ImportShader(Shader):
	BaseShader = unreal.load_asset(f'/Uiana/Materials/{Shader}')
	return BaseShader

def ReturnObjectName(name):
	rformPar =name.rfind(' ') + 1
	return name[rformPar:len(name)]
def import_shaders():
	BaseShader = unreal.load_asset('/Uiana/Materials/ValoOpaqueMasterNEW')
	return BaseShader

def importDecalShaders():
	BaseShader = unreal.load_asset('/Uiana/Materials/ValoDecals')
	return BaseShader
def ConvertToLoadableMaterial(Mesh,Type):
	typestring = str(Type)
	NewName = Mesh.replace(f'{Type}', "")
	return NewName
def ConvertToLoadableUE(Mesh,Type,ActualType):
	if Mesh == None:
		return None
	Name = Mesh["ObjectName"]
	typestring = str(Type)
	NewName = Name.replace(f'{Type}', "")
	PathToGo = f'/Game/ValorantContent/{ActualType}/{NewName}'
	return PathToGo
def GetTransform(Prop):
	TransformData = None
	bIsInstanced = False
	Props = Prop
	Quat = unreal.Quat()
	if HasKey("TransformData",Props):
		TransformData = Props["TransformData"]
		bIsInstanced = True
	if HasKey("RelativeLocation",Props) or HasKey("OffsetLocation",Props) or HasKey("Translation",TransformData) :
		if bIsInstanced:
			Location = TransformData["Translation"]
		else:
			Location =  Props["RelativeLocation"] if HasKey("RelativeLocation", Props) else Props["OffsetLocation"]
		LocationUnreal = unreal.Vector(Location["X"],Location["Y"],Location["Z"])
	else:
		LocationUnreal = unreal.Vector(0.0,0.0,0.0)

	if HasKey("RelativeScale3D",Props) or HasKey("Scale3D",TransformData):
		if bIsInstanced:
			Scale =  TransformData["Scale3D"]
			ScaleUnreal = unreal.Vector(Scale["X"],Scale["Y"],Scale["Z"])
		else:
			Scale =  Props["RelativeScale3D"]
			ScaleUnreal = unreal.Vector(Scale["X"],Scale["Y"],Scale["Z"])
	else:
		ScaleUnreal = unreal.Vector(1.0,1.0,1.0)
	if HasKey("RelativeRotation",Props) or HasKey("Rotation",TransformData):
		if bIsInstanced:
			Rotation = TransformData["Rotation"]
			Quat = unreal.Quat(Rotation["X"],Rotation["Y"],Rotation["Z"],Rotation["W"])
			RotationUnreal = unreal.Rotator(0.0,0.0,0.0)
		else:
			Rotation = Props["RelativeRotation"]
			RotationUnreal = unreal.Rotator(Rotation["Roll"],Rotation["Pitch"],Rotation["Yaw"])
	else:
		RotationUnreal = unreal.Rotator(0.0,0.0,0.0)
	Trans = unreal.Transform(LocationUnreal, RotationUnreal, ScaleUnreal)
	if bIsInstanced:
		Trans.set_editor_property("rotation",Quat)
	return Trans
def HasKey(key,array):
	if array == None:
		return False
	if key in array:
		return True
	else:
		return False
def returnUnrealVector(prop):
	vec = unreal.Vector(prop["X"],prop["Y"],prop["Z"])
	return vec
def GetClassName(self):
	return type(self).__name__
def returnUnrealRotator(prop):
	Quat = unreal.Quat(x=prop["X"], y=prop["Y"], z=prop["Z"], w=prop["W"])
	rot = Quat.rotator()
	return rot
def ReturnFormattedString(string,prefix):
	start = string.rfind(prefix) + 1
	end = len(string)
	return string[start:end]
def HasSetting(asset,comp,black):
	if asset == "LightmassSettings":
		return True
	if asset[0] == "b":
		asset= asset[1:len(asset)]
	asset = asset.lower()
	propdir = dir(comp)
	for findprop in propdir:
		noslashes = findprop.replace("_","")
		if noslashes == asset:
			if asset in black:
				return False
			return True
	return False
def HasTransform(prop):
	bFactualBool = False
	if HasKey("RelativeLocation",prop):
		bFactualBool = True
	if HasKey("RelativeRotation",prop):
		bFactualBool = True
	if HasKey("RelativeScale3D",prop):
		bFactualBool = True
	if bFactualBool :
		return GetTransform(prop)
	return bFactualBool
def GetInitialName(ka):
	slash = ka.find('_')
	if slash == -1:
		return ka
	lenka = len(ka)
	return ka[0:slash].lower()

def FindNonSlasher(dictstuff, value):
	dact = dir(dictstuff)
	if type(value) == list:
		return None
	newvalue = value.replace("_","")
	for joga in dact:
		noslash = joga.replace("_","")
		uppernoslash = noslash.upper()
		if uppernoslash == newvalue:
			return joga
def filter_objects(umap_DATA, lights: bool = False) -> list:

	objects = umap_DATA
	filtered_list = []

	# Debug check
	if SELECTIVE_OBJECTS:
		for filter_model_name in SELECTIVE_OBJECTS:
			for og_model in objects:
				object_type = get_object_type(og_model)
				if object_type == "mesh":
					if filter_model_name in og_model["Properties"]["StaticMesh"]["ObjectPath"]:
						og_model["Name"] = og_model["Properties"]["StaticMesh"]["ObjectPath"]
						filtered_list.append(og_model)

				elif object_type == "decal":
					if filter_model_name in og_model["Outer"]:
						og_model["Name"] = og_model["Outer"]
						filtered_list.append(og_model)

				elif object_type == "light":
					if filter_model_name in og_model["Outer"]:
						og_model["Name"] = og_model["Outer"]
						filtered_list.append(og_model)

	else:
		filtered_list = objects

	new_list = []

	def is_blacklisted(object_name: str) -> bool:
		for blocked in BLACKLIST:
			if blocked.lower() in object_name.lower():
				return True
		return False

	# Check for blacklisted items
	for og_model in filtered_list:
		model_name_lower = get_obj_name(data=og_model, mat=False).lower()

		if is_blacklisted(model_name_lower):
			continue
		else:
			new_list.append(og_model)

	return new_list

def get_obj_name(data: dict, mat: bool):
	if mat:
		s = data["ObjectPath"]
	else:
		if HasKey("Properties",data) == False:
			return "None"
		if "StaticMesh" in data["Properties"]:
			s = data["Properties"]["StaticMesh"]["ObjectPath"]
		else:
			s = data["Outer"]
	k = get_name(s)
	return k
def get_name(s: str) -> str:
	return Path(s).stem

def cast(object_to_cast=None, object_class=None):
	try:
		return object_class.cast(object_to_cast)
	except:
		return None
def PrintExecutionTime(number):
	print(f"--- %s seconds{number} ---" % (time.time() - start_time))
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
			return HasTransform(j["Properties"])
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
		self.import_Mesh = UESet.bImportMesh
		self.import_materials = UESet.bImportMaterial
		self.import_sublevel = UESet.bImportSubLevels
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

class ActorDefs():
	def __init__(self,Actor):
		self.name = Actor["Name"]
		self.type = Actor["Type"]
		self.props = Actor["Properties"]
		self.outer = Actor["Outer"]
		self.transform = HasTransform(self.props)

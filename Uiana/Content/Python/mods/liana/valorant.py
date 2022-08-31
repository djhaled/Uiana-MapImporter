
from pathlib import Path
from .helpers import *


def filter_umap(umap_data: dict) -> list:
	umap_filtered = list()

	mesh_types = ["staticmesh", "staticmeshcomponent", "instancedstaticmeshcomponent", "hierarchicalinstancedstaticmeshcomponent"]
	gen_types = ['pointlightcomponent',"postprocessvolume" ,"culldistancevolume","scenecomponent","lightmasscharacterindirectdetailvolume","brushcomponent","precomputedvisibilityvolume",'rectlightcomponent', 'spotlightcomponent', 'skylightcomponent',  'scenecapturecomponentcube',"lightmassimportancevolume","billboardcomponent", 'directionallightcomponent', 'exponentialheightfogcomponent', 'lightmassportalcomponent', 'spherereflectioncapturecomponent']
	object_types = []
	decal_types = ["decalcomponent"]

	for obj in umap_data:
		ObjType = obj["Type"]
		object_types.append(ObjType)
		if ObjType.lower() in mesh_types:
			if "Properties" in obj:
				umap_filtered.append(obj)

		if ObjType.lower() in gen_types:
			umap_filtered.append(obj)
		if ObjType.lower() in decal_types:
			umap_filtered.append(obj)
		if ObjType.lower().endswith("_c"):	
			umap_filtered.append(obj)
		if "buildingfoundation" in ObjType.lower():
			umap_filtered.append(obj)


	return umap_filtered, object_types
def GetActualName(name):#\Game\Environments\Crag\Map\Crag_Game
	newlen = name.rfind('\\') + 1
	jc = name[newlen:len(name)]
	return jc

def get_objects(umap_data):
	umap_objects = list()
	umap_materials = list()
	allobjes = list()
	umap_actors = list()
	idx = 0 
	for obj in umap_data:
		if obj["Type"].endswith("_C") and HasKeyzin("Template",obj):	
			if idx == 0:	
				idx = idx + 1	
				continue	
			oType = obj["Template"]	
			umap_actors.append(oType)	
		if "Properties" in obj:
			if "StaticMesh" in obj["Properties"]:
				if obj["Properties"]["StaticMesh"] is not None:
					obj_path = get_object_path(data=obj, mat=False)
					umap_objects.append(obj_path)
					allobjes.append(fix_path(obj_path))

					if "OverrideMaterials" in obj["Properties"]:
						for mat in obj["Properties"]["OverrideMaterials"]:
							if mat is not None:
								ovrmat = get_object_path(data=mat, mat=True)
								umap_materials.append(ovrmat)
								allobjes.append(fix_path(ovrmat))

			elif "DecalMaterial" in obj["Properties"]:
				mat = obj["Properties"]["DecalMaterial"]
				if mat is not None:
					decalmat = get_object_path(data=mat, mat=True)
					umap_materials.append(decalmat)
					allobjes.append(fix_path(decalmat))

	return umap_objects, umap_materials, allobjes,umap_actors


def get_object_path(data: dict, mat: bool):
	if mat:
		s = data["ObjectPath"]
	else:
		s = data["Properties"]["StaticMesh"]["ObjectPath"]

	s = s.split(".", 1)[0].replace('/', '\\')
	return s


def get_object_type(model_data: dict) -> str:
	ModelType = model_data["Type"]
	lights = ["PointLightComponent","PostProcessVolume","PrecomputedVisibilityVolume","CullDistanceVolume", "RectLightComponent","LightmassCharacterIndirectDetailVolume", "SpotLightComponent","SkyLightComponent","LightmassImportanceVolume","SceneCaptureComponentCube","SphereReflectionCaptureComponent","DirectionalLightComponent","ExponentialHeightFogComponent","LightmassPortalComponent"]
	meshes = ["StaticMeshComponent", "InstancedStaticMeshComponent", "HierarchicalInstancedStaticMeshComponent"]
	decals = ["DecalComponent"]
	blueprint = ["SceneComponent"]
	if model_data["Type"] in meshes and HasKeyzin("StaticMesh",model_data["Properties"]) and  HasKeyzin("Template",model_data) == False:
		return "mesh"
	if model_data["Type"] in lights:
		return "light"
	if model_data["Type"] in decals:
		return "decal"
	if model_data["Type"].endswith("_C"):	
		return "blueprint"
	if "BuildingFoundation" in model_data["Type"]:
		return "foundation"


def get_object_materials(model_json: dict):
	# model_json = _common.read_json(model)
	model_materials = list()
	allmates = list()
	for objm in model_json:
		if objm["Type"] == "StaticMesh":
			StaticMat = objm["Properties"]["StaticMaterials"]
			for mat in StaticMat:
				if mat is not None and "MaterialInterface" in mat:
					if mat["MaterialInterface"] is not None:
						material = mat["MaterialInterface"]
						MInter = get_object_path(data=material, mat=True)
						allmates.append(fix_path(MInter))
						model_materials.append(MInter)

	return model_materials, allmates


def fix_path(a: str):
	pathlist = a.split('\\')
	GameName = ''
	for idx,pl in enumerate(pathlist):
		if pl == "Content":
			GameName = pathlist[idx-1]
	b = a.replace(f"{GameName}\\Content", "Game")
	c = b.replace("Engine\\Content", "Engine")
	return c


def get_light_type(object):
	if "Point" in object["Type"]:
		return "POINT"
	if "Spot" in object["Type"]:
		return "SPOT"
	if "RectLightComponent" in object["Type"]:
		return "AREA"


def get_name(s: str) -> str:
	return Path(s).stem

def HasKeyzin(key,array):
	if key in array:
		return True
	else:
		return False



# ANCHOR Shaders

def get_valorant_shader(group_name: str):
	print("Shader")


# ANCHOR Getters

def get_rgb_255(pv: dict) -> tuple:
	return (
		pv["R"] / 255,
		pv["G"] / 255,
		pv["B"] / 255,
		pv["A"] / 255
	)


def get_rgb(pv: dict) -> tuple:
	return (
		pv["R"],
		pv["G"],
		pv["B"],
		pv["A"])


def get_texture_path(s: dict, f: str):
	ParamValue = s["ParameterValue"]
	if ParamValue == None:
		return None
	a = Path(os.path.splitext(s["ParameterValue"]["ObjectPath"])[0].strip("/")).__str__()
	b = fix_path(a=a) + f
	return b


def get_texture_path_yo(s: str, f: str):
	a = Path(os.path.splitext(s)[0].strip("/")).__str__()
	b = fix_path(a=a) + f
	return b


class MapObject(object):
	def __init__(self, settings: Settings, data: dict, umap_name: str,umap_data:list):
		self.umap = umap_name
		self.map_folder = settings.selected_map.folder_path
		self.objects_folder = settings.selected_map.objects_path
		self.data = data
		self.umapdata = umap_data
		self.name = self.get_object_name()
		self.objname = self.getOBJName()
		self.uname = self.get_object_game_name()
		self.object_path = self.get_object_path()
		self.json = self.get_object_data_OG()
		self.model_path = self.get_local_model_path(p=settings.assets_path)

	def get_local_model_path(self, p: Path) -> str:
		a = p.joinpath(os.path.splitext(self.data["Properties"]["StaticMesh"]["ObjectPath"])[0].strip("/")).__str__()
		return fix_path(a) + ".pskx"

	def get_object_name(self) -> str:
		s = self.data["Properties"]["StaticMesh"]["ObjectPath"]
		k = Path(s).stem
		return k
	def getOBJName(self)->str:
		s = self.data["Properties"]["StaticMesh"]["ObjectName"]
		news = s[s.rfind(" ")+1:len(s)]
		k = Path(news).stem
		return k
	def get_object_game_name(self) -> str:
		s = self.data["Outer"]
		return s

	def get_object_path(self, fix: bool = False) -> str:
		s = self.data["Properties"]["StaticMesh"]["ObjectPath"]
		s = s.split(".", 1)[0].replace('/', '\\')
		if fix:
			return fix_path(s)
		else:
			return s

	def is_instanced(self) -> bool:
		if "PerInstanceSMData" in self.data and "Instanced" in self.data["Type"]:
			return True
		else:
			return False

	def get_object_data_OG(self) -> dict:
		return read_json(self.objects_folder.joinpath(f"{self.name}.json"))

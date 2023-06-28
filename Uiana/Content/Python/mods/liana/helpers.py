import json
import os
import re
import time
from collections.abc import Iterable
from pathlib import Path
from subprocess import run

import unreal

SELECTIVE_OBJECTS = []
projectpath = unreal.Paths.project_plugins_dir()
newpath = projectpath + 'Uiana/Content/Python/assets/umapTYPE.json'
f = open(newpath)
JsonMapTypeData = json.load(f)
FILEBROWSER_PATH = os.path.join(os.getenv('WINDIR'), 'explorer.exe')


# -------------------------- #
def clear_level():
    AllActors = unreal.EditorLevelLibrary.get_all_level_actors()
    for j in AllActors:
        unreal.EditorLevelLibrary.destroy_actor(j)


def ReturnBPLoop(data, name):
    for lop in data:
        if lop["Name"] == name:
            return lop

def return_parent(parent_name):
    actual_shader_name = parent_name[parent_name.rfind(' ')+1:len(parent_name)]
    DefEnv = import_shader(actual_shader_name)
    if not DefEnv:
        ParentName = "BaseEnv_MAT_V4"
    else:
        ParentName = actual_shader_name
    return ParentName
def reduce_bp_json(BigData):
    FullJson = {}
    newjson = []
    sceneroot = []
    ChildNodes = []
    GameObjects = []
    for fnc in BigData:
        fName = fnc["Name"]
        fType = fnc["Type"]
        if "Properties" not in fnc:
            continue
        FProps = fnc["Properties"]
        if fType == "SimpleConstructionScript":
            SceneRoot = FProps["DefaultSceneRootNode"]["ObjectName"]
            sceneroot.append(SceneRoot[SceneRoot.rfind(':') + 1:len(SceneRoot)])
        if "Node" in fName:
            Name = FProps["ComponentTemplate"]["ObjectName"]
            ActualName = Name[Name.rfind(':') + 1:len(Name)]
            Component = ReturnBPLoop(BigData, ActualName)
            FProps["CompProps"] = Component["Properties"] if "Properties" in Component else None
            newjson.append(fnc)
            if has_key("ChildNodes", FProps):
                for CN in FProps["ChildNodes"]:
                    ChildObjectName = CN["ObjectName"]
                    ChildName = ChildObjectName[ChildObjectName.rfind('.') + 1:len(ChildObjectName)]
                    ChildNodes.append(ChildName)
        if fName == "GameObjectMesh":
            GameObjects.append(fnc)
    FullJson["Nodes"] = newjson
    FullJson["SceneRoot"] = sceneroot
    FullJson["GameObjects"] = GameObjects
    FullJson["ChildNodes"] = ChildNodes
    return FullJson

def check_export(settings):
    exp_path = settings.selected_map.folder_path.joinpath("exported.yo")
    if exp_path.exists():
        exp_data = json.load(open(exp_path))
        val_version = exp_data[0]
        current_val_version = get_valorant_version()
        if settings.val_version != val_version or settings.dev_force_reexport:
            return True
    else:
        return True
    return False
        
def write_export_file():
    new_json = [get_valorant_version()]
    json_object = json.dumps(new_json, indent=4)
    return json_object
    
def get_cubemap_texture(Seting):
    pathCube = Seting["ObjectName"]
    newtext = pathCube.replace("TextureCube ", "")
    AssetPath = (f'/Uiana/CubeMaps/{newtext}.{newtext}')
    TextureCubeMap = unreal.load_asset(AssetPath)
    return TextureCubeMap

def get_valorant_version():
    val_file = "C:\\ProgramData\\Riot Games\\Metadata\\valorant.live\\valorant.live.ok"
    if os.path.exists(val_file):
        with open(val_file, "r") as f:
            file = f.read()
            split_file = file.split('\n')
            return split_file[0].split('/')[-1].split('.')[0]
    else:
        return None
def get_ies_texture(setting):
    pathIES = setting["ObjectName"]
    StartNewTextureName = pathIES
    NewTextureName = return_formatted_string(StartNewTextureName, "_")
    AssetPath = (f'/Uiana/IESProfiles/{NewTextureName}.{NewTextureName}')
    TextureIES = unreal.load_asset(AssetPath)
    return TextureIES


def set_material_vector_value(Mat, ParamName, Value):
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(Mat, ParamName, Value)
    unreal.MaterialEditingLibrary.update_material_instance(Mat)


def set_material_scalar_value(Mat, ParamName, Value):
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(Mat, ParamName, Value)
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
    "M_Pitt_Caustics_Box",
    "box_for_volumes",
    "BombsiteMarker_0_BombsiteA_Glow",
    "BombsiteMarker_0_BombsiteB_Glow",
    "_col",
    "M_Pitt_Lamps_Glow",
    "SM_Pitt_Water_Lid",
    "Bombsite_0_ASiteSide",
    "Bombsite_0_BSiteSide"
    "For_Volumes",
    "Foxtrot_ASite_Plane_DU",
    "Foxtrot_ASite_Side_DU",
    "BombsiteMarker_0_BombsiteA_Glow",
    "BombsiteMarker_0_BombsiteB_Glow",
    "DirtSkirt",
    "Tech_0_RebelSupplyCargoTarpLargeCollision",
]



def get_umap_type(mapname):
    for j in JsonMapTypeData:
        NewMapName = j["Name"]
        MapType = j["StreamingType"]
        if mapname == NewMapName:
            return eval(f'unreal.{MapType}')


def import_shader(Shader):
    BaseShader = unreal.load_asset(f'/Uiana/Materials/{Shader}')
    return BaseShader


def return_object_name(name):
    rformPar = name.rfind(' ') + 1
    return name[rformPar:len(name)]



def mesh_to_asset(Mesh, Type, ActualType):
    if Mesh == None:
        return None
    Name = Mesh["ObjectName"]
    typestring = str(Type)
    NewName = Name.replace(f'{Type}', "")
    PathToGo = f'/Game/ValorantContent/{ActualType}/{NewName}'
    print(Mesh)
    asset = unreal.load_asset(PathToGo)
    print(asset)
    return asset


def path_convert(path: str) -> str:
    b, c, rest = path.split("\\", 2)
    if b == "ShooterGame":
        b = "Game"
    if c == "Content":
        c = ""
    return "\\".join((b, c, rest))

def get_scene_transform(prop):
    quat = unreal.Quat()
    LocationUnreal = unreal.Vector(0.0, 0.0, 0.0)
    ScaleUnreal = unreal.Vector(1.0, 1.0, 1.0)
    RotationUnreal = unreal.Rotator(0.0, 0.0, 0.0)
    if has_key("SceneAttachRelativeLocation",prop):
        loc = prop["SceneAttachRelativeLocation"]
        LocationUnreal = unreal.Vector(loc["X"], loc["Y"], loc["Z"])
    if has_key("SceneAttachRelativeLocation",prop):
        rot = prop["SceneAttachRelativeRotation"]
        RotationUnreal = unreal.Rotator(rot["Roll"], rot["Pitch"], rot["Yaw"])
    if has_key("SceneAttachRelativeLocation",prop):
        scale = prop["SceneAttachRelativeScale3D"]
        ScaleUnreal = unreal.Vector(scale["X"], scale["Y"], scale["Z"])
    return unreal.Transform(LocationUnreal, RotationUnreal, ScaleUnreal)

def get_transform(Prop):
    TransformData = None
    bIsInstanced = False
    Props = Prop
    Quat = unreal.Quat()
    if has_key("TransformData", Props):
        TransformData = Props["TransformData"]
        bIsInstanced = True
    if has_key("RelativeLocation", Props) or has_key("OffsetLocation", Props) or has_key("Translation", TransformData) :
        if bIsInstanced:
            Location = TransformData["Translation"]
        else:
            Location = Props["RelativeLocation"]
        LocationUnreal = unreal.Vector(Location["X"], Location["Y"], Location["Z"])
    else:
        LocationUnreal = unreal.Vector(0.0, 0.0, 0.0)

    if has_key("RelativeScale3D", Props) or has_key("Scale3D", TransformData):
        if bIsInstanced:
            Scale = TransformData["Scale3D"]
            ScaleUnreal = unreal.Vector(Scale["X"], Scale["Y"], Scale["Z"])
        else:
            Scale = Props["RelativeScale3D"]
            ScaleUnreal = unreal.Vector(Scale["X"], Scale["Y"], Scale["Z"])
    else:
        ScaleUnreal = unreal.Vector(1.0, 1.0, 1.0)
    if has_key("RelativeRotation", Props) or has_key("Rotation", TransformData):
        if bIsInstanced:
            Rotation = TransformData["Rotation"]
            Quat = unreal.Quat(Rotation["X"], Rotation["Y"], Rotation["Z"], Rotation["W"])
            RotationUnreal = unreal.Rotator(0.0, 0.0, 0.0)
        else:
            Rotation = Props["RelativeRotation"]
            RotationUnreal = unreal.Rotator(Rotation["Roll"], Rotation["Pitch"], Rotation["Yaw"])
    else:
        RotationUnreal = unreal.Rotator(0.0, 0.0, 0.0)
    Trans = unreal.Transform(LocationUnreal, RotationUnreal, ScaleUnreal)
    if bIsInstanced:
        Trans.set_editor_property("rotation", Quat)
    return Trans

    
def has_key(key, array):
    if array == None:
        return False
    if key in array:
        return True
    else:
        return False




def GetClassName(self):
    return type(self).__name__





def return_formatted_string(string, prefix):
    start = string.rfind(prefix) + 1
    end = len(string)
    return string[start:end]


def has_transform(prop):
    bFactualBool = False
    if has_key("AttachParent", prop):
        bFactualBool = False
    if has_key("RelativeLocation", prop):
        bFactualBool = True
    if has_key("SceneAttachRelativeLocation", prop):
        bFactualBool = True
    if has_key("SceneAttachRelativeRotation", prop):
        bFactualBool = True
    if has_key("SceneAttachRelativeScale3D", prop):
        bFactualBool = True
    if has_key("RelativeRotation", prop):
        bFactualBool = True
    if has_key("RelativeScale3D", prop):
        bFactualBool = True
    if bFactualBool:
        return get_transform(prop)
    return bFactualBool



def return_python_unreal_enum(value):
    ind = 0
    value = re.sub(r'([a-z])([A-Z])', r'\1_\2', value)
    if value[0] == "_":
        ind = 1
    return value[ind:len(value)].upper()


def filter_objects(umap_DATA, current_umap_name) -> list:
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
    
    # Check for blacklisted items
    for og_model in filtered_list:
        objname = get_obj_name(data=og_model, mat=False)
        if not objname:
            continue
        model_name_lower = objname.lower()
        if is_blacklisted(model_name_lower):
            continue
        else:
            new_list.append(og_model)

    return new_list


def is_blacklisted(object_name: str) -> bool:
    for blocked in BLACKLIST:
        if blocked.lower() in object_name.lower():
            return True
    return False


def get_obj_name(data: dict, mat: bool):
    if mat:
        s = data["ObjectPath"]
    else:
        if has_key("Properties", data) == False:
            return "None"
        if "StaticMesh" in data["Properties"]:
            d = data["Properties"]["StaticMesh"]
            if not d:
                return None
            s = d["ObjectPath"]
        else:
            if not has_key("Outer", data):
                return None
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

def get_mat(decal_dict):
    mat_name = get_obj_name(data=decal_dict, mat=True)
    return unreal.load_asset(
        f'/Game/ValorantContent/Materials/{mat_name}.{mat_name}')

def get_scene_parent(obj, OuterName, umapfile):
    types = ["SceneComponent", "BrushComponent", "StaticMeshComponent"]
    if OuterName == 'PersistentLevel':
        OuterName = obj["Name"]
    for j in umapfile:
        tipo = j["Type"]
        if not has_key("Outer", j):
            continue
        outer = j["Outer"]
        if outer == "PersistentLevel":
            outer = j["Name"]
        # print(f'OuterName trying to find is {OuterName} and current outer is {outer} // also tipo is {tipo}')
        if has_key("Properties", j) == False:
            continue
        KeyOuter = has_key("AttachParent", j["Properties"])
        if outer == OuterName and tipo in types and KeyOuter == False:
            return has_transform(j["Properties"])


# exit()
def set_unreal_prop(self,prop_name,prop_value):
    try:
        self.set_editor_property(prop_name,prop_value)
    except:
        print(f'UianaPropLOG: Error setting {prop_name} to {prop_value}')



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
    def __init__(self, UESet):
        self.aes = UESet.vAesKey
        self.texture_format = ".png"
        ########## have to fix so it gets actual dir
        self.script_root = UESet.PPluginPath
        self.tools_path = self.script_root.joinpath("tools")
        self.importer_assets_path = self.script_root.joinpath("assets")
        self.paks_path = UESet.PPakFolder
        self.import_decals = UESet.bImportDecal
        self.import_blueprints = UESet.bImportBlueprint
        self.import_lights = UESet.bImportLights
        self.import_Mesh = UESet.bImportMesh
        self.import_materials = UESet.bImportMaterial
        self.import_sublevel = UESet.bImportSubLevels
        self.manual_lmres_mult = UESet.iManualLMResMult
        self.combine_umaps = False
        self.val_version = get_valorant_version()
        self.dev_force_reexport = False
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

## Map Definitions
class Map:
    def __init__(self, selected_map_name: str, maps_path: Path, all_umaps: list):
        self.name = selected_map_name
        # print(maps_path, self.name)
        self.folder_path = maps_path.joinpath(self.name)

        self.umaps = all_umaps[self.name]
        # print(self)
        self.actors_path = self.folder_path.joinpath("actors")
        self.materials_path = self.folder_path.joinpath("materials")
        self.materials_ovr_path = self.folder_path.joinpath("materials_ovr")
        self.objects_path = self.folder_path.joinpath("objects")
        self.scenes_path = self.folder_path.joinpath("scenes")
        self.umaps_path = self.folder_path.joinpath("umaps")
        create_folders(self)

# Actor definitions
class actor_defs():
    def __init__(self, Actor):
        self.data = Actor
        self.name = Actor["Name"] if has_key("Name", Actor) else None
        self.type = Actor["Type"] if has_key("Type", Actor) else None
        self.props = Actor["Properties"] if has_key("Properties", Actor) else {}
        ## add new attribute that gets every key from props that starts with "SceneAttach" and adds it to a dict if Actor has_key else none
        self.scene_props = {k: v for k, v in self.props.items() if k.startswith("SceneAttach")} if has_key("SceneAttach", self.props) else None
        self.outer = Actor["Outer"] if has_key("Outer", Actor) else None
        self.transform = has_transform(self.props)
        self.debug = f'ActorName: {self.name} // ActorType: {self.type} // ActorOuter: {self.outer} // ActorTransform: {self.transform} // ActorProps: {self.props.keys()}'

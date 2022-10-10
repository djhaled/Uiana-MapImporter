from .helpers import *


def filter_umap(umap_data: dict) -> list:
    umap_filtered = list()

    mesh_types = ["staticmesh", "staticmeshcomponent", "instancedstaticmeshcomponent",
                  "hierarchicalinstancedstaticmeshcomponent"]
    gen_types = ['pointlightcomponent', "postprocessvolume", "culldistancevolume", "scenecomponent",
                 "lightmasscharacterindirectdetailvolume", "brushcomponent", "precomputedvisibilityvolume",
                 'rectlightcomponent', 'spotlightcomponent', 'skylightcomponent', 'scenecapturecomponentcube',
                 "lightmassimportancevolume", "billboardcomponent", 'directionallightcomponent',
                 'exponentialheightfogcomponent', 'lightmassportalcomponent', 'spherereflectioncapturecomponent']
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

    return umap_filtered, object_types

VFX_WHITELIST = [
    "SM_VentSmoke_Duo",
    "M_Pitt_Lamps_Glow",
    "SM_Pitt_Dome_Glass",
    "SM_Pitt_Water_Surface",
    "LightShaft",
]
def get_objects(umap_data,current_umap):
    umap_objects = list()
    umap_materials = list()
    umap_actors = list()
    idx = 0
    for obj in umap_data:
        if obj["Type"].endswith("_C") and HasKeyzin("Template", obj):
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

                    if "OverrideMaterials" in obj["Properties"]:
                        for mat in obj["Properties"]["OverrideMaterials"]:
                            if mat is not None:
                                umap_materials.append(get_object_path(data=mat, mat=True))

            elif "DecalMaterial" in obj["Properties"]:
                mat = obj["Properties"]["DecalMaterial"]
                if mat is not None:
                    umap_materials.append(get_object_path(data=mat, mat=True))

    return umap_objects, umap_materials, umap_actors


def get_object_path(data: dict, mat: bool):
    if mat:
        s = data["ObjectPath"]
    else:
        s = data["Properties"]["StaticMesh"]["ObjectPath"]

    s = s.split(".", 1)[0].replace('/', '\\')
    return s


def get_object_type(model_data: dict) -> str:
    ModelType = model_data["Type"]
    lights = ["PointLightComponent", "PostProcessVolume", "PrecomputedVisibilityVolume", "CullDistanceVolume",
              "RectLightComponent", "LightmassCharacterIndirectDetailVolume", "SpotLightComponent", "SkyLightComponent",
              "LightmassImportanceVolume", "SceneCaptureComponentCube", "SphereReflectionCaptureComponent",
              "DirectionalLightComponent", "ExponentialHeightFogComponent", "LightmassPortalComponent"]
    meshes = ["StaticMeshComponent", "InstancedStaticMeshComponent", "HierarchicalInstancedStaticMeshComponent"]
    decals = ["DecalComponent"]
    blueprint = ["SceneComponent"]
    if model_data["Type"] in meshes:
        return "mesh"
    if model_data["Type"] in lights:
        return "light"
    if model_data["Type"] in decals:
        return "decal"
    if model_data["Type"].endswith("_C"):
        return "blueprint"


def get_object_materials(model_json: dict):
    # model_json = _common.read_json(model)
    model_materials = list()
    for objm in model_json:
        if objm["Type"] == "StaticMesh":
            StaticMat = objm["Properties"]["StaticMaterials"]
            for mat in StaticMat:
                if mat is not None and "MaterialInterface" in mat:
                    if mat["MaterialInterface"] is not None:
                        material = mat["MaterialInterface"]
                        model_materials.append(get_object_path(data=material, mat=True))

    return model_materials


def fix_path(a: str):
    b = a.replace("ShooterGame\\Content", "Game")
    c = b.replace("Engine\\Content", "Engine")
    return c




def get_name(s: str) -> str:
    return Path(s).stem


def HasKeyzin(key, array):
    if key in array:
        return True
    else:
        return False


# ANCHOR Shaders


# ANCHOR Getters



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





class MapObject(object):
    def __init__(self, settings: Settings, data: dict, umap_name: str, umap_data: list):
        self.umap = umap_name
        self.map_folder = settings.selected_map.folder_path
        self.objects_folder = settings.selected_map.objects_path
        self.data = data
        self.umapdata = umap_data
        self.name = self.get_object_name()
        self.uname = self.get_object_game_name()
        self.object_path = self.get_object_path()
        self.json = self.get_object_data_OG()
        self.model_path = self.get_local_model_path(p=settings.assets_path)

    def get_local_model_path(self, p: Path) -> str:
        if not HasKeyzin("StaticMesh", self.data["Properties"]):
            return "NoPath"
        a = p.joinpath(os.path.splitext(self.data["Properties"]["StaticMesh"]["ObjectPath"])[0].strip("/")).__str__()
        return fix_path(a) + ".pskx"

    def get_object_name(self) -> str:
        if not HasKeyzin("StaticMesh", self.data["Properties"]):
            return "NoMesh"
        s = self.data["Properties"]["StaticMesh"]["ObjectPath"]
        k = Path(s).stem
        return k

    def get_object_game_name(self) -> str:
        s = self.data["Outer"]
        return s

    def get_object_path(self, fix: bool = False) -> str:
        if not HasKeyzin("StaticMesh", self.data["Properties"]):
            return "NoPath"
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
        if self.name == "NoMesh":
            return "NoData"
        return read_json(self.objects_folder.joinpath(f"{self.name}.json"))

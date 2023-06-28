import os
import re
import subprocess
import time
from pathlib import Path
import unreal
import winsound
from mods.liana.helpers import *
from mods.liana.valorant import *

# BigSets 
all_textures = []
all_blueprints = {}
object_types = []
all_level_paths = []

AssetTools = unreal.AssetToolsHelpers.get_asset_tools()


## Returns a array with all OverrideMaterials
def create_override_material(data):
    material_array = []
    for mat in data["Properties"]["OverrideMaterials"]:
        if not mat:
            material_array.append(None)
            continue
        object_name = return_object_name(mat["ObjectName"])
        if object_name == "Stone_M2_Steps_MI1":
            object_name = "Stone_M2_Steps_MI"
        if "MaterialInstanceDynamic" in object_name:
            material_array.append(None)
            continue
        material_array.append(unreal.load_asset(
            f'/Game/ValorantContent/Materials/{object_name}'))
    return material_array


def extract_assets(settings: Settings):
    ## Extracting Assets on umodel
    asset_objects = settings.selected_map.folder_path.joinpath("all_assets.txt")
    args = [settings.umodel.__str__(),
            f"-path={settings.paks_path.__str__()}",
            f"-game=valorant",
            f"-aes={settings.aes}",
            f"-files={asset_objects}",
            "-export",
            f"-{settings.texture_format.replace('.', '')}",
            f"-out={settings.assets_path.__str__()}"]
    subprocess.call(args, stderr=subprocess.DEVNULL)


def extract_data(
        settings: Settings,
        export_directory: str,
        asset_list_txt: str = ""):
    ## Extracts the data from CUE4Parse (Json's)
    args = [settings.cue4extractor.__str__(),
            "--game-directory", settings.paks_path.__str__(),
            "--aes-key", settings.aes,
            "--export-directory", export_directory.__str__(),
            "--map-name", settings.selected_map.name,
            "--file-list", asset_list_txt,
            "--game-umaps", settings.umap_list_path.__str__()
            ]
    subprocess.call(args)


def get_map_assets(settings: Settings):
    ## Handles the extraction of the assets and filters them by type
    umaps = []
    if check_export(settings):
        extract_data(
            settings, export_directory=settings.selected_map.umaps_path)
        extract_assets(settings)

        umaps = get_files(
            path=settings.selected_map.umaps_path.__str__(), extension=".json")
        umap: Path

        object_list = list()
        actor_list = list()
        materials_ovr_list = list()
        materials_list = list()

        for umap in umaps:
            umap_json, asd = filter_umap(read_json(umap))
            object_types.append(asd)

            # save json
            save_json(umap.__str__(), umap_json)

            # get objects
            umap_objects, umap_materials, umap_actors = get_objects(umap_json, umap)
            actor_list.append(umap_actors)
            object_list.append(umap_objects)
            materials_ovr_list.append(umap_materials)
        # ACTORS
        actor_txt = save_list(filepath=settings.selected_map.folder_path.joinpath(
            f"_assets_actors.txt"), lines=actor_list)
        extract_data(
            settings,
            export_directory=settings.selected_map.actors_path,
            asset_list_txt=actor_txt)
        actors = get_files(
            path=settings.selected_map.actors_path.__str__(),
            extension=".json")

        for ac in actors:
            actor_json = read_json(ac)
            actor_objects, actor_materials, local4list = get_objects(actor_json, umap)
            object_list.append(actor_objects)
            materials_ovr_list.append(actor_materials)
        # next
        object_txt = save_list(filepath=settings.selected_map.folder_path.joinpath(
            f"_assets_objects.txt"), lines=object_list)
        mats_ovr_txt = save_list(filepath=settings.selected_map.folder_path.joinpath(
            f"_assets_materials_ovr.txt"), lines=materials_ovr_list)

        extract_data(
            settings,
            export_directory=settings.selected_map.objects_path,
            asset_list_txt=object_txt)
        extract_data(
            settings,
            export_directory=settings.selected_map.materials_ovr_path,
            asset_list_txt=mats_ovr_txt)

        # ---------------------------------------------------------------------------------------

        models = get_files(
            path=settings.selected_map.objects_path.__str__(),
            extension=".json")
        model: Path
        for model in models:
            model_json = read_json(model)
            # save json
            save_json(model.__str__(), model_json)
            # get object materials
            model_materials = get_object_materials(model_json)
            # get object textures
            # ...

            materials_list.append(model_materials)
        save_list(filepath=settings.selected_map.folder_path.joinpath("all_assets.txt"), lines=[
            [
                path_convert(path) for path in _list
            ] for _list in object_list + materials_list + materials_ovr_list
        ])
        mats_txt = save_list(filepath=settings.selected_map.folder_path.joinpath(
            f"_assets_materials.txt"), lines=materials_list)
        extract_data(
            settings,
            export_directory=settings.selected_map.materials_path,
            asset_list_txt=mats_txt)
        extract_assets(settings)
        with open(settings.selected_map.folder_path.joinpath('exported.yo').__str__(), 'w') as out_file:
            out_file.write(write_export_file())
        with open(settings.assets_path.joinpath('exported.yo').__str__(), 'w') as out_file:
            out_file.write(write_export_file())

    else:
        umaps = get_files(
            path=settings.selected_map.umaps_path.__str__(), extension=".json")

    return umaps


def set_material(
        ue_material,
        settings: Settings,
        mat_data: actor_defs, ):
    ## Sets the material parameters to the material
    if not mat_data.props:
        return
    mat_props = mat_data.props

    set_textures(mat_props, ue_material, settings=settings)
    set_all_settings(mat_props, ue_material)

    # fix this
    if "BasePropertyOverrides" in mat_props:
        base_prop_override = set_all_settings(mat_props["BasePropertyOverrides"],
                                              unreal.MaterialInstanceBasePropertyOverrides())
        set_unreal_prop(ue_material, "BasePropertyOverrides", base_prop_override)
        unreal.MaterialEditingLibrary.update_material_instance(ue_material)

    if "StaticParameters" in mat_props:
        if "StaticSwitchParameters" in mat_props["StaticParameters"]:
            for param in mat_props["StaticParameters"]["StaticSwitchParameters"]:
                param_name = param["ParameterInfo"]["Name"].lower()
                param_value = param["Value"]
                unreal.MaterialEditingLibrary.set_material_instance_static_switch_parameter_value(
                    ue_material, param_name, bool(param_value))
        ## Unreal doesn't support mask parameters switch lolz so have to do this
        if "StaticComponentMaskParameters" in mat_props["StaticParameters"]:
            for param in mat_props["StaticParameters"]["StaticComponentMaskParameters"]:
                mask_list = ["R", "G", "B"]
                for mask in mask_list:
                    value = param[mask]
                    unreal.MaterialEditingLibrary.set_material_instance_static_switch_parameter_value(
                        ue_material, mask, bool(value))
    if "ScalarParameterValues" in mat_props:
        for param in mat_props["ScalarParameterValues"]:
            param_name = param['ParameterInfo']['Name'].lower()
            param_value = param["ParameterValue"]
            set_material_scalar_value(ue_material, param_name, param_value)

    if "VectorParameterValues" in mat_props:
        for param in mat_props["VectorParameterValues"]:
            param_name = param['ParameterInfo']['Name'].lower()
            param_value = param["ParameterValue"]
            set_material_vector_value(ue_material, param_name, get_rgb(param_value))


def set_textures(mat_props: dict, material_reference, settings: Settings):
    ## Sets the textures to the material
    set_texture_param = unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value
    if not has_key("TextureParameterValues", mat_props):
        return
    for tex_param in mat_props["TextureParameterValues"]:
        tex_game_path = get_texture_path(s=tex_param, f=settings.texture_format)
        if not tex_game_path:
            continue
        tex_local_path = settings.assets_path.joinpath(tex_game_path).__str__()
        param_name = tex_param['ParameterInfo']['Name'].lower()
        tex_name = Path(tex_local_path).stem
        if Path(tex_local_path).exists():
            loaded_texture = unreal.load_asset(
                f'/Game/ValorantContent/Textures/{tex_name}')
            if not loaded_texture:
                continue
            set_texture_param(material_reference, param_name, loaded_texture)
    unreal.MaterialEditingLibrary.update_material_instance(material_reference)


def settings_create_ovr_material(mat_dict: list):
    ## No clue (?) but not gonna bother rn no idea why i didnt use the first ovr material func
    material_array = []
    loaded = None
    for mat in mat_dict:
        if not mat:
            material_array.append(None)
            continue
        object_name = return_object_name(mat["ObjectName"])
        if "MaterialInstanceDynamic" in object_name:
            material_array.append(None)
            continue
        loaded = unreal.load_asset(f'/Uiana/Materials/{object_name}')
        if loaded == None:
            loaded = unreal.load_asset(f'/Game/ValorantContent/Materials/{object_name}')
        material_array.append(loaded)
    return material_array


def set_all_settings(asset_props: dict, component_reference):
    ## Sets all the settings to the component (material, mesh, etc) from "Properties" using some weird black magic
    if not asset_props:
        return
    for setting in asset_props:
        value_setting = asset_props[setting]
        try:
            editor_property = component_reference.get_editor_property(setting)
        except:
            continue
        class_name = type(editor_property).__name__
        type_value = type(value_setting).__name__
        if type_value == "int" or type_value == "float" or type_value == "bool":
            if setting == "InfluenceRadius" and value_setting == 0:
                set_unreal_prop(component_reference, setting, 14680)
                continue
            set_unreal_prop(component_reference, setting, value_setting)
            continue
        if "::" in value_setting:
            value_setting = value_setting.split("::")[1]
            # Try to make these automatic? with 'unreal.classname.value"?
        if class_name == "LinearColor":
            set_unreal_prop(component_reference, setting, unreal.LinearColor(r=value_setting['R'], g=value_setting['G'],
                                                                             b=value_setting['B']))
            continue
        if class_name == "Vector4":
            set_unreal_prop(component_reference, setting, unreal.Vector4(x=value_setting['X'], y=value_setting['Y'],
                                                                         z=value_setting['Z'], w=value_setting['W']))
            continue
        if "Color" in class_name:
            set_unreal_prop(component_reference, setting, unreal.Color(r=value_setting['R'], g=value_setting['G'],
                                                                       b=value_setting['B'], a=value_setting['A']))
            continue
        if type_value == "dict":
            if setting == "IESTexture":
                set_unreal_prop(component_reference, setting, get_ies_texture(value_setting))
                continue
            if setting == "Cubemap":
                set_unreal_prop(component_reference, setting, get_cubemap_texture(value_setting))
                continue
            if setting == "DecalMaterial":
                component_reference.set_decal_material(get_mat(value_setting))
                continue
            if setting == "DecalSize":
                set_unreal_prop(component_reference, setting,
                                unreal.Vector(value_setting["X"], value_setting["Y"], value_setting["Z"]))
                continue
            if setting == "StaticMesh":
                ##mesh_loaded = unreal.BPFL.set_mesh_reference(value_setting["ObjectName"],"Meshes")
                mesh_loaded = mesh_to_asset(value_setting, "StaticMesh ", "Meshes")
                set_unreal_prop(component_reference, setting, mesh_loaded)
                continue
            if setting == "BoxExtent":
                set_unreal_prop(component_reference, 'box_extent',
                                unreal.Vector(x=value_setting['X'], y=value_setting['Y'], z=value_setting['Z'])),
                continue
            if setting == "LightmassSettings":
                set_unreal_prop(component_reference, 'lightmass_settings', get_light_mass(value_setting,
                                                                                          component_reference.get_editor_property(
                                                                                              'lightmass_settings')))
                continue
            continue
        if type_value == "list":
            if setting == "OverrideMaterials":
                mat_override = settings_create_ovr_material(value_setting)
                set_unreal_prop(component_reference, setting, mat_override)
                continue
            continue
        python_unreal_value = return_python_unreal_enum(value_setting)
        try:
            value = eval(f'unreal.{class_name}.{python_unreal_value}')
        except:
            continue
        set_unreal_prop(component_reference, setting, value)
    return component_reference


def get_light_mass(light_mass: dict, light_mass_reference):
    blacklist_lmass = ['bLightAsBackFace', 'bUseTwoSidedLighting']
    ## Sets the lightmass settings to the component
    for l_mass in light_mass:
        if l_mass in blacklist_lmass:
            continue
        value_setting = light_mass[l_mass]
        first_name = l_mass
        if l_mass[0] == "b":
            l_mass = l_mass[1:]
        value = re.sub(r'(?<!^)(?=[A-Z])', '_', l_mass)
        set_unreal_prop(light_mass_reference, value, value_setting)
    return light_mass_reference


def import_light(light_data: actor_defs, all_objs: list):
    ## Imports the light actors to the World
    light_type_replace = light_data.type.replace("Component", "")
    if not light_data.transform:
        light_data.transform = get_scene_transform(light_data.scene_props)
        # light_data.transform = get_scene_parent(light_data.data,light_data.outer,all_objs)
    if not light_data.transform:
        return
    light = unreal.EditorLevelLibrary.spawn_actor_from_class(eval(f'unreal.{light_type_replace}'),
                                                             light_data.transform.translation,
                                                             light_data.transform.rotation.rotator())
    light.set_folder_path(f'Lights/{light_type_replace}')
    light.set_actor_label(light_data.name)
    light.set_actor_scale3d(light_data.transform.scale3d)
    light_component = unreal.BPFL.get_component(light)
    if type(light_component) == unreal.BrushComponent:
        light_component = light
    if hasattr(light_component, "settings"):
        set_unreal_prop(light_component, "Unbound", True)
        set_unreal_prop(light_component, "Priority", 1.0)
        set_all_settings(light_data.props["Settings"], light_component.settings)
    set_all_settings(light_data.props, light_component)


def import_decal(decal_data: actor_defs):
    ## Imports the decal actors to the World
    if not decal_data.transform or has_key("Template", decal_data.data):
        return
    decal = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.DecalActor, decal_data.transform.translation,
                                                             decal_data.transform.rotation.rotator())
    decal.set_folder_path(f'Decals')
    decal.set_actor_label(decal_data.name)
    decal.set_actor_scale3d(decal_data.transform.scale3d)
    decal_component = decal.decal
    if type(decal_data) == dict:
        decal_component.set_decal_material(get_mat(actor_def=decal_data["DecalMaterial"]))
    set_all_settings(decal_data.props, decal_component)


## fix this so it stops returning #some bps are not spawning because of attachparent ig fix it later

def fix_actor_bp(actor_data: actor_defs, settings: Settings):
    ## Fixes spawned blueprints with runtime-changed components
    try:
        component = unreal.BPFL.get_component_by_name(all_blueprints[actor_data.outer], actor_data.name)
    except:
        return
    if not component:
        return
    if has_key("StaticMesh", actor_data.props):
        loaded = mesh_to_asset(actor_data.props["StaticMesh"], "StaticMesh ", "Meshes")
        #loaded = unreal.BPFL.set_mesh_reference(actor_data.props["StaticMesh"]["Name"],"Meshes")
        component.set_editor_property('static_mesh', loaded)
    if has_key("OverrideMaterials", actor_data.props):
        if settings.import_materials:
            mat_override = create_override_material(actor_data.data)
            if mat_override and "Barrier" not in actor_data.name:
                unreal.BPFL.set_override_material(all_blueprints[actor_data.outer], actor_data.name, mat_override)

    if not has_key("AttachParent", actor_data.props):
        return
    transform = has_transform(actor_data.props)
    if type(transform) != bool:
        if has_key("RelativeScale3D", actor_data.props):
            component = unreal.BPFL.get_component_by_name(all_blueprints[actor_data.outer], actor_data.name)
            set_unreal_prop(component, "relative_scale3d", transform.scale3d)
        if has_key("RelativeLocation", actor_data.props):
            component = unreal.BPFL.get_component_by_name(all_blueprints[actor_data.outer], actor_data.name)
            set_unreal_prop(component, "relative_location", transform.translation)
        if has_key("RelativeRotation", actor_data.props):
            component = unreal.BPFL.get_component_by_name(all_blueprints[actor_data.outer], actor_data.name)
            set_unreal_prop(component, "relative_rotation", transform.rotation.rotator())


def import_mesh(mesh_data: actor_defs, settings: Settings, map_obj: MapObject):
    ## Imports the mesh actor to the world
    override_vertex_colors = []
    if has_key("Template", mesh_data.data):
        fix_actor_bp(mesh_data, settings)
        return
    if not has_key("StaticMesh", mesh_data.props):
        return
    transform = get_transform(mesh_data.props)
    if not transform:
        return
    unreal_mesh_type = unreal.StaticMeshActor
    if map_obj.is_instanced():
        unreal_mesh_type = unreal.HismActor
    mesh_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal_mesh_type, location=unreal.Vector(),
                                                                  rotation=unreal.Rotator())
    mesh_actor.set_actor_label(mesh_data.outer)
    if has_key("LODData", mesh_data.data):
        override_vertex_colors = get_override_vertex_color(mesh_data.data)
    if map_obj.is_instanced():
        component = mesh_actor.hism_component
        mesh_actor.set_folder_path('Meshes/Instanced')
        for inst_index in mesh_data.data["PerInstanceSMData"]:
            component.add_instance(get_transform(inst_index))
    else:
        component = mesh_actor.static_mesh_component
        folder_name = 'Meshes/Static'
        if map_obj.umap.endswith('_VFX'):
            folder_name = 'Meshes/VFX'
        mesh_actor.set_folder_path(folder_name)
    set_all_settings(mesh_data.props, component)
    component.set_world_transform(transform, False, False)
    if len(override_vertex_colors) > 0:
        unreal.BPFL.paint_sm_vertices(component, override_vertex_colors, map_obj.model_path)
    if has_key("OverrideMaterials", mesh_data.props):
        if not settings.import_materials:
            return
        mat_override = create_override_material(mesh_data.data)
        if mat_override:
            set_unreal_prop(component, "override_materials", mat_override)


def set_mesh_build_settings(settings: Settings):
    ## Sets the build settings for the mesh since umodel exports it with wrong LMapCoordinateIndex and LMapResolution and Collision obviously
    light_res_multiplier = settings.manual_lmres_mult
    objects_path = settings.selected_map.objects_path
    list_objects = objects_path
    for m_object in os.listdir(list_objects):
        object_json = read_json(objects_path.joinpath(m_object))
        for o_object in object_json:
            key = actor_defs(o_object)
            if key.type == "StaticMesh":
                light_map_res = round(256 * light_res_multiplier / 4) * 4
                light_map_coord = 1
                if has_key("LightMapCoordinateIndex", key.props):
                    light_map_coord = key.props["LightMapCoordinateIndex"]
                if has_key("LightMapResolution", key.props):
                    light_map_res = round(key.props["LightMapResolution"] * light_res_multiplier / 4) * 4
                mesh_load = unreal.load_asset(f"/Game/ValorantContent/Meshes/{key.name}")
                if mesh_load:
                    cast_mesh = unreal.StaticMesh.cast(mesh_load)
                    actual_coord = cast_mesh.get_editor_property("light_map_coordinate_index")
                    actual_resolution = cast_mesh.get_editor_property("light_map_resolution")
                    if actual_coord != light_map_coord:
                        set_unreal_prop(cast_mesh, "light_map_coordinate_index", light_map_coord)
                    if actual_resolution != light_map_res:
                        set_unreal_prop(cast_mesh, "light_map_resolution", light_map_res)
            if key.type == "BodySetup":
                if has_key("CollisionTraceFlag", key.props):
                    col_trace = re.sub('([A-Z])', r'_\1', key.props["CollisionTraceFlag"])
                    mesh_load = unreal.load_asset(f"/Game/ValorantContent/Meshes/{key.outer}")
                    if mesh_load:
                        cast_mesh = unreal.StaticMesh.cast(mesh_load)
                        body_setup = cast_mesh.get_editor_property("body_setup")
                        str_collision = 'CTF_' + col_trace[8:len(col_trace)].upper()
                        set_unreal_prop(body_setup, "collision_trace_flag",
                                        eval(f'unreal.CollisionTraceFlag.{str_collision}'))
                        set_unreal_prop(cast_mesh, "body_setup", body_setup)


def import_umap(settings: Settings, umap_data: dict, umap_name: str):
    ## Imports a umap according to filters and settings selected // at the moment i don't like it since 
    ## it's a bit messy and i don't like the way i'm doing it but it works
    ## Looping before the main_import just for blueprints to be spawned first, no idea how to fix it lets stay like this atm
    objects_to_import = filter_objects(umap_data, umap_name)
    if settings.import_blueprints:
        for objectIndex, object_data in enumerate(objects_to_import):
            object_type = get_object_type(object_data)
            if object_type == "blueprint":
                import_blueprint(actor_defs(object_data), objects_to_import)
    for objectIndex, object_data in enumerate(objects_to_import):
        object_type = get_object_type(object_data)
        actor_data_definition = actor_defs(object_data)
        if object_type == "mesh" and settings.import_Mesh:
            map_object = MapObject(settings=settings, data=object_data, umap_name=umap_name, umap_data=umap_data)
            import_mesh(mesh_data=actor_data_definition, settings=settings, map_obj=map_object)
        if object_type == "decal" and settings.import_decals:
            import_decal(actor_data_definition)
        if object_type == "light" and settings.import_lights:
            import_light(actor_data_definition, objects_to_import)


def level_streaming_setup():
    ## Sets up the level streaming for the map if using the streaming option
    world = unreal.EditorLevelLibrary.get_editor_world()
    for level_path in all_level_paths:
        unreal.EditorLevelUtils.add_level_to_world(world, level_path, unreal.LevelStreamingAlwaysLoaded)


def import_blueprint(bp_actor: actor_defs, umap_data: list):
    ## Imports a blueprint actor from the umap
    transform = bp_actor.transform
    if not transform:
        transform = get_scene_transform(bp_actor.scene_props)
    if type(transform) == bool:
        transform = get_transform(bp_actor.props)
    if not transform:
        return
    bp_name = bp_actor.type[0:len(bp_actor.type) - 2]
    loaded_bp = unreal.load_asset(f"/Game/ValorantContent/Blueprints/{bp_name}.{bp_name}")
    actor = unreal.EditorLevelLibrary.spawn_actor_from_object(loaded_bp, transform.translation,
                                                              transform.rotation.rotator())
    if not actor:
        return
    all_blueprints[bp_actor.name] = actor
    actor.set_actor_label(bp_actor.name)
    actor.set_actor_scale3d(transform.scale3d)


def create_new_level(map_name):
    ## Creates a new level with the map name
    new_map = map_name.split('_')[0]
    map_path = (f"/Game/ValorantContent/Maps/{new_map}/{map_name}")
    loaded_map = unreal.load_asset(map_path)
    sub_system_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    unreal.LevelEditorSubsystem.new_level(sub_system_editor, map_path)
    all_level_paths.append(map_path)


def get_override_vertex_color(mesh_data: dict):
    ## Gets the override vertex color from the mesh data
    lod_data = mesh_data["LODData"]
    vtx_array = []
    for lod in lod_data:
        if has_key("OverrideVertexColors", lod):
            vertex_to_convert = lod["OverrideVertexColors"]["Data"]
            for rgba_hex in vertex_to_convert:
                vtx_array.append(unreal.BPFL.return_from_hex(rgba_hex))
    return vtx_array


def import_all_textures_from_material(material_data: dict, settings: Settings):
    ## Imports all the textures from a material json first
    mat_info = actor_defs(material_data[0])
    if mat_info.props:
        if has_key("TextureParameterValues", mat_info.props):
            tx_parameters = mat_info.props["TextureParameterValues"]
            for tx_param in tx_parameters:
                tex_game_path = get_texture_path(s=tx_param, f=settings.texture_format)
                if not tex_game_path:
                    continue
                tex_local_path = settings.assets_path.joinpath(tex_game_path).__str__()
                if tex_local_path not in all_textures:
                    all_textures.append(tex_local_path)


def create_material(material_data: list, settings: Settings):
    ## Creates a material from the material data 
    mat_data = material_data[0]
    mat_data = actor_defs(mat_data)
    parent = "BaseEnv_MAT_V4"
    if "Blend" in mat_data.name and "BaseEnv_Blend_MAT_V4_V3Compatibility" not in mat_data.name:
        parent = "BaseEnv_Blend_MAT_V4"
    if not mat_data.props:
        return
    loaded_material = unreal.load_asset(f"/Game/ValorantContent/Materials/{mat_data.name}.{mat_data.name}")
    if not loaded_material:
        loaded_material = AssetTools.create_asset(mat_data.name, '/Game/ValorantContent/Materials/',
                                                  unreal.MaterialInstanceConstant,
                                                  unreal.MaterialInstanceConstantFactoryNew())
    if has_key("Parent", mat_data.props):
        parent = return_parent(mat_data.props["Parent"]["ObjectName"])
    material_instance = unreal.MaterialInstanceConstant.cast(loaded_material)
    set_unreal_prop(material_instance, "parent", import_shader(parent))
    set_material(settings=settings, mat_data=mat_data, ue_material=loaded_material)


def import_all_meshes(settings: Settings):
    ## Imports all the meshes from the map first accordingly.
    all_meshes = []
    obj_path = settings.selected_map.folder_path.joinpath("_assets_objects.txt")
    with open(obj_path, 'r') as file:
        lines = file.read().splitlines()
    exp_path = str(settings.assets_path)
    for line in lines:
        if is_blacklisted(line.split('\\')[-1]):
            continue
        line_arr = line.split('\\')
        if line_arr[0] == "Engine":
            continue
        else:
            line_arr.pop(0)
            line_arr.pop(0)
        joined_lines_back = "\\".join(line_arr)
        full_path = exp_path + '\\Game\\' + joined_lines_back + ".pskx"
        if full_path not in all_meshes:
            all_meshes.append(full_path)
    # import
    unreal.BPFL.import_meshes(all_meshes, str(settings.selected_map.objects_path))


def imports_all_textures(settings: Settings):
    ## Imports all the texture from materials.
    mat_path = settings.selected_map.materials_path
    mat_ovr_path = settings.selected_map.materials_ovr_path

    for path_mat in os.listdir(mat_path):
        mat_json = read_json(mat_path.joinpath(path_mat))
        import_all_textures_from_material(mat_json, settings)
    for path_ovr_mat in os.listdir(mat_ovr_path):
        mat_ovr_json = read_json(mat_ovr_path.joinpath(path_ovr_mat))
        import_all_textures_from_material(mat_ovr_json, settings)
    unreal.BPFL.import_textures(all_textures)


def create_bp(full_data: dict, bp_name: str, settings: Settings):
    ## Creates a blueprint from the json data
    BlacklistBP = ["SoundBarrier", "SpawnBarrierProjectile", "BP_UnwalkableBlockingVolumeCylinder",
                   'BP_StuckPickupVolume', "BP_BlockingVolume", "TargetingBlockingVolume_Box", "directional_look_up"]
    # BlacklistBP = ['SpawnBarrier','SoundBarrier','SpawnBarrierProjectile','Gumshoe_CameraBlockingVolumeParent_Box','DomeBuyMarker','BP_StuckPickupVolume','BP_LevelBlockingVolume','BP_TargetingLandmark','BombSpawnLocation']
    bp_name = bp_name.split('.')[0]
    bp_actor = unreal.load_asset(f'/Game/ValorantContent/Blueprints/{bp_name}')
    if not bp_actor and bp_name not in BlacklistBP:
        bp_actor = AssetTools.create_asset(bp_name, '/Game/ValorantContent/Blueprints/', unreal.Blueprint,
                                           unreal.BlueprintFactory())
    else:
        return
    data = full_data["Nodes"]
    if len(data) == 0:
        return
    root_scene = full_data["SceneRoot"]
    default_scene_root = root_scene[0].split('.')[-1]
    game_objects = full_data["GameObjects"]
    nodes_root = full_data["ChildNodes"]
    for idx, bpc in enumerate(data):
        if bpc["Name"] == default_scene_root:
            del data[idx]
            data.insert(len(data), bpc)
            break
    data.reverse()
    nodes_array = []
    for bp_node in data:
        if bp_node["Name"] in nodes_root:
            continue
        component_name = bp_node["Properties"]["ComponentClass"]["ObjectName"].replace(
            "Class ", "")
        try:
            unreal_class = eval(f'unreal.{component_name}')
        except:
            continue
        properties = bp_node["Properties"]
        if has_key("ChildNodes", properties):
            nodes_array = handle_child_nodes(properties["ChildNodes"], data, bp_actor)
        comp_internal_name = properties["InternalVariableName"]
        component = unreal.BPFL.create_bp_comp(
            bp_actor, unreal_class, comp_internal_name, nodes_array)
        if has_key("CompProps", properties):
            properties = properties["CompProps"]
        set_mesh_settings(properties, component)
        set_all_settings(properties, component)
    for game_object in game_objects:
        if bp_name == "SpawnBarrier":
            continue
        component = unreal.BPFL.create_bp_comp(bp_actor, unreal.StaticMeshComponent, "GameObjectMesh", nodes_array)
        set_all_settings(game_object["Properties"], component)
        set_mesh_settings(game_object["Properties"], component)


def set_mesh_settings(mesh_properties: dict, component):
    set_all_settings(mesh_properties, component)
    transform = get_transform(mesh_properties)
    if has_key("RelativeRotation", mesh_properties):
        set_unreal_prop(component, "relative_rotation", transform.rotation.rotator())
    if has_key("RelativeLocation", mesh_properties):
        set_unreal_prop(component, "relative_location", transform.translation)
    if has_key("RelativeScale3D", mesh_properties):
        set_unreal_prop(component, "relative_scale3d", transform.scale3d)


def handle_child_nodes(child_nodes_array: dict, entire_data: list, bp_actor):
    ## Handles the child nodes of the blueprint since they are not in order.
    local_child_array = []
    for child_node in child_nodes_array:
        child_obj_name = child_node["ObjectName"]
        child_name = child_obj_name.split('.')[-1]
        for c_node in entire_data:
            component_name = c_node["Properties"]["ComponentClass"]["ObjectName"].replace(
                "Class ", "")
            try:
                unreal_class = eval(f'unreal.{component_name}')
            except:
                continue
            internal_name = c_node["Properties"]["InternalVariableName"]
            if "TargetViewMode" in internal_name or "Decal1" in internal_name or "SM_Barrier_Back_VisionBlocker" in internal_name:
                continue

            if c_node["Name"] == child_name:
                u_node, comp_node = unreal.BPFL.create_node(
                    bp_actor, unreal_class, internal_name)
                local_child_array.append(u_node)
                set_all_settings(c_node["Properties"]["CompProps"], comp_node)
                transform = has_transform(c_node["Properties"]["CompProps"])
                if type(transform) != bool:
                    comp_node.set_editor_property("relative_location", transform.translation)
                    comp_node.set_editor_property("relative_rotation", transform.rotation.rotator())
                    comp_node.set_editor_property("relative_scale3d", transform.scale3d)
                break
    return local_child_array


def import_all_blueprints(settings: Settings):
    ## Imports all the blueprints from the actors folder.
    bp_path = settings.selected_map.actors_path
    for bp in os.listdir(bp_path):
        bp_json = reduce_bp_json(read_json(settings.selected_map.actors_path.joinpath(bp)))
        create_bp(bp_json, bp, settings)


def import_all_materials(settings: Settings):
    ## Imports all the materials from the materials folder.
    mat_path = settings.selected_map.materials_path
    mat_ovr_path = settings.selected_map.materials_ovr_path
    for path_mat in os.listdir(mat_path):
        mat_json = read_json(mat_path.joinpath(path_mat))
        create_material(mat_json, settings)
    for path_ovr_mat in os.listdir(mat_ovr_path):
        mat_ovr_json = read_json(mat_ovr_path.joinpath(path_ovr_mat))
        create_material(mat_ovr_json, settings)


def import_map(setting):
    ## Main function first it sets some lighting settings ( have to revisit it for people who don't want the script to change their lighting settings)
    ## then it imports all meshes / textures / blueprints first and create materials from them.
    ## then each umap from the /maps folder is imported and the actors spawned accordingly.
    unreal.BPFL.change_project_settings()
    unreal.BPFL.execute_console_command('r.DefaultFeature.LightUnits 0')
    unreal.BPFL.execute_console_command('r.DynamicGlobalIlluminationMethod 0')
    all_level_paths.clear()
    settings = Settings(setting)
    umap_json_paths = get_map_assets(settings)
    if not settings.import_sublevel:
        create_new_level(settings.selected_map.name)
    clear_level()
    if settings.import_materials:
        txt_time = time.time()
        imports_all_textures(settings)
        print(f'Exported all textures in {time.time() - txt_time} seconds')
        mat_time = time.time()
        import_all_materials(settings)
        print(f'Exported all materials in {time.time() - mat_time} seconds')
    m_start_time = time.time()
    if settings.import_Mesh:
        import_all_meshes(settings)
    print(f'Exported all meshes in {time.time() - m_start_time} seconds')
    bp_start_time = time.time()
    if settings.import_blueprints:
        import_all_blueprints(settings)
    print(f'Exported all blueprints in {time.time() - bp_start_time} seconds')
    umap_json_path: Path
    actor_start_time = time.time()
    with unreal.ScopedSlowTask(len(umap_json_paths), "Importing levels") as slow_task:
        slow_task.make_dialog(True)
        idx = 0
        for index, umap_json_path in reversed(
                list(enumerate(umap_json_paths))):
            umap_data = read_json(umap_json_path)
            umap_name = umap_json_path.stem
            slow_task.enter_progress_frame(
                work=1, desc=f"Importing level:{umap_name}  {idx}/{len(umap_json_paths)} ")
            if settings.import_sublevel:
                create_new_level(umap_name)
            import_umap(settings=settings, umap_data=umap_data, umap_name=umap_name)
            if settings.import_sublevel:
                unreal.EditorLevelLibrary.save_current_level()
            idx = idx + 1
        print("--- %s seconds to spawn actors ---" % (time.time() - actor_start_time))
        if settings.import_sublevel:
            level_streaming_setup()
        if settings.import_Mesh:
            set_mesh_build_settings(settings=settings)
        # winsound.Beep(7500, 983)

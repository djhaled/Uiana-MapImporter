
import os
from os.path import exists
import subprocess
import unreal
import time
from mods.liana.helpers import *
from mods.liana.valorant import *

start_time = time.time()
AssetRegistry = unreal.AssetRegistryHelpers.get_asset_registry()
Seting = None
RepeatedMats = []
AllTasks = []
AllTextures= []
object_types = []
AllLoadableMaterials = {}
AllMeshes = []  
AllLevelPaths = []

testbek = 0

def IterateArrayMats(arr):
	ActualName = ReturnFormattedString(arr,"/")
	for j in AllLoadableMaterials:
		if j.find(ActualName) != -1:
			return j
	return None
def GetMaterialToOverride(Data):
	Props = Data["Properties"]
	MaterialArray = []
	OverrideMaterials = Props["OverrideMaterials"]
	for j in OverrideMaterials:
		Loadable = ConvertToLoadableUE(j,"MaterialInstanceConstant ")
		if Loadable == None:
			MaterialArray.append(None)
			continue
		Result = IterateArrayMats(Loadable)
		if Result == None:
			continue
		ToLoad = AllLoadableMaterials[f"{Result}"]
		Shi = ConvertToLoadableMaterial(ToLoad,"MaterialInstanceConstant ")
		Material = unreal.load_asset(Shi)
		MaterialArray.append(Material)
	return MaterialArray
def extract_assets(settings: Settings):
	if settings.assets_path.joinpath("exported.yo").exists():
		pass
	else:
		args = [settings.umodel.__str__(),
				f"-path={settings.paks_path.__str__()}",
				f"-game=valorant",
				f"-aes={settings.aes}",
				"*.uasset",
				"-export",
				"-noanim",
				"-nooverwrite",
				f"-{settings.texture_format.replace('.', '')}",
				f"-out={settings.assets_path.__str__()}"]
		subprocess.call(args,stderr=subprocess.DEVNULL)



def extract_data(settings: Settings, export_directory: str, asset_list_txt: str = ""):
	args = [settings.cue4extractor.__str__(),
			"--game-directory", settings.paks_path.__str__(),
			"--aes-key", settings.aes,
			"--export-directory", export_directory.__str__(),
			"--map-name", settings.selected_map.name,
			"--file-list", asset_list_txt,
			"--game-umaps", settings.umap_list_path.__str__()
			]
	subprocess.call(args)


def do_import_tasks(Meshes,tasks,bTexture):
	unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
	if Meshes is not None:
		for t in Meshes:
			if Seting.import_materials == True:
				set_materials(Settings,t,False)
			else:
				pass
	AllTasks.clear()
	AllMeshes.clear()
	AllTextures.clear()


def get_object(map_object, index):
	name = map_object.get_object_name()
	path_to_file = unreal.load_asset(f"/Game/Meshes/All/{name}")
	if path_to_file != None:
		return
	task = unreal.AssetImportTask()
	task.set_editor_property('destination_path', '/Game/Meshes/All')
	task.set_editor_property('filename', map_object.model_path)
	task.set_editor_property('automated', True)
	task.set_editor_property('save', True)
	task.set_editor_property('replace_existing', False)
	AllMeshes.append(map_object)
	AllTasks.append(task)


def get_map_assets(settings: Settings):
	umaps = []

	if not settings.selected_map.folder_path.joinpath("exported.yo").exists():
		extract_data(settings, export_directory=settings.selected_map.umaps_path)
		extract_assets(settings)

		umaps = get_files(
			path=settings.selected_map.umaps_path.__str__(), extension=".json")
		umap: Path

		object_list = list()
		materials_ovr_list = list()
		materials_list = list()

		for umap in umaps:
			umap_json, asd = filter_umap(read_json(umap))
			object_types.append(asd)

			# save json
			save_json(umap.__str__(), umap_json)

			# get objects
			umap_objects, umap_materials = get_objects(umap_json)

			object_list.append(umap_objects)
			materials_ovr_list.append(umap_materials)

		object_txt = save_list(filepath=settings.selected_map.folder_path.joinpath(
			f"_assets_objects.txt"), lines=object_list)
		mats_ovr_txt = save_list(filepath=settings.selected_map.folder_path.joinpath(
			f"_assets_materials_ovr.txt"), lines=materials_ovr_list)

		extract_data(settings, export_directory=settings.selected_map.objects_path,
					 asset_list_txt=object_txt)
		extract_data(settings, export_directory=settings.selected_map.materials_ovr_path,
					 asset_list_txt=mats_ovr_txt)

		# ---------------------------------------------------------------------------------------

		models = get_files(
			path=settings.selected_map.objects_path.__str__(), extension=".json")
		model: Path
		for model in models:

			model_json = read_json(model)[2]
			model_name = model.stem

			# save json
			save_json(model.__str__(), model_json)

			# get object materials
			model_materials = get_object_materials(model_json)

			# get object textures
			# ...

			materials_list.append(model_materials)

		mats_txt = save_list(filepath=settings.selected_map.folder_path.joinpath(
			f"_assets_materials.txt"), lines=materials_list)
		extract_data(settings, export_directory=settings.selected_map.materials_path,
					 asset_list_txt=mats_txt)

		with open(settings.selected_map.folder_path.joinpath('exported.yo').__str__(), 'w') as out_file:
			out_file.write("")
		with open(settings.assets_path.joinpath('exported.yo').__str__(), 'w') as out_file:
			out_file.write("")

	else:
		umaps = get_files(
			path=settings.selected_map.umaps_path.__str__(), extension=".json")

	return umaps
# TODO : MATERIALS
	### WAITING ON HALF TO FIX .PSKS
def SetDecalMaterial(Set,MapObject):
	Set = Seting
	object_properties_OG = MapObject["Properties"]
	ObjectName = MapObject["Name"]
	if "DecalMaterial" in MapObject["Properties"]:
		yoyo = MapObject["Properties"]["DecalMaterial"]
		mat_name = get_obj_name(data=yoyo, mat=True)
		mat_json = read_json(Set.selected_map.materials_ovr_path.joinpath(f"{mat_name}.json"))
		Mat = unreal.load_asset(f'/Game/Meshes/All/{mat_name}.{mat_name}')
		if Mat is None:
			Mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset(mat_name,'/Game/Meshes/All/', unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
			unreal.EditorAssetLibrary.save_asset(f'/Game/Meshes/All/{mat_name}')
		Mat = unreal.MaterialInstanceConstant.cast(Mat)
		MatBase = importDecalShaders()
		Mat.set_editor_property('parent', MatBase)
		set_material(settings=Settings,  mat_data=mat_json[0], object_cls=MapObject,UEMat = Mat,decal=True )
		return Mat
def set_materials(Set,MapObject,decal):
	Set = Seting
	object_properties_OG = MapObject.json["Properties"]
	ObjectName = MapObject.json["Name"]
	object_properties = MapObject.data["Properties"]
		
	if "StaticMaterials" in object_properties_OG:
		for index, mat in enumerate(object_properties_OG["StaticMaterials"]):
			if type(mat["MaterialInterface"]) is dict:
				mat_name = get_obj_name(data=mat["MaterialInterface"], mat=True)
				if "WorldGridMaterial" not in mat_name and mat_name not in RepeatedMats:
					mat_json = read_json(Set.selected_map.materials_path.joinpath(f"{mat_name}.json"))
					mat_data = mat_json[0]
					Mat = unreal.load_asset(f'/Game/Meshes/All/{mat_name}.{mat_name}')
					if Mat is None:
						continue
					RepeatedMats.append(mat_name)
					Mat = unreal.MaterialInstanceConstant.cast(Mat)
					MatBase = import_shaders()
					Parent = Mat.set_editor_property('parent', MatBase)
					set_material(settings=Settings,  mat_data=mat_json[0], object_cls=MapObject,UEMat = Mat )

	if "OverrideMaterials" in object_properties:
		for index, mat in enumerate(object_properties["OverrideMaterials"]):
			if type(mat) is dict:
				mat_name = get_obj_name(data=mat, mat=True)
				if mat_name  in RepeatedMats:
					continue
				mat_json = read_json(Set.selected_map.materials_ovr_path.joinpath(f"{mat_name}.json"))
				Mat = unreal.load_asset(f'/Game/Meshes/All/{mat_name}.{mat_name}')
				if Mat is None and mat_name not in RepeatedMats:
					RepeatedMats.append(mat_name)
					Asset = unreal.load_asset(f'/Game/Meshes/All/{mat_name}.{mat_name}')
					if (Asset):
						pass
					else:
						Mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset(mat_name,'/Game/Meshes/All/', unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
						unreal.EditorAssetLibrary.save_asset(f'/Game/Meshes/All/{mat_name}')
				Mat = unreal.MaterialInstanceConstant.cast(Mat)
				MatBase = import_shaders()
				Test = Mat.set_editor_property('parent', MatBase)
				set_material(settings=Settings,  mat_data=mat_json[0], object_cls=MapObject,UEMat = Mat )


# SECTION : Set Material

def set_material(settings: Settings, UEMat,  mat_data: dict, override: bool = False, decal: bool = False, object_cls: MapObject = None):
	if "Properties" not in mat_data:
		return
	mat_props = mat_data["Properties"]
	if "Parent" in mat_props:
		mat_type = get_name(mat_props["Parent"]["ObjectPath"])
	else:
		mat_type = "NO PARENT"
	mat_props = mat_data["Properties"]
	mat_name = mat_data["Name"]
	SetTextures(mat_props,UEMat)
	SetAllSettings(mat_props,UEMat)
	BasePropsBlacklist = ['bVertexFog','LightingSourceDirectionality','bOverride_IndirectLightingContributionValue','IndirectLightingContributionValue','TranslucencyDepthMode','ShadingModel','bOverride_VertexFog','bOverride_CubemapSource','CubemapSource','bOverride_SortPriorityOffset','SortPriorityOffset','bOverride_Fresnel','bFresnel','bOverride_SpecularModel','SpecularModel','bSpecularModel','bOverride_CubemapMode','CubemapMode']
	if "BasePropertyOverrides" in mat_props:
		for prop_name, prop_value in mat_props["BasePropertyOverrides"].items():
			if "BlendMode" == prop_name:
				if "BLEND_Translucent" in prop_value:
					blend_mode = unreal.BlendMode.BLEND_TRANSLUCENT
				elif "BLEND_Masked" in prop_value:
					blend_mode = unreal.BlendMode.BLEND_MASKED
				elif "BLEND_Additive" in prop_value:
					blend_mode = unreal.BlendMode.BLEND_ADDITIVE
				elif "BLEND_Modulate" in prop_value:
					blend_mode = unreal.BlendMode.BLEND_MODULATE
				elif "BLEND_AlphaComposite" in prop_value:
					blend_mode = unreal.BlendMode.BLEND_ALPHA_COMPOSITE
				elif "BLEND_AlphaHoldout" in prop_value:
					blend_mode = unreal.BlendMode.BLEND_OPAQUE
				BasePropOverride = unreal.MaterialInstanceBasePropertyOverrides()
				BasePropOverride.set_editor_property('bOverride_BlendMode', True)
				BasePropOverride.set_editor_property('blend_mode', blend_mode)
				UEMat.set_editor_property('BasePropertyOverrides',BasePropOverride)
				unreal.MaterialEditingLibrary.update_material_instance(UEMat)
				continue
			if prop_name not in BasePropsBlacklist:
				BasePropOverride = unreal.MaterialInstanceBasePropertyOverrides()
				BasePropOverride.set_editor_property(prop_name, prop_value)
	if "StaticParameters" in mat_props:
		if "StaticSwitchParameters" in mat_props["StaticParameters"]:
			for param in mat_props["StaticParameters"]["StaticSwitchParameters"]:
				param_name = param["ParameterInfo"]["Name"].lower()
				param_value = param["Value"]
				unreal.MaterialEditingLibrary.set_material_instance_static_switch_parameter_value(UEMat, param_name,bool(param_value))
		if "StaticComponentMaskParameters" in mat_props["StaticParameters"]:
			pass
			# for param in mat_props["StaticParameters"]["StaticComponentMaskParameters"]:
			# 	param_name = param["ParameterInfo"]["Name"].lower()
			# 	param_value = param["ParameterInfo"]["bOverride"]
			# 	if param_name == "mask":
			# 		# MASK = "R"
			# 		colors = {"R", "G", "B", "A"}
			# 		for color in colors:
			# 			if color in param:
			# 				if param[color]:
			# 					pass
								#if f"Use {color}" in N_SHADER.inputs:
									#N_SHADER.inputs[f"Use {color}"].default_value = 1
	if "ScalarParameterValues" in mat_props:
		for param in mat_props["ScalarParameterValues"]:
			param_name = param['ParameterInfo']['Name'].lower()
			param_value = param["ParameterValue"]
			SetMaterialScalarValue(UEMat,param_name,param_value)
	if "VectorParameterValues" in mat_props:
		for param in mat_props["VectorParameterValues"]:
			param_name = param['ParameterInfo']['Name'].lower()
			param_value = param["ParameterValue"]
			SetMaterialVectorValue(UEMat,param_name,get_rgb(param_value))


def get_scalar_value(mat_props, s_param_name):
	if "ScalarParameterValues" in mat_props:
		for param in mat_props["ScalarParameterValues"]:
			param_name = param['ParameterInfo']['Name'].lower()
			if s_param_name.lower() in param_name:
				return param["ParameterValue"]

# SECTION Get Textures
# NOTE: Might be tuned bit more

COUNT = 0

def ImportTexture(Path):
	task = unreal.AssetImportTask()
	task.set_editor_property('destination_path', '/Game/Meshes/Textures')
	task.set_editor_property('filename', Path)
	task.set_editor_property('automated', True)
	task.set_editor_property('save', True)
	task.set_editor_property('replace_existing', False)
	AllTextures.append(task)



def SetTextures(mat_props: dict, MatRef):
	Set = Seting
	ImportedTexture = None
	blacklist_tex = [
		"Albedo_DF",
		"MRA_MRA",
		"Normal_NM",
		"Diffuse B Low",
		"Blank_M0_NM",
		"Blank_M0_Flat_00_black_white_NM",
		"flatnormal",
		"Diffuse B Low",
		"flatwhite",
	]
	if (HasKey("TextureParameterValues",mat_props) == False):
		return
	for param in mat_props["TextureParameterValues"]:
		tex_game_path = get_texture_path(s=param, f=Set.texture_format)
		tex_local_path = Set.assets_path.joinpath(tex_game_path).__str__()
		param_name = param['ParameterInfo']['Name'].lower()
		tex_name = Path(tex_local_path).stem
		if "diffuse b low" not in param_name:
			if Path(tex_local_path).exists() and tex_name not in blacklist_tex:
				ImportedTexture = unreal.load_asset(f'/Game/Meshes/Textures/{tex_name}.{tex_name}')
			if ImportedTexture == None:
				continue
			if "diffuse" == param_name or "albedo" == param_name :
				MatParameterValue = unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(MatRef, 'Diffuse', ImportedTexture)
			if "diffuse a" == param_name  or "texture a" == param_name or "rgba" == param_name:
				MatParameterValue = unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(MatRef, 'Diffuse A', ImportedTexture)
			if "diffuse b" == param_name:
				MatParameterValue = unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(MatRef, 'Diffuse B', ImportedTexture)
			if "mra" == param_name:
				MatParameterValue = unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(MatRef, 'MRA', ImportedTexture)
			if  "mra a" == param_name:
				MatParameterValue = unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(MatRef, 'MRA A', ImportedTexture)
			if "mra b" == param_name :
				MatParameterValue = unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(MatRef, 'MRA B', ImportedTexture)
			if "normal" == param_name:
				MatParameterValue = unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(MatRef, 'Normal', ImportedTexture)
				unreal.MaterialEditingLibrary.set_material_instance_static_switch_parameter_value(MatRef, 'HasJustNormal',True)
			if  "texture a normal" == param_name or "normal a" == param_name:
				MatParameterValue = unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(MatRef, 'Texture A Normal', ImportedTexture)
			if "normal b" == param_name or "texture b normal" == param_name:
				MatParameterValue = unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(MatRef, 'Texture B Normal', ImportedTexture)
				pass
			if "mask" in param_name or "rgba" in param_name:
				pass

	unreal.MaterialEditingLibrary.update_material_instance(MatRef)


def SpawnMiscObject(data, umap):
	for obj in data:
		object_type = get_object_type(obj)
		if object_type != "misc":
			continue
		TransformActor = None
		IsSceneComp = False
		SceneComps = ["TextRenderComponent","TargetPoint","CameraComponent","CineCameraComponent","LevelSequenceActor","SceneCaptureComponent2D","SphereReflectionCaptureComponent","BoxComponent","TriggerVolume","KillZVolume"]
		MiscActor = obj
		OuterName = MiscActor["Outer"]
		if OuterName == "PersistentLevel":
			OuterName = MiscActor["Name"]
	# if MiscActor["Type"] != "BP_PlayerKillVolume_C":
	# 	return
		if HasKey("Properties",MiscActor):
			ActorProps = MiscActor["Properties"]
		else:
			continue
		Actor = unreal.ValActor
		HasTForm = HasTransform(ActorProps)
		if HasTForm == True:
			TransformActor = GetTransform(MiscActor,False)
		else:
			bak = GetAttachScene(MiscActor,OuterName,umap)
			if bak == None:
				continue
			TransformActor = GetTransform(bak,False)
		if TransformActor == None:
			continue
		ActorType = MiscActor["Type"]
		if ActorType in SceneComps:
			IsSceneComp = True
	#######SetPositionInWorld###########
			 ########SetLightningTypes#######
		ActorTypeNew = ActorType.lower().replace("component","")
		if IsSceneComp == True:
			TypeSceneComp = ActorType.replace("Component","Actor")
			if ActorType == "SceneCaptureComponent2D":
				TypeSceneComp = "SceneCapture2D"
			if ActorType == "SphereReflectionCaptureComponent":
				TypeSceneComp = "SphereReflectionCapture"
			if ActorType == "BoxComponent":
				TypeSceneComp = "ValActor"
			Actor = eval(f'unreal.{TypeSceneComp}')
			ActorTypeEval = Actor 
			########SpawnLightAndGetReferenceForComp#######
		SpawnActor = unreal.EditorLevelLibrary.spawn_actor_from_class(Actor, TransformActor.translation, TransformActor.rotation.rotator())
		SpawnActor.set_actor_scale3d(TransformActor.scale3d)
		ActorTypeEval = SpawnActor
		if IsSceneComp == False:
			if ActorType == "BP_BlockingVolume_C":
				ActorTypeNew = "blocking_volume"
			ActorTypeEval = eval(f'SpawnActor.create_{ActorTypeNew}_component()')
		else:
			if TypeSceneComp == "TextRenderActor":
				ActorTypeEval = SpawnActor.text_render
			if TypeSceneComp == "CameraActor":
				ActorTypeEval = SpawnActor.camera_component
			if TypeSceneComp == "CineCameraActor":
				ActorTypeEval = SpawnActor.camera_component 
			if ActorType == "SceneCaptureComponent2D":
				ActorTypeEval = SpawnActor.capture_component2d
			if ActorType == "SphereReflectionCaptureComponent":
				ActorTypeEval = SpawnActor.capture_component


		SpawnActor.set_actor_label(OuterName)
		if ActorType  == "TriggerVolume":
			continue
		if ActorType == "BP_BlockingVolume_C":
			PropRight = GetBlockingMesh(obj,OuterName,umap)
			PropRightProps = PropRight["Properties"]
			PathToGo = ConvertToLoadableUE(PropRightProps["StaticMesh"],"StaticMesh ")
			MeshToLoad = unreal.load_asset(PathToGo)
			ActorTypeEval.set_static_mesh(MeshToLoad)
			ActorTypeEval = SpawnActor
		Comp = ActorTypeEval
		SetAllSettings(ActorProps,Comp)
def SetSMSettings():
	OBJPath = Seting.selected_map.objects_path
	### first normal mats #######
	ListObjs = os.listdir(OBJPath)
	for j in ListObjs:
		Join = OBJPath.joinpath(j)
		ObjJson = read_json(Join)
		sm = ObjJson
		if sm["Type"] == "StaticMesh":
			Props = sm["Properties"]
			Name = sm["Name"]
			LmCoord = 0
			LMRes = 256
			if HasKey("LightMapResolution",Props):
				LMRes = Props["LightMapResolution"]
			if HasKey("LightMapCoordinateIndex",Props):
				LmCoord = Props["LightMapCoordinateIndex"]
			MeshToLoad = unreal.load_asset(f"/Game/Meshes/All/{Name}")
			if (MeshToLoad):
				CastSM = unreal.StaticMesh.cast(MeshToLoad)
				CastSM.set_editor_property("light_map_coordinate_index", LmCoord)
				CastSM.set_editor_property("light_map_resolution", LMRes)





def SetAllSettings(asset,Comp):
	blackmisc = ["currentfocusdistance","CachedMaxDrawDistance","OnComponentBeginOverlap"]
	for Setting in asset:
		bHasIt = HasSetting(Setting,Comp,blackmisc)
		if bHasIt == True:
			ActorSetting = asset[Setting]
			if Setting in blackmisc:
				continue
			if Setting == "OrthoWidth":
				Setting = "ortho_width"
			PropSet = Comp.get_editor_property(Setting)
			classname = GetClassName(PropSet)
			if type(ActorSetting) == int or type(ActorSetting) == float or type(ActorSetting) == bool :
				Comp.set_editor_property(Setting, ActorSetting)
				continue
			if type(ActorSetting) == str:
				ActorSetting = ActorSetting.upper()
			if "::" in ActorSetting:
				ActorSetting = ReturnFormattedString(ActorSetting,":")
			if classname == "Color":
				Colorized = unreal.Color(r=ActorSetting['R'], g=ActorSetting['G'], b=ActorSetting['B'], a=ActorSetting['A'])
				Comp.set_editor_property(Setting, Colorized)
				continue
			if type(ActorSetting) == dict:
				if Setting == "LightmassSettings":
					### gotta fix later
					if type(Comp) == unreal.MaterialInstanceConstant:
						ReturnLMass = SetMaterialLightmassSetting(ActorSetting)
						Comp.set_editor_property("lightmass_settings",ReturnLMass)
						continue
					ReturnLMass = SetLightMassSettings(ActorSetting)
					Comp.set_editor_property("lightmass_settings",ReturnLMass)
				continue
			ActualValue = FindNonSlasher(eval(f'unreal.{classname}'),ActorSetting)
			if ActualValue == None:
				continue
			value = eval(f'unreal.{classname}.{ActualValue}')
			Comp.set_editor_property(Setting, value)
def SetMaterialLightmassSetting(ActorSetting):
	Set = unreal.LightmassMaterialInterfaceSettings()
	for val in ActorSetting:
		if val == "DiffuseBoost":
			Set.set_editor_property("diffuse_boost",ActorSetting[val])
		if val == "bCastShadowAsMasked":
			Set.set_editor_property("cast_shadow_as_masked",ActorSetting[val])
		if val == "ExportResolutionScale":
			Set.set_editor_property("export_resolution_scale",ActorSetting[val])
		return Set

def SetLightMassSettings(ActorSetting):
	Set = unreal.LightmassPrimitiveSettings()
	for val in ActorSetting:
		if val == "DiffuseBoost":
			Set.set_editor_property("diffuse_boost",ActorSetting[val])
		if val == "EmissiveBoost":
			Set.set_editor_property("emissive_boost",ActorSetting[val])
		if val == "FullyOccludedSamplesFraction":
			Set.set_editor_property("fully_occluded_samples_fraction",ActorSetting[val])
		if val == "bShadowIndirectOnly":
			Set.set_editor_property("shadow_indirect_only",ActorSetting[val])
		if val == "bUseEmissiveForStaticLighting":
			Set.set_editor_property("use_emissive_for_static_lighting",ActorSetting[val])
		if val == "bUseTwoSidedLighting":
			Set.set_editor_property("use_two_sided_lighting",ActorSetting[val])
		if val == "bUseVertexNormalForHemisphereGather":
			Set.set_editor_property("use_vertex_normal_for_hemisphere_gather",ActorSetting[val])
		return Set

def ImportDecal(DecalData):
	Transform = GetTransform(object_data,False)
	DecActor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.DecalActor,Transform[0],Transform[1])
	DecActor.set_actor_scale3d(Transform[2])
	DecalComponent = DecActor.decal
	DecalMat = SetDecalMaterial(Settings, object_data)
	DecalComponent.set_decal_material(DecalMat)
	BlacklistDecal =  ['DecalMaterial','LightColorType','RelativeScale3D','RelativeRotation','RelativeLocation','bVertexFog','bOverrideColor','bOverrideIntensity','DetailMode','VisibilityId','bAllowCullingWhenFacingCamera','LightColorOverride','LightIntensityOverride','LightingSourceDirectionality','bOverride_IndirectLightingContributionValue','IndirectLightingContributionValue','TranslucencyDepthMode','ShadingModel','bOverride_VertexFog','bOverride_CubemapSource','CubemapSource','bOverride_SortPriorityOffset','SortPriorityOffset','bOverride_Fresnel','bFresnel','bOverride_SpecularModel','SpecularModel','bSpecularModel','bOverride_CubemapMode','CubemapMode']
	for propName,PropValue in object_data["Properties"].items():
		if propName not in BlacklistDecal:
			if propName == "DecalSize":
				PropValue = unreal.Vector(PropValue["X"],PropValue["Y"],PropValue["Z"])
			DecalComponent.set_editor_property(propName,PropValue)
def ImportLights(OBJData, ArrObjsImport):
	ActualData = OBJData
	LightningType = ActualData["Type"]
	LightningProps = ActualData["Properties"]
	LightningOuter = ActualData["Outer"]
	LightningActualName = ActualData["Name"]
	if HasTransform(LightningProps) == False:
		attachscene = GetAttachScene(ActualData,LightningOuter,ArrObjsImport)
		ActualData = attachscene
	if LightningType == "SphereReflectionCaptureComponent":
		if HasKey("Cubemap",LightningProps) == False:
			return
	TransformLights = GetTransform(ActualData,False)
	PostProcessSettings = []
	LightTypeNoComp = LightningType.replace("Component","")
	LightType = eval(f'unreal.{LightTypeNoComp}')
			########SpawnLightAndGetReferenceForComp#######
	LightActor = unreal.EditorLevelLibrary.spawn_actor_from_class(LightType, TransformLights.translation, TransformLights.rotation.rotator())
	LightActor.set_actor_label(LightningActualName)
	LightActor.set_actor_scale3d(TransformLights.scale3d )
	if hasattr(LightActor,"light_component"):
		CompToUse = LightActor.light_component
	elif  hasattr(LightActor,"portal_component"):
		CompToUse = LightActor.portal_component
	elif hasattr(LightActor,"scene_component"):
		CompToUse = LightActor.scene_component
	elif hasattr(LightActor,"settings"):
		LightActor.set_editor_property("Unbound", True)
		LightActor.set_editor_property("Priority",1.0)
		CompToUse = LightActor.settings
	elif hasattr(LightActor,"capture_component"):
		CompToUse = LightActor.capture_component
	elif hasattr(LightActor,"component") == False:
		CompToUse = LightActor
	else:
		CompToUse = LightActor.component 
	#Figure out Mobility
	Mobility = unreal.ComponentMobility.STATIC
	LightActor.modify()
	if HasKey("Settings",LightningProps) == True:
		PostProcessSettings = LightningProps["Settings"]
		SetPostProcessSettings(PostProcessSettings,CompToUse)
	for Setting in LightningProps:
		LightSettingType = type(LightningProps[Setting])
		LightSetting = LightningProps[Setting]
				#if Setting == "ReflectionSourceType":
					#continue
		if Setting == "IESTexture":
			CompToUse.set_editor_property('IESTexture',SetIesTexture(LightSetting))
		if Setting == "Cubemap":
			CompToUse.set_editor_property('Cubemap',SetCubeMapTexture(LightSetting))
		if Setting == "LightColor":
			Colorized = unreal.Color(r=LightSetting['R'], g=LightSetting['G'], b=LightSetting['B'], a=LightSetting['A'])
			CompToUse.set_editor_property('LightColor',Colorized)
		if Setting == "FogInscatteringColor":
			Colorized = unreal.LinearColor(r=LightSetting['R'], g=LightSetting['G'], b=LightSetting['B'], a=LightSetting['A'])
			CompToUse.set_editor_property('fog_inscattering_luminance',Colorized)
		if Setting == "Mobility":
			CompToUse.set_editor_property('Mobility',Mobility)
	SetAllSettings(LightningProps,CompToUse)


def import_umap(settings: Settings, umap_data: dict, umap_name: str):
	map_object = None
	test = []
	objectsToImport = filter_objects(umap_data)
	if COUNT != 0:
		objectsToImport = objectsToImport[:COUNT]
	for objectIndex, object_data in enumerate(objectsToImport):
		objectIndex = f"{objectIndex:03}"
		object_type = get_object_type(object_data)
		if object_type == "mesh" and Seting.import_Mesh == True:
			#if "Lighting" not in umap_name:
			map_object = MapObject(settings=settings, data=object_data, umap_name=umap_name)
			imported_object = import_object(map_object=map_object, object_index=objectIndex)
		if object_type == "decal" and settings.import_decals:
			ImportDecal(object_data)
		if object_type == "light" and settings.import_lights:
			ImportLights(object_data,objectsToImport)
	do_import_tasks(AllMeshes,AllTasks,False)
	if Seting.import_Mesh == True:
		SpawnMeshesInMap(umap_data,settings,umap_name)
	if Seting.import_Misc == True:
		SpawnMiscObject(umap_data,umap_data)
def LevelStreamingStuff():
	world = unreal.EditorLevelLibrary.get_editor_world()
	for j in AllLevelPaths:
		JAfterSlash = ReturnFormattedString(j,"/")
		MapType = GetUMapType(JAfterSlash)
		unreal.EditorLevelUtils.add_level_to_world(world, j, MapType)
		ReadableMapType = GetReadableUMapType(JAfterSlash)
		if ReadableMapType == "LevelStreamingDynamic":
			SubSys = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
			Level2 = unreal.LevelEditorSubsystem.get_current_level(SubSys)
			unreal.EditorLevelUtils.set_level_visibility(Level2,False,False)
# ANCHOR: Functions
def SetPostProcessSettings(AllSettings,Comp):
	for Setting in AllSettings:
		bekaBlackList=["bOverride_AmbientOcclusionTintColor","AutoExposureBiasBackup","AmbientOcclusionTintColor","SavedSelections","bOverride_AresAdaptiveSharpenEnable",'FilmContrast','FilmWhitePoint',"bOverride_AresClarityEnable","bOverride_IndirectLightingScaleCurve","bOverride_AutoExposureBiasBackup","IndirectLightingColor","IndirectLightingScaleCurve","bOverride_ScreenPercentage"]
		if Setting not in bekaBlackList:
			Comp.set_editor_property(Setting, AllSettings[Setting])

def CreateNewLevel(mapname):
	newmap = GetInitialName(mapname)
	startpath = f"/Game/Maps/{newmap}/{mapname}"
	SubSystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
	unreal.LevelEditorSubsystem.new_level(SubSystem,startpath)
	AllLevelPaths.append(startpath)
def SpawnMeshesInMap(data,set,mapname):
	AllAssets = AssetRegistry.get_assets_by_path('/Game/Meshes/All/')
	for j in AllAssets:
		Asset = j.get_asset()
		NameFull = Asset.get_full_name()
		if "MaterialInstanceConstant " in NameFull:
			Name = NameFull.replace("MaterialInstanceConstant ", "")
			ActualName = ReturnFormattedString(Name,".")
			AllLoadableMaterials[f"{ActualName}"] = NameFull
	for j in data:
		object_type = get_object_type(j)
		if object_type == "mesh":
			HasVCol = False
			map_object = MapObject(settings=set, data=j, umap_name=mapname)
			ActualData = map_object.data
			LocalData = ActualData
			NameProp = ActualData["Outer"]
			if HasKey("Template",ActualData):
				bIsBlocking = IsBlockingVolume(j,NameProp,data)
				if bIsBlocking == True:
					continue
			ObjectProps = ActualData["Properties"]
			if HasKey("StaticMesh",ObjectProps):
				PathToGo = ConvertToLoadableUE(ObjectProps["StaticMesh"],"StaticMesh ")
			if HasTransform(ObjectProps) == False:
				LocalData = GetAttachScene(j,NameProp,data)
			Transform = GetTransform(LocalData,False)
			if Transform == None:
				continue
			SMActor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.ValActor,Transform.translation,Transform.rotation.rotator())
			SMActor.set_actor_scale3d(Transform.scale3d)
			MeshToLoad = unreal.load_asset(PathToGo)
			OvrVertexes = []
			if HasKey("LODData",ActualData):
				Lod = ActualData["LODData"]
				for itLod in Lod:
					if HasKey("OverrideVertexColors",itLod):
						DataToConvert = itLod["OverrideVertexColors"]["Data"]
						for rgba_hex in DataToConvert:
							testbk = []
							Color = unreal.BPFL.return_from_hex(rgba_hex)
							OvrVertexes.append(Color)
							HasVCol = True
			if map_object.is_instanced():
				instance_data = ActualData["PerInstanceSMData"]
				Instance = SMActor.create_instance_component(OvrVertexes,MeshToLoad)
				SMActor.set_actor_label(NameProp)
				for j in instance_data:
					Trans = GetTransform(j,True)
					if Trans == None:
						continue
					Transform = unreal.Transform(location=Trans.translation, rotation=Trans.rotation.rotator(), scale=Trans.scale3d)
					Instance.add_instance(Transform)
				if(HasVCol == True):
					unreal.BPFL.paint_sm_vertices(Instance,OvrVertexes)
			else:
				Instance = SMActor.create_static_component(OvrVertexes,MeshToLoad)
				if(HasVCol == True):
					unreal.BPFL.paint_sm_vertices(Instance,OvrVertexes)
				SMActor.set_actor_label(NameProp)
			SetAllSettings(ObjectProps,Instance)
			if HasKey("OverrideMaterials",ObjectProps):
				MatOver = GetMaterialToOverride(ActualData)
				if MatOver != None:
					Instance.set_editor_property('override_materials',MatOver) 

		#exit()




				#break


def import_object(map_object: MapObject,  object_index: int):

	master_object = None

	if Path(map_object.model_path).exists():
		master_object = get_object(map_object, object_index)

		


# ANCHOR Post Processing

def ImportAllTexturesFromMaterial(matJson):
	for node in matJson:
		if HasKey("Properties",node):
			props = node["Properties"]
			if HasKey("TextureParameterValues",props):
				TextParamValues = props["TextureParameterValues"]
				for param in TextParamValues:
					tex_game_path = get_texture_path(s=param, f=Seting.texture_format)
					tex_local_path = Seting.assets_path.joinpath(tex_game_path).__str__()
					param_name = param['ParameterInfo']['Name'].lower()
					tex_name = Path(tex_local_path).stem
					ImportTexture(tex_local_path)


	

def ExportAllTexture():
	MatPath = Seting.selected_map.materials_path
	MatOverridePath = Seting.selected_map.materials_ovr_path

	### first normal mats #######
	listMatPath = os.listdir(MatPath)
	for pathzin in listMatPath:
		bago = MatPath.joinpath(pathzin)
		MatJson = read_json(bago)
		ImportAllTexturesFromMaterial(MatJson)
	   ########## mat ovverrides##############
	listOverridePath = os.listdir(MatOverridePath)
	for pathovr in listOverridePath:
		entireovrpath = MatOverridePath.joinpath(pathovr)
		MatJson = read_json(entireovrpath)
		ImportAllTexturesFromMaterial(MatJson)




	do_import_tasks(None,AllTextures,True)

def import_map(Setting):
	AllLevelPaths.clear()
	settings = Settings(Setting)
	global Seting
	Seting = settings
	umap_json_paths = get_map_assets(Seting)
	ClearLevel()
	#  Check if the game files are exported
	######### export all textures before ###########
	if (Seting.import_materials == True):
		ExportAllTexture()
	###### above takes 0.09 might fix #######
	umap_json_path: Path
	for index, umap_json_path in reversed(list(enumerate(umap_json_paths))):
		umap_data = read_json(umap_json_path)
		umap_name = umap_json_path.stem
		CreateNewLevel(umap_name)
		import_umap(settings=settings, umap_data=umap_data, umap_name=umap_name)
		unreal.EditorLevelLibrary.save_current_level()
	LevelStreamingStuff()
	SetSMSettings()
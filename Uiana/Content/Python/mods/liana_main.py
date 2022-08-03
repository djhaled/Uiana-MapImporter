
import os
from os.path import exists
import re
import winsound
import subprocess
from pathlib import Path
import unreal
import time
from mods.liana.helpers import *
from mods.liana.valorant import *

Seting = None
AllTasks = []
AllMeshes = [] 
AllTextures= []
object_types = []
AllLevelPaths = []
file = "snd.mp3"


BaseEnv = ["BaseEnv_Blend_MAT_V4","BaseEnv_Blend_MAT_V4_V3Compatibility","BaseEnv_MAT_V4","BaseEnv_MAT_V4_Inst","BaseEnv_MAT","BlendEnv_MAT","BaseEnvEmissiveUnlit_MAT"]
def GetMaterialToOverride(Data):
	Props = Data["Properties"]
	MaterialArray = []
	OverrideMaterials = Props["OverrideMaterials"]
	for mat in OverrideMaterials:
		Loadable = ConvertToLoadableUE(mat,"MaterialInstanceConstant ","Materials")
		if Loadable == None:
			MaterialArray.append(None)
			continue
		Material = unreal.load_asset(Loadable)
		MaterialArray.append(Material)
	return MaterialArray

def bIsDefaultEnv(asset):
	if asset in BaseEnv:
		return "BaseEnv_MAT_V4"
	else:
		return None

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
	path_to_file = unreal.load_asset(f"/Game/ValorantContent/Meshes/{name}")
	if path_to_file != None:
		return
	task = unreal.AssetImportTask()
	task.set_editor_property('destination_path', '/Game/ValorantContent/Meshes')
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
	Parent = "DecalBugged"
	if "DecalMaterial" in MapObject["Properties"]:
		yoyo = MapObject["Properties"]["DecalMaterial"]
		mat_name = get_obj_name(data=yoyo, mat=True)
		mat_json = read_json(Set.selected_map.materials_ovr_path.joinpath(f"{mat_name}.json"))
		MaterialData = mat_json[0]
		if HasKey("Properties",MaterialData) == False:
			return
		Props = MaterialData["Properties"]
		if HasKey("Parent",Props) == False:
			idk = 0 
			#print(MaterialData["Name"])
		else:
			Parent = Props["Parent"]["ObjectName"]
		Mat = unreal.load_asset(f'/Game/ValorantContent/Materials/{mat_name}.{mat_name}')
		if Mat is None:
			Mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset(mat_name,'/Game/ValorantContent/Materials/', unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
			unreal.EditorAssetLibrary.save_asset(f'/Game/ValorantContent/Materials/{mat_name}')
		Mat = unreal.MaterialInstanceConstant.cast(Mat)
		MatBase = ImportShader(Parent.replace("Material ", ""))
		Mat.set_editor_property('parent', MatBase)
		set_material(settings=Settings,  mat_data=MaterialData, object_cls=MapObject,UEMat = Mat,decal=True )
		return Mat
def set_materials(Set,MapObject,decal):
	ParentToImport = None
	Set = Seting
	object_properties_OG = MapObject.json["Properties"]
	ObjectName = MapObject.json["Name"]
	object_properties = MapObject.data["Properties"]
	Parenteses = "MatBugged"
	if "StaticMaterials" in object_properties_OG:
		for index, mat in enumerate(object_properties_OG["StaticMaterials"]):
			if type(mat["MaterialInterface"]) is dict:
				mat_name = get_obj_name(data=mat["MaterialInterface"], mat=True)
				if "WorldGridMaterial" not in mat_name:
					mat_json = read_json(Set.selected_map.materials_path.joinpath(f"{mat_name}.json"))
					mat_data = mat_json[0]
					MatProps = mat_data["Properties"]
					Mat = unreal.load_asset(f'/Game/ValorantContent/Materials/{mat_name}.{mat_name}')
					if Mat is None:
						continue
					if HasKey("Parent",MatProps):
						ParentToImport = ReturnParent(MatProps["Parent"]["ObjectName"])
					Mat = unreal.MaterialInstanceConstant.cast(Mat)
					MatBase = ImportShader(ParentToImport)
					if MatBase == None:
						print(ParentToImport)
					#MatBase = import_shaders()
					Parent = Mat.set_editor_property('parent', MatBase)
					set_material(settings=Settings,  mat_data=mat_json[0], object_cls=MapObject,UEMat = Mat )

	if "OverrideMaterials" in object_properties:
		Parenteses = "MatBugged"
		for index, mat in enumerate(object_properties["OverrideMaterials"]):
			if type(mat) is dict:
				mat_name = get_obj_name(data=mat, mat=True)
				mat_json = read_json(Set.selected_map.materials_ovr_path.joinpath(f"{mat_name}.json"))
				mat_data = mat_json[0]
				if HasKey("Properties",mat_data):
					MatProps = mat_data["Properties"]
				else:
					print(f'This fucking MATERIAL DOESNT HAVE PROPERTIES {mat_data["Name"]}')
					return
				Mat = unreal.load_asset(f'/Game/ValorantContent/Materials/{mat_name}.{mat_name}')
				if Mat == None:
					Mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(mat_name,'/Game/ValorantContent/Materials/', unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
					unreal.EditorAssetLibrary.save_asset(f'/Game/ValorantContent/Materials/{mat_name}')
				Mat = unreal.MaterialInstanceConstant.cast(Mat)
				if HasKey("Parent",MatProps):
					ParentToImport = ReturnParent(MatProps["Parent"]["ObjectName"])
				MatBase = ImportShader(ParentToImport)
				if MatBase == None:
					print(ParentToImport)
				Test = Mat.set_editor_property('parent', MatBase)
				set_material(settings=Settings,  mat_data=mat_json[0], object_cls=MapObject,UEMat = Mat )


# SECTION : Set Material
def ReturnParent(parentName):
	rformPar =parentName.rfind(' ') + 1
	ActualName = parentName[rformPar:len(parentName)]
	DefEnv = bIsDefaultEnv(ActualName)
	if DefEnv != None:
		ParentName = "BaseEnv_MAT_V4"
	else:
		ParentName = ActualName
	return ParentName


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
	BasePropsBlacklist = ['bVertexFog','bOverride_DecalDiffuseLighting','bDecalDiffuseLighting','bDitherOpacityMask','DecalDiffuseLighting','LightingSourceDirectionality','bOverride_IndirectLightingContributionValue','IndirectLightingContributionValue','TranslucencyDepthMode','ShadingModel','bOverride_VertexFog','bOverride_CubemapSource','CubemapSource','bOverride_SortPriorityOffset','SortPriorityOffset','bOverride_Fresnel','bFresnel','bOverride_SpecularModel','SpecularModel','bSpecularModel','bOverride_CubemapMode','CubemapMode']
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
					blend_mode = unreal.BlendMode.BLEND_BLEND_ALPHA_HOLDOUT
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
			for param in mat_props["StaticParameters"]["StaticComponentMaskParameters"]:
				listosa = ["R","G","B"]
				for pa in listosa:
					value = param[pa]
					unreal.MaterialEditingLibrary.set_material_instance_static_switch_parameter_value(UEMat, pa,bool(value))
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
	task.set_editor_property('destination_path', '/Game/ValorantContent/Textures')
	task.set_editor_property('filename', Path)
	task.set_editor_property('automated', True)
	task.set_editor_property('save', True)
	task.set_editor_property('replace_existing', False)
	AllTextures.append(task)



def SetTextures(mat_props: dict, MatRef):
	Set = Seting
	ImportedTexture = None
	if (HasKey("TextureParameterValues",mat_props) == False):
		return
	for param in mat_props["TextureParameterValues"]:
		tex_game_path = get_texture_path(s=param, f=Set.texture_format)
		tex_local_path = Set.assets_path.joinpath(tex_game_path).__str__()
		param_name = param['ParameterInfo']['Name'].lower()
		tex_name = Path(tex_local_path).stem
		if "diffuse b low" not in param_name:
			if Path(tex_local_path).exists():
				ImportedTexture = unreal.load_asset(f'/Game/ValorantContent/Textures/{tex_name}.{tex_name}')
			if ImportedTexture == None:
				continue
			if "rgba" == param_name:
				MatParameterValue = unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(MatRef, 'RGBA', ImportedTexture)
			if "diffuse" == param_name or "albedo" == param_name :
				MatParameterValue = unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(MatRef, 'Diffuse', ImportedTexture)
			if "diffuse a" == param_name  or "texture a" == param_name :
				MatParameterValue = unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(MatRef, 'Diffuse A', ImportedTexture)
			if "diffuse b" == param_name:
				MatParameterValue = unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(MatRef, 'Diffuse B', ImportedTexture)
				unreal.MaterialEditingLibrary.set_material_instance_static_switch_parameter_value(MatRef, 'Use B',True)
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
			if "mask" in param_name or "Mask Textuer" in param_name or "Mask Texture" in param_name:
				MatParameterValue = unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(MatRef, 'Mask Textuer', ImportedTexture)
			if "mask" in param_name or "rgba" in param_name:
				pass

	unreal.MaterialEditingLibrary.update_material_instance(MatRef)

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
			MeshToLoad = unreal.load_asset(f"/Game/ValorantContent/Meshes/{Name}")
			if (MeshToLoad):
				CastSM = unreal.StaticMesh.cast(MeshToLoad)
				CastSM.set_editor_property("light_map_coordinate_index", LmCoord)
				CastSM.set_editor_property("light_map_resolution", LMRes)





def SetAllSettings(asset,Comp):
	blackmisc = ["CachedMaxDrawDistance","OnComponentBeginOverlap","Mobility"]
	for Setting in asset:
		bHasIt = HasSetting(Setting,Comp,blackmisc)
		if bHasIt == True:
			ActorSetting = asset[Setting]
			if Setting in blackmisc:
				continue
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
					ReturnLMass = None
					### gotta fix later
					if type(Comp) == unreal.MaterialInstanceConstant:
						ReturnLMass = SetLightmassSetting(ActorSetting,"LightmassMaterialInterfaceSettings")
						continue
					if type(Comp) == unreal.DirectionalLightComponent or type(Comp) == unreal.SpotLightComponent:
						ReturnLMass = SetLightmassSetting(ActorSetting,"LightmassDirectionalLightSettings")
						continue
					if type(Comp) == unreal.PointLightComponent:
						ReturnLMass = SetLightmassSetting(ActorSetting,"LightmassPointLightSettings")
						continue
					if type(Comp) == unreal.StaticMeshComponent or type(Comp) == unreal.HierarchicalInstancedStaticMeshComponent:
						ReturnLMass = SetLightmassSetting(ActorSetting,"LightmassPrimitiveSettings")
					if ReturnLMass == None:
						continue
					Comp.set_editor_property("lightmass_settings",ReturnLMass)
					continue
				continue
			ActualValue = FindNonSlasher(eval(f'unreal.{classname}'),ActorSetting)
			if ActualValue == None:
				continue
			value = eval(f'unreal.{classname}.{ActualValue}')
			Comp.set_editor_property(Setting, value)
def SetLightmassSetting(ActorSetting,Evalu):
	Set = eval(f'unreal.{Evalu}()')
	for val in ActorSetting:
		if val == "bCastShadowAsTranslucent" or val == "bOverrideCastShadowAsTranslucent":
			continue
		num = 1
		if val.startswith("b"):
			num = 2
		newstr = re.sub('([A-Z])', r'_\1', val)
		Set.set_editor_property(newstr[num:len(newstr)].lower(),ActorSetting[val])
	return Set

def ImportDecal(DecalData):
	ActorInfo = ActorDefs(DecalData)
	if ActorInfo.transform == False:
		return
	DecActor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.DecalActor,ActorInfo.transform.translation,ActorInfo.transform.rotation.rotator())
	DecActor.set_folder_path(f'Decals')
	DecActor.set_actor_scale3d(ActorInfo.transform.scale3d)
	DecalComponent = DecActor.decal
	DecalMat = SetDecalMaterial(Settings, DecalData)
	DecalComponent.set_decal_material(DecalMat)
	BlacklistDecal =  ['DecalMaterial','LightColorType','CachedVertexFogIntensityFromVolumes','bVertexFog','bOverrideColor','bOverrideIntensity','DetailMode','VisibilityId','bAllowCullingWhenFacingCamera','LightColorOverride','LightIntensityOverride','LightingSourceDirectionality','bOverride_IndirectLightingContributionValue','IndirectLightingContributionValue','TranslucencyDepthMode','ShadingModel','bOverride_VertexFog','bOverride_CubemapSource','CubemapSource','bOverride_SortPriorityOffset','SortPriorityOffset','bOverride_Fresnel','bFresnel','bOverride_SpecularModel','SpecularModel','bSpecularModel','bOverride_CubemapMode','CubemapMode']
	for propName,PropValue in ActorInfo.props.items():
		if propName not in BlacklistDecal:
			SetAllSettings(DecalData,DecalComponent)
def ImportLights(OBJData, ArrObjsImport):
	ActorInfo = ActorDefs(OBJData)
	if ActorInfo.type == "SphereReflectionCaptureComponent":
		if HasKey("Cubemap",ActorInfo.props) == False:
			return
	PostProcessSettings = []
	LightTypeNoComp = ActorInfo.type.replace("Component","")
	LightType = eval(f'unreal.{LightTypeNoComp}')
			########SpawnLightAndGetReferenceForComp#######
	if ActorInfo.transform == False:
		ActorInfo.transform = GetAttachScene(OBJData,ActorInfo.outer,ArrObjsImport)
	LightActor = unreal.EditorLevelLibrary.spawn_actor_from_class(LightType, ActorInfo.transform.translation, ActorInfo.transform.rotation.rotator())
	LightActor.set_folder_path(f'Lights/{LightTypeNoComp}')
	LightActor.set_actor_label(ActorInfo.name)
	LightActor.set_actor_scale3d(ActorInfo.transform.scale3d )
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
	if HasKey("Settings",ActorInfo.props) == True:
		PostProcessSettings = ActorInfo.props["Settings"]
		SetPostProcessSettings(PostProcessSettings,CompToUse)
	for Setting in ActorInfo.props:
		LightSettingType = type(ActorInfo.props[Setting])
		LightSetting = ActorInfo.props[Setting]
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
	SetAllSettings(ActorInfo.props,CompToUse)


def import_umap(settings: Settings, umap_data: dict, umap_name: str):
	map_object = None
	test = []
	objectsToImport = filter_objects(umap_data)
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
	startpath = f"/Game/ValorantContent/Maps/{newmap}/{mapname}"
	bLoaded =unreal.load_asset(startpath)
	SubSystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
	unreal.LevelEditorSubsystem.new_level(SubSystem,startpath)
	AllLevelPaths.append(startpath)
def GetOverrideVertexColor(data):
	vtxarray = []
	Lod = data["LODData"]
	for itLod in Lod:
		if HasKey("OverrideVertexColors",itLod):
			DataToConvert = itLod["OverrideVertexColors"]["Data"]
			for rgba_hex in DataToConvert:
				Color = unreal.BPFL.return_from_hex(rgba_hex)
				vtxarray.append(Color)
	return vtxarray
def SpawnMeshesInMap(data,set,mapname):
	for j in data:
		OvrVertexes = []
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
				PathToGo = ConvertToLoadableUE(ObjectProps["StaticMesh"],"StaticMesh ","Meshes")
			if HasTransform(ObjectProps) == False:
				LocalData = GetAttachScene(j,NameProp,data)
			Transform = GetTransform(ActualData["Properties"])
			if Transform == None:
				continue
			SMActor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.ValActor,Transform.translation,Transform.rotation.rotator())
			SMActor.set_actor_label(NameProp)
			SMActor.set_actor_scale3d(Transform.scale3d)
			MeshToLoad = unreal.load_asset(PathToGo)
			PathOriginal = GetActualPath(ObjectProps["StaticMesh"])
			if HasKey("LODData",ActualData):
				OvrVertexes = GetOverrideVertexColor(ActualData)
			if map_object.is_instanced():
				instance_data = ActualData["PerInstanceSMData"]
				Instance = SMActor.create_instance_component(MeshToLoad)
				SMActor.set_folder_path(f'Meshes/Instanced')
				for inst in instance_data:
					Trans = GetTransform(inst)
					if Trans == None:
						continue
					Transform = unreal.Transform(location=Trans.translation, rotation=Trans.rotation.rotator(), scale=Trans.scale3d)
					Instance.add_instance(Transform)
				if len(OvrVertexes) > 0:
					unreal.BPFL.paint_sm_vertices(Instance,OvrVertexes,PathOriginal)
			else:
				Instance = SMActor.create_static_component(MeshToLoad)
				FolderName = 'Meshes/Static'
				if mapname.endswith("_VFX"):
					FolderName = 'VFX/Meshes'
				SMActor.set_folder_path(FolderName)
				if len(OvrVertexes) > 0:
					unreal.BPFL.paint_sm_vertices(Instance,OvrVertexes,PathOriginal)
			SetAllSettings(ObjectProps,Instance)
			if HasKey("OverrideMaterials",ObjectProps):
				MatOver = GetMaterialToOverride(ActualData)
				if MatOver != None:
					Instance.set_editor_property('override_materials',MatOver) 

		#exit()




				#break

def GetActualPath(name):
	pathe = name["ObjectPath"]
	rfindpoint = pathe.rfind('.') 
	fixexportdir =str(Seting.export_path) +'\\export\\Game'
	fixedName = pathe[0:rfindpoint].replace("ShooterGame/Content",fixexportdir) + '.pskx'
	windowsfix = fixedName.replace("/","\\")
	return windowsfix
	#/find = name.rfind("/") + 1
	#newname = name[find:len(name)]

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
					if tex_game_path == None:
						continue
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
		if Seting.import_sublevel == True:
			CreateNewLevel(umap_name)
		import_umap(settings=settings, umap_data=umap_data, umap_name=umap_name)
		if Seting.import_sublevel == True:
			unreal.EditorLevelLibrary.save_current_level()
	if Seting.import_sublevel == True:
		LevelStreamingStuff()
	SetSMSettings()
	winsound.Beep(18000, 100)


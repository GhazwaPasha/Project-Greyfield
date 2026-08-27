import os
import unreal

editor_asset = unreal.EditorAssetLibrary
level_lib = unreal.EditorLevelLibrary
actor_subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
level_subsys = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

MAP_PATH = "/Game/Maps/Map_Small2v2"
MAP_FILE = "C:/Users/Ghazw/Desktop/Project Greyfield/Content/Maps/Map_Small2v2.umap"
SEED = 12345  # fixed seed for a reproducible, verifiable bake (not random per-match anymore)

if os.path.exists(MAP_FILE):
    os.remove(MAP_FILE)
    print("Removed stale map file:", MAP_FILE)

unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
print("Created blank in-memory level")

MOVABLE = unreal.ComponentMobility.MOVABLE

directional_light = actor_subsys.spawn_actor_from_class(
    unreal.DirectionalLight, unreal.Vector(0, 0, 2000), unreal.Rotator(-45, 45, 0)
)
directional_light.set_actor_label("Sun")
light_comp = directional_light.get_component_by_class(unreal.DirectionalLightComponent)
if light_comp:
    light_comp.set_editor_property("mobility", MOVABLE)
    light_comp.set_editor_property("intensity", 80000.0)
    light_comp.set_editor_property("atmosphere_sun_light", True)

sky_light = actor_subsys.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 2000), unreal.Rotator(0, 0, 0))
sky_light.set_actor_label("SkyLight")
sky_light_comp = sky_light.get_component_by_class(unreal.SkyLightComponent)
if sky_light_comp:
    sky_light_comp.set_editor_property("mobility", MOVABLE)
    sky_light_comp.set_editor_property("intensity", 1.0)
    sky_light_comp.set_editor_property("real_time_capture", True)

sky_atmosphere = actor_subsys.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
sky_atmosphere.set_actor_label("SkyAtmosphere")
sky_atmosphere.get_editor_property("root_component").set_editor_property("mobility", MOVABLE)

height_fog = actor_subsys.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
height_fog.set_actor_label("ExponentialHeightFog")
height_fog.get_editor_property("root_component").set_editor_property("mobility", MOVABLE)

pp_volume = actor_subsys.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
pp_volume.set_actor_label("PostProcessVolume")
pp_volume.set_editor_property("unbound", True)
pp_settings = pp_volume.get_editor_property("settings")
pp_settings.set_editor_property("override_auto_exposure_method", True)
pp_settings.set_editor_property("auto_exposure_method", unreal.AutoExposureMethod.AEM_HISTOGRAM)
pp_settings.set_editor_property("override_auto_exposure_min_brightness", True)
pp_settings.set_editor_property("auto_exposure_min_brightness", 0.03)
pp_settings.set_editor_property("override_auto_exposure_max_brightness", True)
pp_settings.set_editor_property("auto_exposure_max_brightness", 8.0)
pp_volume.set_editor_property("settings", pp_settings)

# --- The actual bake: generate the landscape NOW, at edit time, in this editor world ---
world = level_lib.get_editor_world()
landscape = unreal.GreyfieldMapGenerationSubsystem.generate_map_for_world(world, unreal.GreyfieldMapSize.SMALL2V2, SEED)
if landscape is None:
    print("!!! GenerateMap FAILED - returned None !!!")
else:
    print("GenerateMap succeeded:", landscape.get_name())
    print("Landscape material:", landscape.landscape_material.get_name() if landscape.landscape_material else "NULL")

# Plain GameMode (not the Procedural subclass) - the level is baked now, nothing to generate at
# runtime anymore.
world_settings = world.get_world_settings()
world_settings.set_editor_property("default_game_mode", unreal.GreyfieldGameMode)

saved = unreal.EditorLoadingAndSavingUtils.save_map(world, MAP_PATH)
print("save_map returned:", saved)
editor_asset.save_directory("/Game/Maps", False, True)

print("=== SUMMARY ===")
print("Level saved:", MAP_PATH, "seed:", SEED)

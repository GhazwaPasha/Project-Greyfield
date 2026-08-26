import os
import unreal

editor_asset = unreal.EditorAssetLibrary
level_lib = unreal.EditorLevelLibrary
actor_subsys = unreal.EditorActorSubsystem()

MAP_PATH = "/Game/Maps/Map_Small2v2"
MAP_FILE = "C:/Users/Ghazw/Desktop/Project Greyfield/Content/Maps/Map_Small2v2.umap"

# Fresh empty level (not World Partition - this is a small test map, doesn't need it). Checking
# the raw file rather than EditorAssetLibrary.does_asset_exist: on a fresh headless launch the
# asset registry scan may not have indexed a just-created file yet, making does_asset_exist race
# and return false right after a prior run's new_level() already saved one to disk.
if os.path.exists(MAP_FILE):
    os.remove(MAP_FILE)
    print("Removed stale map file:", MAP_FILE)

# new_level(path) validates the destination against the asset registry, which can still think a
# just-deleted map exists (registry cache lags the raw file delete above on a fresh headless
# launch). new_blank_map() instead creates an untitled in-memory level with no destination path
# at all, sidestepping that validation - we explicitly Save As at the very end instead.
unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
print("Created blank in-memory level, will Save As", MAP_PATH, "at the end")

# --- Lighting so the generated landscape is actually visible in PIE ---
# Mobility matched to NewMap's own working setup (2026-08-25 fix): this project runs
# r.AllowStaticLighting=False (fully dynamic lighting only, confirmed in DefaultEngine.ini), but
# spawn_actor_from_class defaults these to Stationary mobility - which can't render at all with
# static lighting disabled project-wide. Every lighting/sky actor in NewMap is explicitly Movable
# - matching that here.
MOVABLE = unreal.ComponentMobility.MOVABLE

directional_light = actor_subsys.spawn_actor_from_class(
    unreal.DirectionalLight, unreal.Vector(0, 0, 2000), unreal.Rotator(-45, 45, 0)
)
directional_light.set_actor_label("Sun")
light_comp = directional_light.get_component_by_class(unreal.DirectionalLightComponent)
if light_comp:
    light_comp.set_editor_property("mobility", MOVABLE)
    # 2026-08-26: WAS 6.0, copied from NewMap on the assumption "matches the known-working
    # level" would carry over safely. It didn't - UE5 DirectionalLight intensity is physical Lux
    # (real daylight is ~1,000-10,000 lux overcast, ~100,000 lux direct sun; 6.0 is deep-twilight
    # level, functionally imperceptible without heavy exposure compensation). NewMap's simple
    # placeholder-box scene apparently reads as acceptable at 6.0; this landscape, under real
    # PBR-ish material response and Lighting-Only view mode (which has zero exposure
    # compensation - it showed solid black too, not just Lit), evidently doesn't. 80000 matches
    # the physically-plausible daylight range Epic's own SkyAtmosphere-paired sample levels use.
    light_comp.set_editor_property("intensity", 80000.0)
    light_comp.set_editor_property("atmosphere_sun_light", True)

sky_light = actor_subsys.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 2000), unreal.Rotator(0, 0, 0))
sky_light.set_actor_label("SkyLight")
sky_light_comp = sky_light.get_component_by_class(unreal.SkyLightComponent)
if sky_light_comp:
    sky_light_comp.set_editor_property("mobility", MOVABLE)
    sky_light_comp.set_editor_property("intensity", 1.0)  # matches NewMap's SkyLight
    sky_light_comp.set_editor_property("real_time_capture", True)

sky_atmosphere = actor_subsys.spawn_actor_from_class(
    unreal.SkyAtmosphere, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0)
)
sky_atmosphere.set_actor_label("SkyAtmosphere")
sky_atmosphere_root = sky_atmosphere.get_editor_property("root_component")
if sky_atmosphere_root:
    sky_atmosphere_root.set_editor_property("mobility", MOVABLE)

height_fog = actor_subsys.spawn_actor_from_class(
    unreal.ExponentialHeightFog, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0)
)
height_fog.set_actor_label("ExponentialHeightFog")
height_fog_root = height_fog.get_editor_property("root_component")
if height_fog_root:
    height_fog_root.set_editor_property("mobility", MOVABLE)

# --- PostProcessVolume (2026-08-26 addition) ---
# Neither NewMap nor Map_Small2v2 had one (confirmed via headless inspection), so this wasn't
# actually the cause of the still-open Lit-mode-black bug (root cause #4 in this file's map-gen
# section) - but every level should have one regardless, since with no PostProcessVolume the
# project falls back to whatever Project Settings > Rendering > Default Settings specifies, with
# no per-level override available if that ever needs tuning. Unbound + wide exposure range so it
# can never itself clip the scene to black.
pp_volume = actor_subsys.spawn_actor_from_class(
    unreal.PostProcessVolume, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0)
)
pp_volume.set_actor_label("PostProcessVolume")
pp_volume.set_editor_property("unbound", True)
pp_settings = pp_volume.get_editor_property("settings")
pp_settings.set_editor_property("override_auto_exposure_method", True)
pp_settings.set_editor_property("auto_exposure_method", unreal.AutoExposureMethod.AEM_HISTOGRAM)
pp_settings.set_editor_property("override_auto_exposure_min_brightness", True)
pp_settings.set_editor_property("auto_exposure_min_brightness", 0.03)
pp_settings.set_editor_property("override_auto_exposure_max_brightness", True)
pp_settings.set_editor_property("auto_exposure_max_brightness", 8.0)
pp_settings.set_editor_property("override_auto_exposure_bias", True)
pp_settings.set_editor_property("auto_exposure_bias", 0.0)
pp_volume.set_editor_property("settings", pp_settings)

# No NavMeshBoundsVolume and no MapGenTestTrigger actor here anymore (2026-08-25 fix): the
# trigger's BeginPlay ran too late (after GameMode's own FindPlayerStart had already tried and
# fallen back to spawning the camera pawn at world origin - which put it inside solid landscape
# geometry once, seen as a black screen). Generation now runs from
# AGreyfieldGameMode_Procedural::InitGame, before any player spawn - see GreyfieldGameMode.h/.cpp.
# A NavMeshBoundsVolume with no real geometry to build against at edit time was also just
# producing an editor "needs rebuild" nag for no benefit yet (no units/pathfinding on generated
# maps this session) - real runtime nav mesh rebuild after generation is a separate follow-up.

# --- World Settings: use the procedural GameMode variant (generates the map in InitGame) ---
world = level_lib.get_editor_world()
world_settings = world.get_world_settings()
world_settings.set_editor_property("default_game_mode", unreal.GreyfieldGameMode_Procedural)

saved = unreal.EditorLoadingAndSavingUtils.save_map(world, MAP_PATH)
print("save_map returned:", saved)
editor_asset.save_directory("/Game/Maps", False, True)

print("=== SUMMARY ===")
print("Level saved:", MAP_PATH)
print("Actors placed: Sun, SkyLight, SkyAtmosphere, ExponentialHeightFog (all Movable, matching NewMap)")
print("GameMode: GreyfieldGameMode_Procedural (generates Small2v2 in InitGame, before player spawn)")

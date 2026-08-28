import unreal

MAP_PATH = "/Game/Maps/NewMap"

level_subsys = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level_subsys.load_level(MAP_PATH)
print("Loaded level:", MAP_PATH)

actor_subsys = unreal.EditorActorSubsystem()
all_actors = actor_subsys.get_all_level_actors()

for actor in all_actors:
    class_name = actor.get_class().get_name()
    if "NavMeshBoundsVolume" in class_name or "PlayerStart" in class_name or "GreyfieldBuilding" in class_name or "GreyfieldHQ" in class_name:
        loc = actor.get_actor_location()
        scale = actor.get_actor_scale3d()
        origin, extent = actor.get_actor_bounds(only_colliding_components=False)
        print(f"{class_name}: {actor.get_name()} at {loc} scale {scale} bounds_origin {origin} bounds_extent {extent}")

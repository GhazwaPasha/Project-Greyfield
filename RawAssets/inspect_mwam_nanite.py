import unreal

level_subsys = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actor_subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

MAP_PATH = "/Game/MWLandscapeAutoMaterial/Maps/LandscapeAutoMaterial_MountainRange_Example"
level_subsys.load_level(MAP_PATH)
actors = actor_subsys.get_all_level_actors()

for a in actors:
    if a.get_class().get_name() == "Landscape":
        print("--- Landscape:", a.get_actor_label(), "---")
        for prop in ["enable_nanite", "b_enable_nanite", "nanite_lod_index"]:
            try:
                print(f"  {prop}:", a.get_editor_property(prop))
            except Exception as e:
                print(f"  {prop}: <not found: {e}>")

print("=== END ===")

import unreal

level_subsys = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actor_subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

MAP_PATH = "/Game/MWLandscapeAutoMaterial/Maps/LandscapeAutoMaterial_MountainRange_Example"
level_subsys.load_level(MAP_PATH)
actors = actor_subsys.get_all_level_actors()

for a in actors:
    if a.get_class().get_name() == "Landscape":
        print("--- Landscape:", a.get_actor_label(), "---")
        try:
            info = a.get_landscape_info() if hasattr(a, "get_landscape_info") else None
            print("get_landscape_info:", info)
        except Exception as e:
            print("get_landscape_info failed:", e)
        try:
            layers = a.landscape_layers() if hasattr(a, "landscape_layers") else None
            print("landscape_layers():", layers)
        except Exception as e:
            print("landscape_layers() failed:", e)
        # Try LandscapeLibrary if it exists
        for libname in ["LandscapeLibrary", "EditorLandscapeLibrary"]:
            if hasattr(unreal, libname):
                print(f"{libname} exists")
                lib = getattr(unreal, libname)
                print("  dir:", [d for d in dir(lib) if not d.startswith("_")])

print("=== END ===")

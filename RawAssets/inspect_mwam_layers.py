import unreal

level_subsys = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actor_subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

MAP_PATH = "/Game/MWLandscapeAutoMaterial/Maps/LandscapeAutoMaterial_MountainRange_Example"
level_subsys.load_level(MAP_PATH)
actors = actor_subsys.get_all_level_actors()

for a in actors:
    if a.get_class().get_name() == "Landscape":
        print("--- Landscape:", a.get_actor_label(), "---")
        print("landscape material:", a.get_editor_property("landscape_material").get_name())

# Check for LandscapeLayerInfoObject assets anywhere in the whole project (not just the plugin
# folder, in case the example map's paint layers live under /Game/Maps or elsewhere)
registry = unreal.AssetRegistryHelpers.get_asset_registry()
filter_all = unreal.ARFilter(class_names=["LandscapeLayerInfoObject"], recursive_paths=True)
assets = registry.get_assets(filter_all)
print("ALL LandscapeLayerInfoObject assets in project:", [str(x.package_name) for x in assets])

print("=== END ===")

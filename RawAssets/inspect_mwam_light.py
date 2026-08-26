import unreal

level_subsys = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actor_subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

MAP_PATH = "/Game/MWLandscapeAutoMaterial/Maps/LandscapeAutoMaterial_MountainRange_Example"
level_subsys.load_level(MAP_PATH)
actors = actor_subsys.get_all_level_actors()

for a in actors:
    cls_name = a.get_class().get_name()
    if cls_name == "DirectionalLight":
        print("--- DirectionalLight:", a.get_actor_label(), "---")
        print("  rotation:", a.get_actor_rotation())
        comp = a.get_component_by_class(unreal.DirectionalLightComponent)
        for prop in ["affects_world", "atmosphere_sun_light", "cast_shadows", "cast_dynamic_shadows",
                     "indirect_lighting_intensity", "volumetric_scattering_intensity", "forward_shading_priority",
                     "temperature", "use_temperature", "light_source_angle", "cast_cloud_shadows",
                     "atmosphere_sun_light_index", "light_function_material", "specular_scale", "visible",
                     "cast_volumetric_shadow", "light_color"]:
            try:
                print(f"  {prop}:", comp.get_editor_property(prop))
            except Exception as e:
                print(f"  {prop}: <not found>")
    if cls_name == "Landscape":
        # check affects_world-equivalent / visible / hidden_in_game
        print("--- Landscape ---")
        for prop in ["hidden", "actor_hidden_in_game"]:
            try:
                print(f"  {prop}:", a.get_editor_property(prop))
            except Exception:
                pass
        root = a.get_editor_property("root_component")
        try:
            print("  visible:", root.get_editor_property("visible"))
        except Exception:
            pass
        try:
            print("  hidden_in_game:", root.get_editor_property("hidden_in_game"))
        except Exception:
            pass

print("=== END ===")

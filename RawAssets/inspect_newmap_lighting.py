import unreal

level_subsys = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level_subsys.load_level("/Game/Maps/NewMap")

actor_subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = actor_subsys.get_all_level_actors()

print("=== NewMap lighting/sky actors ===")
for a in actors:
    cls_name = a.get_class().get_name()
    if any(k in cls_name for k in ["DirectionalLight", "SkyLight", "SkyAtmosphere", "ExponentialHeightFog", "PostProcessVolume", "Sky"]):
        print("---", cls_name, "-", a.get_actor_label(), "---")
        root = a.get_editor_property("root_component")
        if root:
            print("  root mobility:", root.get_editor_property("mobility"))
        if "DirectionalLight" in cls_name:
            comp = a.get_component_by_class(unreal.DirectionalLightComponent)
            if comp:
                print("  intensity:", comp.get_editor_property("intensity"))
                print("  mobility:", comp.get_editor_property("mobility"))
                print("  atmosphere_sun_light:", comp.get_editor_property("atmosphere_sun_light"))
                print("  cast_shadows:", comp.get_editor_property("cast_shadows"))
                print("  visible:", comp.get_editor_property("visible"))
        elif "SkyLight" in cls_name:
            comp = a.get_component_by_class(unreal.SkyLightComponent)
            if comp:
                print("  intensity:", comp.get_editor_property("intensity"))
                print("  real_time_capture:", comp.get_editor_property("real_time_capture"))
                print("  source_type:", comp.get_editor_property("source_type"))
        elif "PostProcessVolume" in cls_name:
            print("  unbound:", a.get_editor_property("unbound"))
            settings = a.get_editor_property("settings")
            print("  auto_exposure_method override:", settings.get_editor_property("override_auto_exposure_method") if hasattr(settings, "get_editor_property") else "n/a")

print("=== END ===")

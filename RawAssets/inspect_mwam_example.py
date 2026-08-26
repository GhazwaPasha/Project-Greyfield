import unreal

level_subsys = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actor_subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

MAP_PATH = "/Game/MWLandscapeAutoMaterial/Maps/LandscapeAutoMaterial_MountainRange_Example"

print(f"\n========== {MAP_PATH} ==========")
level_subsys.load_level(MAP_PATH)
actors = actor_subsys.get_all_level_actors()
print("total actor count:", len(actors))

class_names = sorted(set(a.get_class().get_name() for a in actors))
print("all actor classes present:", class_names)

for a in actors:
    cls_name = a.get_class().get_name()
    if "RuntimeVirtualTexture" in cls_name:
        print("---", cls_name, "-", a.get_actor_label(), "---")
        print("  location:", a.get_actor_location())
        print("  scale:", a.get_actor_scale3d())
        try:
            vt = a.get_editor_property("virtual_texture")
            print("  virtual_texture:", vt.get_name() if vt else None)
        except Exception as e:
            print("  virtual_texture read failed:", e)
    if cls_name == "Landscape":
        print("---", cls_name, "-", a.get_actor_label(), "---")
        try:
            rvts = a.get_editor_property("runtime_virtual_textures")
            print("  runtime_virtual_textures:", [t.get_name() for t in rvts] if rvts else rvts)
        except Exception as e:
            print("  runtime_virtual_textures read failed:", e)
        try:
            mat = a.get_editor_property("landscape_material")
            print("  landscape_material:", mat.get_name() if mat else None)
        except Exception as e:
            print("  landscape_material read failed:", e)
        for prop in ["nanite_landscape", "cast_shadow", "lighting_channels", "static_lighting_resolution",
                     "num_subsections", "num_sections_per_component", "component_size_quads"]:
            try:
                print(f"  {prop}:", a.get_editor_property(prop))
            except Exception as e:
                print(f"  {prop} read failed:", e)
        root = a.get_editor_property("root_component")
        if root:
            try:
                print("  root cast_shadow:", root.get_editor_property("cast_shadow"))
            except Exception as e:
                print("  root cast_shadow read failed:", e)
            try:
                print("  root lighting_channels:", root.get_editor_property("lighting_channels"))
            except Exception as e:
                print("  root lighting_channels read failed:", e)
    if cls_name == "SkyLight":
        comp = a.get_component_by_class(unreal.SkyLightComponent)
        if comp:
            print("--- SkyLight -", a.get_actor_label(), "intensity:", comp.get_editor_property("intensity"),
                  "real_time_capture:", comp.get_editor_property("real_time_capture"))
    if cls_name == "PostProcessVolume":
        print("---", cls_name, "-", a.get_actor_label(), "unbound:", a.get_editor_property("unbound"), "---")
    if cls_name == "DirectionalLight":
        comp = a.get_component_by_class(unreal.DirectionalLightComponent)
        if comp:
            print("--- DirectionalLight -", a.get_actor_label(), "intensity:", comp.get_editor_property("intensity"),
                  "mobility:", comp.get_editor_property("mobility"))
    if "LightmassImportanceVolume" in cls_name:
        print("---", cls_name, "-", a.get_actor_label(), "---")

print("=== END ===")

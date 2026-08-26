import unreal

pipeline = unreal.InterchangeGenericAssetsPipeline()
print("=== top-level pipeline attrs ===")
for name in dir(pipeline):
    if not name.startswith("_"):
        print(name)

for sub_attr in ["mesh_pipeline", "common_meshes_properties", "common_skeletal_meshes_and_animations_properties"]:
    try:
        sub = getattr(pipeline, sub_attr)
        print("=== %s (%s) ===" % (sub_attr, type(sub)))
        for name in dir(sub):
            if not name.startswith("_") and ("combine" in name.lower() or "merge" in name.lower() or "static" in name.lower()):
                print(" ", name)
    except Exception as e:
        print("no attr", sub_attr, e)

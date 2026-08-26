import bpy
import os

RAW_ROOT = "C:/Users/Ghazw/Desktop/Project Greyfield/RawAssets/Units"

files = [
    "Aircraft/CrescentCoalition/kaan.glb",
    "Aircraft/RedDragon/h-20.glb",
    "Aircraft/RedDragon/j-10-vigorous-dragon.glb",
    "Aircraft/RedDragon/j-16-hidden-dragon.glb",
    "Aircraft/RedDragon/j-20.glb",
    "Aircraft/RedDragon/j-31-gyrfalcon.glb",
    "Aircraft/RedDragon/j-50.glb",
    "Aircraft/UnitedAlliance/b-2-spirit.glb",
    "Aircraft/UnitedAlliance/f-15c-eagle.glb",
    "Aircraft/UnitedAlliance/f-16.glb",
    "Aircraft/UnitedAlliance/f-22-raptor.glb",
    "Aircraft/UnitedAlliance/f-35b-lightning-ii.glb",
    "Infantry/CrescentCoalition/crescent-infantry.glb",
    "Infantry/GlobalLiberationFront/glf-infantry.glb",
    "Infantry/RedDragon/red-dragon-infantry.glb",
    "Infantry/UnitedAlliance/united-infantry.glb",
]

results = []

for rel in files:
    src = os.path.join(RAW_ROOT, rel).replace("\\", "/")
    dst = src[:-4] + "_merged.glb"

    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=src)

    mesh_objs = [o for o in bpy.context.scene.objects if o.type == "MESH"]
    armature_objs = [o for o in bpy.context.scene.objects if o.type == "ARMATURE"]
    before = len(mesh_objs)

    if len(mesh_objs) > 1 and not armature_objs:
        # Static (unrigged) props: join all mesh parts into a single mesh object,
        # preserving per-part materials as material slots.
        bpy.ops.object.select_all(action="DESELECT")
        bpy.context.view_layer.objects.active = mesh_objs[0]
        for o in mesh_objs:
            o.select_set(True)
        bpy.ops.object.join()

    mesh_objs_after = [o for o in bpy.context.scene.objects if o.type == "MESH"]
    after = len(mesh_objs_after)

    bpy.ops.export_scene.gltf(filepath=dst, export_format="GLB")

    results.append((rel, before, after, len(armature_objs)))
    print("PROCESSED: {} meshes_before={} meshes_after={} armatures={}".format(rel, before, after, len(armature_objs)))

print("=== MERGE SUMMARY ===")
for rel, before, after, arm in results:
    print(" - {}: {} -> {} mesh(es), {} armature(s)".format(rel, before, after, arm))

import bpy
import os

RAW_ROOT = "C:/Users/Ghazw/Desktop/Project Greyfield/RawAssets/Units"

# Scoped re-run, 2026-08-25: only the 3 newly-replaced infantry models
# (United Alliance, Crescent Coalition, Red Dragon). These have NO armature
# (confirmed via reimport into UE: fragmented into 20-40 disconnected
# StaticMesh pieces), so this joins them into one static mesh the same way
# unrigged aircraft parts get joined -- this does NOT add a skeleton, it
# only prepares a single clean mesh for the Mixamo auto-rig upload step.
files = [
    "Infantry/CrescentCoalition/crescent-infantry.glb",
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
        bpy.ops.object.select_all(action="DESELECT")
        bpy.context.view_layer.objects.active = mesh_objs[0]
        for o in mesh_objs:
            o.select_set(True)
        bpy.ops.object.join()

    mesh_objs_after = [o for o in bpy.context.scene.objects if o.type == "MESH"]
    after = len(mesh_objs_after)

    bpy.ops.export_scene.gltf(filepath=dst, export_format="GLB")

    # Also export FBX for the Mixamo upload step (Mixamo takes FBX/OBJ, not GLB).
    fbx_dst = src[:-4] + "_merged.fbx"
    bpy.ops.export_scene.fbx(filepath=fbx_dst)

    results.append((rel, before, after, len(armature_objs)))
    print("PROCESSED: {} meshes_before={} meshes_after={} armatures={}".format(rel, before, after, len(armature_objs)))

print("=== MERGE SUMMARY ===")
for rel, before, after, arm in results:
    print(" - {}: {} -> {} mesh(es), {} armature(s)".format(rel, before, after, arm))

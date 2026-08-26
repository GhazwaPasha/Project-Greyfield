import bpy

src = "C:/Users/Ghazw/Desktop/Project Greyfield/RawAssets/Units/Aircraft/UnitedAlliance/f22_raptor_new.glb"
dst = "C:/Users/Ghazw/Desktop/Project Greyfield/RawAssets/Units/Aircraft/UnitedAlliance/f22_raptor_new_merged.glb"

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

print("PROCESSED: f22_raptor_new.glb meshes_before={} meshes_after={} armatures={}".format(before, after, len(armature_objs)))

import unreal

mgr = unreal.InterchangeManager.get_interchange_manager()

print("=== InterchangeManager methods with 'scene' or 'level' ===")
for name in dir(mgr):
    if not name.startswith("_") and ("scene" in name.lower() or "level" in name.lower()):
        print(" ", name)

for candidate in ["import_scene", "import_scene_async"]:
    fn = getattr(mgr, candidate, None)
    print("---", candidate, "---")
    print("doc:", getattr(fn, "__doc__", None))

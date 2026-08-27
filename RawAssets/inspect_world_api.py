import unreal

print("has GreyfieldMapSize:", hasattr(unreal, "GreyfieldMapSize"))
if hasattr(unreal, "GreyfieldMapSize"):
    enum_cls = unreal.GreyfieldMapSize
    print("enum values:", list(enum_cls))

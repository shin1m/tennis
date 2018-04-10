import os
import bpy
D = bpy.data

D.objects['Armature'].select = True
bpy.ops.wm.collada_export(
    filepath=os.path.splitext(D.filepath)[0] + '.dae',
    selected=True,
    include_children=True,
    include_armatures=True,
    deform_bones_only=True
)

import os
import bpy
D = bpy.data

D.actions['ArmatureAction'].name = '0ArmatureAction'
bpy.ops.wm.append(directory=os.path.dirname(__file__) + '//male.blend/Action/', filepath='male.blend', filename='ArmatureAction')
D.objects['Armature'].animation_data.action = D.actions['ArmatureAction']

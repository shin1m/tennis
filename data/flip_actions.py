import re
import bpy
from mathutils import Quaternion
D = bpy.data

pattern_data_path = re.compile(r'pose\.bones\["((?:\w|\.)+)"\]\.(\w+)')

curves = D.actions['ArmatureAction'].fcurves
path2index = {}
i = 0
for curve in curves:
    match = pattern_data_path.match(curve.data_path)
    bone = match.group(1)
    field = match.group(2)
    if curve.array_index == 0:
        path2index[(bone, field)] = i
    i += 1

def flip_location(bone):
    i = path2index[(bone, 'location')]
    for frame in curves[i].keyframe_points:
        frame.co[1] = -frame.co[1]

def rotation(i, j):
    w = curves[i].keyframe_points[j]
    x = curves[i + 1].keyframe_points[j]
    y = curves[i + 2].keyframe_points[j]
    z = curves[i + 3].keyframe_points[j]
    return w, x, y, z, Quaternion([w.co[1], x.co[1], y.co[1], z.co[1]])

def flip_x(w, x, y, z, q):
    w.co[1] = q.w
    x.co[1] = q.x
    y.co[1] = -q.y
    z.co[1] = -q.z

def flip_z(w, x, y, z, q):
    w.co[1] = q.w
    x.co[1] = -q.x
    y.co[1] = -q.y
    z.co[1] = q.z

def flip_rotation(bone, flip):
    i = path2index[(bone, 'rotation_quaternion')]
    for j in range(len(curves[i].keyframe_points)):
        flip(*rotation(i, j))

def flip_rotations(bone, flip):
    i0 = path2index[(bone + '.L', 'rotation_quaternion')]
    i1 = path2index[(bone + '.R', 'rotation_quaternion')]
    for j in range(len(curves[i0].keyframe_points)):
        w0, x0, y0, z0, q0 = rotation(i0, j)
        w1, x1, y1, z1, q1 = rotation(i1, j)
        flip(w0, x0, y0, z0, q1)
        flip(w1, x1, y1, z1, q0)

flip_location('Root')
flip_rotation('Root', flip_x)
flip_location('Center')
flip_rotation('Center', flip_z)
flip_location('Back')
flip_rotation('Back', flip_z)
flip_location('Neck')
flip_rotation('Neck', flip_z)
flip_location('Head')
flip_rotation('Head', flip_z)

flip_rotations('Leg0', flip_z)
flip_rotations('Leg1', flip_z)
flip_rotations('Foot', flip_z)
flip_rotations('Toe', flip_z)
flip_rotations('Shoulder', flip_x)
flip_rotations('Arm0', flip_z)
flip_rotations('Arm1', flip_z)
flip_rotations('Hand', flip_z)

flip_rotation('Racket', flip_z)

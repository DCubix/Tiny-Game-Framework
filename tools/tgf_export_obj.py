bl_info = {
	"name": "Tiny Game Framework's Tiny Model Exporter",
	"author": "Diego Lopes",
	"version": ( 1, 5, 0 ),
	"blender": ( 2, 7, 9 ),
	"location": "File > Export > Tiny Model (.tmd)",
	"description": "Tiny Model (.tmd)",
	"warning": "",
	"wiki_url": "",
	"category": "Import-Export"
}

import bpy
import bmesh
import collections
import struct
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty
from bpy.types import Operator
from mathutils import Matrix


"""
HEADER
----------------------------------------------------------
IDENTIFIER          |  3 bytes "TMD"
VERTEX COUNT        |  4 bytes (int) 
UV COUNT            |  4 bytes (int) 
FACE COUNT          |  4 bytes (int)
KEYFRAME COUNT      |  4 bytes (int)
HAS UVS             |  1 byte (1 = has uvs, 0 = doesn't)
----------------------------------------------------------
KEYFRAME DATA
----------------------------------------------------------
Sequence of ints of length "KEYFRAME COUNT":
	1, 2, 6, ...
----------------------------------------------------------
VERTEX DATA
----------------------------------------------------------
Sequence of floats of length "VERTEX COUNT":
	x, y, z, ...
----------------------------------------------------------
UV DATA
----------------------------------------------------------
Sequence of floats of length "UV COUNT":
	x, y, ...
----------------------------------------------------------
FACE DATA
----------------------------------------------------------
Sequence of ints of length "FACE COUNT":
	If it has uvs: v0, t0, v1, t1, v2, t2, ...
	Else:          v0, v1, v2, ...
"""

def getActions(obj):
	if obj.animation_data is not None and obj.animation_data.action is not None:
		yield obj.animation_data.action
	if obj.data.shape_keys is not None and \
		obj.data.shape_keys.animation_data is not None and \
		obj.data.shape_keys.animation_data.action is not None:
		yield obj.data.shape_keys.animation_data.action

def writeFile(obj, fileName):
	sce = bpy.context.screen.scene

	## 1. Get all Frames
	frames = []
	lastFrame = 0
	maxFrame = 0
	
	for action in getActions(obj):
		if action is not None:
			start = int(action.frame_range[0])
			end = int(action.frame_range[1])

			for fcurve in action.fcurves:
				for keyframe_point in fcurve.keyframe_points:
					x, y = keyframe_point.co
					if x >= start and x <= end and x not in frames:
						frames.append(maxFrame + int(x))
						lastFrame = int(x)
			maxFrame = max(maxFrame, lastFrame+1)
	
	if len(frames) == 0:
		frames = [1]
	frames.sort()

	VERTICES = collections.OrderedDict()
	UVS = []
	FACES = []
	
	## 2. Get mesh data for each keyframe	
	for frame in frames:
		VERTICES[frame] = []
		bpy.context.scene.frame_set(frame)
		mesh = obj.to_mesh(sce, True, "PREVIEW")
		
		_, rot, scale = obj.matrix_world.decompose()
		R = rot.to_matrix().to_4x4()
		S = Matrix()
		S[0][0] = scale[0]
		S[1][1] = scale[1]
		S[2][2] = scale[2]
		mat = R * S
			
		bm = bmesh.new()
		bm.from_mesh(mesh)
		bmesh.ops.triangulate(bm, faces=bm.faces)
		bm.to_mesh(mesh)
		bm.free()
		del bm
		
		## 2.1. Get POSITIONS
		for vert in mesh.vertices:
			co = mat * vert.co
			VERTICES[frame].append((-co[0], -co[2], co[1]))
	bpy.context.scene.frame_set(frames[0])
	
	## 3. Get UVS
	hasUVs = False
	if len(mesh.uv_layers) > 0:
		hasUVs = True
		for layer in mesh.uv_layers[0].data:
			UVS.append((layer.uv[0], 1.0 - layer.uv[1]))

	## 4. Get FACE INDICES Vi/Ti
	for poly in mesh.polygons:
		for loop_index in range(poly.loop_start, poly.loop_start + poly.loop_total):
			vi = mesh.loops[loop_index].vertex_index
			if hasUVs:
				ti = loop_index
				FACES.append((vi, ti))
			else:
				FACES.append(vi)
	
	## 5. MAKE STRUCT
	string = []
	
	## 5.1. WRITE KEYFRAME DATA
	string.append(struct.pack("I" * len(frames), *frames))
	
	## 5.2. WRITE VERTEX DATA
	VERTS = []
	for _, v in VERTICES.items():
		for vt in v:
			VERTS.extend(vt)
	string.append(struct.pack("f" * len(VERTS), *VERTS))
	
	## 5.3. WRITE UV DATA
	UV = []
	for uv in UVS:
		UV.extend(uv)
	string.append(struct.pack("f" * len(UV), *UV))
	
	## 5.3. WRITE FACE DATA
	FACS = []
	if hasUVs:
		for f in FACES:
			FACS.extend(f)
	else:
		FACS = FACES
	string.append(struct.pack("I" * len(FACS), *FACS))

	header = []
	file = []
	
	## WRITE HEADER
	header.append(struct.pack("ccc", b"T", b"M", b"D"))
	header.append(struct.pack("I", len(VERTICES[frames[0]])))
	header.append(struct.pack("I", len(UVS)))
	header.append(struct.pack("I", len(FACES)))
	header.append(struct.pack("I", len(frames)))
	header.append(struct.pack("B", 1 if hasUVs else 0))
	
	file.extend(header)
	file.extend(string)
	
	fileString = b"".join(file)

	if len(fileName) > 0:
		with open(fileName, "wb") as f:
			f.write(fileString)

	return {'FINISHED'}
	
class ExportTMD(Operator, ExportHelper):
	"""Tiny Game Framework's Tiny Model Format"""
	bl_idname = "tgf.tmd"
	bl_label = "Export Tiny Model"

	filename_ext = ".tmd"

	def execute(self, context):
		return writeFile(bpy.context.active_object, self.filepath)

def menu_func_export(self, context):
    self.layout.operator(ExportTMD.bl_idname, text="Tiny Model (.tmd)")

def register():
	bpy.utils.register_class(ExportTMD)
	bpy.types.INFO_MT_file_export.append(menu_func_export)


def unregister():
	try:
		bpy.utils.unregister_class(ExportTMD)
		bpy.types.INFO_MT_file_export.remove(menu_func_export)
	except:
		pass


if __name__ == "__main__":
	register()

def test():
	writeFile(bpy.context.active_object, "")
#test()
import ctypes
import numpy as np

from pyOpenSubdiv.clib import load_library
OpenSubdiv_clib = load_library.load_library()

def pysubdivide(subdivision_level,vertices,faceVerts,vertsPerFace):
    """
    Documentation
    """
    
    ################ Subdivide ################    
    refine_topology = OpenSubdiv_clib.subdivider_refine_topology
    refine_topology.argtypes = [
        ctypes.c_int, # maxlevel
        ctypes.c_int, # n_verts
        ctypes.c_int, # n_faces     
        ctypes.POINTER((ctypes.c_float)*3), # vertices 
        ctypes.POINTER(ctypes.c_int), # faceVerts
        ctypes.POINTER(ctypes.c_int) # vertsPerFace
    ]
    
    n_verts = len(vertices)
    n_faces = len(vertsPerFace)
    vertices_array = ctypes.ARRAY(n_verts,ctypes.c_float*3)(*[(ctypes.c_float*3)(*vert) for vert in vertices])
    faceVerts_array = (ctypes.c_int*len(faceVerts))(*faceVerts)
    vertsPerFace_array = (ctypes.c_int*n_faces)(*vertsPerFace)

    refine_topology(
        subdivision_level,
        n_verts,
        n_faces,
        vertices_array, # vertices (as c array) 
        faceVerts_array, # faceVerts (as c array)
        vertsPerFace_array # vertsPerFace (as c array)
    )

    ################ Get Results ################
    #### Gather New Mesh Information ####
    OpenSubdiv_clib.nn_verts.restypes = ctypes.c_int 
    new_nverts = OpenSubdiv_clib.nn_verts()

    OpenSubdiv_clib.nn_edges.restypes = ctypes.c_int
    new_nedges = OpenSubdiv_clib.nn_edges()

    OpenSubdiv_clib.nn_faces.restypes = ctypes.c_int
    new_nfaces = OpenSubdiv_clib.nn_faces()

    #### Extract New Vertices #### 
    # new_vert_arrays = []
    # for i in range(new_nverts):
    #     new_vert_arrays.append((ctypes.c_float*3)(*[0,0,0]))
    # (ctypes.c_float*3)(*[0,0,0])
    # Turns out you can do this in one line 
    # It's absolutely absurd, though. 
    new_vertices = (ctypes.ARRAY(new_nverts,ctypes.c_float*3))(*[(ctypes.c_float*3)(*[0,0,0])])

    OpenSubdiv_clib.new_vertices.argtypes = [ctypes.ARRAY(new_nverts,ctypes.c_float*3)]
    OpenSubdiv_clib.new_vertices(new_vertices)

    #### Extract New Edges #### 
    new_edges = (ctypes.ARRAY(new_nedges,ctypes.c_int*2))(*[(ctypes.c_int*2)(*[0,0])])

    OpenSubdiv_clib.new_edges.argtypes = [ctypes.ARRAY(new_nedges,ctypes.c_int*2)]
    OpenSubdiv_clib.new_edges(new_edges)

    #### Extract New Faces #### 
    new_faces = (ctypes.ARRAY(new_nfaces,ctypes.c_int*4))(*[(ctypes.c_int*4)(*[0,0,0,0])])

    OpenSubdiv_clib.new_faces.argtypes = [ctypes.ARRAY(new_nfaces,ctypes.c_int*4)]
    OpenSubdiv_clib.new_faces(new_faces)

    ################ Return ################
    # print(timeit.timeit(lambda: np.ctypeslib.as_array(new_vertices),number=10000)) # This is instantaneous 
    # print(timeit.timeit(lambda: np.ctypeslib.as_array(new_vertices).tolist(),number=10000)) # This is ridiculously slow 

    # tolist() is quite slow but it seems necessary for blender. 
    # Er, well, maybe it's not that bad idk. 
    new_mesh = {
        'vertices' : np.ctypeslib.as_array(new_vertices).tolist(),
        'edges' : np.ctypeslib.as_array(new_edges).tolist(),
        'faces' : np.ctypeslib.as_array(new_faces).tolist()
    }
    return new_mesh 
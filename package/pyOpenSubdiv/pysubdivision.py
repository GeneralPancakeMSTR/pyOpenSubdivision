import ctypes
import numpy as np
import sys
import traceback

from pyOpenSubdiv.clib import load_library
OpenSubdiv_clib = load_library.load_library()

def pysubdivide(subdivision_level,
    vertices,    
    faces, # Used in the maxlevel = 0 case
    faceVerts,
    vertsPerFace,
    verbose=False,    
    mesh=False, # Not implemented 
    obj=False): # Not implemented 

    """
    Documentation
    """   

    ################ Subdivide ################
    subdivider_settings = OpenSubdiv_clib.subdivider_settings
    subdivider_settings.argtypes = [
        ctypes.c_int, # maxlevel 
        ctypes.c_int # verbose
    ]    

    subdivider_settings(subdivision_level,verbose)

    refine_topology = OpenSubdiv_clib.subdivider_refine_topology
    refine_topology.argtypes = [
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
        n_verts,
        n_faces,
        vertices_array, # vertices (as c array) 
        faceVerts_array, # faceVerts (as c array)
        vertsPerFace_array # vertsPerFace (as c array)
    )

    if(subdivision_level <= 0):
        OpenSubdiv_clib.nn_edges.restypes = ctypes.c_int
        new_nedges = OpenSubdiv_clib.nn_edges()

        #### Extract New Edges #### 
        new_edges = (ctypes.ARRAY(new_nedges,ctypes.c_int*2))(*[(ctypes.c_int*2)(*[0,0])])

        OpenSubdiv_clib.new_edges.argtypes = [ctypes.ARRAY(new_nedges,ctypes.c_int*2)]
        OpenSubdiv_clib.new_edges(new_edges)

        #### Reconstruct faces #### 
        if(not faces):
            # This block will never run, because I'm passing faces in as a parameter. 
            # But in the case that you wanted to reconstruct the faces from verstPerface and faceVerts,
            # this is the code to use. 
            # It's slightly slower than just passing the faces in, but not enormously. 
            arr_pos = 0         
            faces = [] 
            for i,verts_in_face in enumerate(vertsPerFace):
                face = []         
                for j in range(verts_in_face): 
                    face.append(faceVerts[arr_pos + j])
                faces.append(face)
                arr_pos+= verts_in_face            
        
        #### Results #### 
        new_mesh = {
            'vertices' : vertices,
            'edges' : np.ctypeslib.as_array(new_edges).tolist(),
            'faces' : faces
        }
        return new_mesh             
    elif(subdivision_level > 0):
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


# [ ] Convert to actual unit test 
def test_pysubdivide():
    from itertools import chain
    import timeit
    import sys

    from pyOpenSubdiv import test_topology

    meshes = {
        'cube':test_topology.cube,
        'suzanne':test_topology.suzanne,
        'triangles':test_topology.triangles,
        'ngons':test_topology.ngons,
        'ngons2':test_topology.ngons2
    }

    print('Subdivision Tests')
    for maxlevel in range(3):
        for topo in meshes: 
            print(f'{topo} @ {maxlevel}')
            mesh = meshes[topo]

            verts = mesh['verts']
            faces = mesh['faces']
            faceVerts = list(chain.from_iterable(faces))
            vertsPerFace = [len(face) for face in faces]

            test_mesh = pysubdivide(maxlevel,verts,faces,faceVerts,vertsPerFace,verbose=False)
    print()

    print('Verbose Test')
    mesh = meshes['cube']

    verts = mesh['verts']
    faces = mesh['faces']
    faceVerts = list(chain.from_iterable(faces))
    vertsPerFace = [len(face) for face in faces]

    test_mesh = pysubdivide(1,verts,faces,faceVerts,vertsPerFace,verbose=True)
    print()

    print('Runtime')
    for maxlevel in range(3):
        iterations = 10000

        runtime = timeit.timeit(lambda: pysubdivide(maxlevel,verts,faces,faceVerts,vertsPerFace,verbose=False),number=iterations)
        print(f'Cube: {runtime:.3f}s @ {maxlevel} x {iterations} ')
    print()
    
    print('Output')
    for i,vert in enumerate(test_mesh['vertices']):
        print(f'v {vert}')
        if(i>8):
            print('...')
            break

    for i,edge in enumerate(test_mesh['edges']):
        print(f'e {edge}')
        if(i>8):
            print('...')
            break

    for i,face in enumerate(test_mesh['faces']):
        print(f'f {face}')
        if(i>8):
            print('...')
            break
    print()
    
    test_topology.convert_to_cpp(meshes['cube'])

if __name__ == "__main__":
    pass




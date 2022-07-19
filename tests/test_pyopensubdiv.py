import timeit 
from itertools import chain

from pyOpenSubdiv.pysubdivision import pysubdivide

verts = [
    [-0.5,-0.5, 0.5],
    [ 0.5,-0.5, 0.5],
    [-0.5, 0.5, 0.5],
    [ 0.5, 0.5, 0.5],
    [-0.5, 0.5,-0.5],
    [ 0.5, 0.5,-0.5],
    [-0.5,-0.5,-0.5],
    [ 0.5,-0.5,-0.5]
]

faces = [
    [0, 1, 3, 2],
    [2, 3, 5, 4],
    [4, 5, 7, 6],
    [6, 7, 1, 0],
    [1, 7, 5, 3],
    [6, 0, 2, 4]
    ]

faceVerts = list(chain.from_iterable(faces))
vertsPerFace = [len(face) for face in faces]

new_mesh = pysubdivide(1,verts,faceVerts,vertsPerFace)

for i,vert in enumerate(new_mesh['vertices']):
    print(f'v {vert}')
    if(i>8):
        print('...')
        break

for i,edge in enumerate(new_mesh['edges']):
    print(f'e {edge}')
    if(i>8):
        print('...')
        break

for i,face in enumerate(new_mesh['faces']):
    print(f'f {face}')
    if(i>8):
        print('...')
        break

# Pretty fast? 
# print(timeit.timeit(lambda: pyOpenSubdiv(2,verts,faceVerts,vertsPerFace),number = 10000))
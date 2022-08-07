import ctypes
import os 
from sys import platform 
def load_library():
    if platform == 'linux' or platform == 'linux2':        
        OpenSubdiv_clib = ctypes.CDLL(os.path.join(os.path.dirname(__file__),'ctypes_OpenSubdiv.so'))
    elif platform == 'darwin':
        # OSX 
        pass
    elif platform == 'win32':        
        here = os.path.dirname(__file__).replace('\\','/') 
        OpenSubdiv_clib = ctypes.CDLL(os.path.join(here,"ctypes_OpenSubdiv.dll"))
    return OpenSubdiv_clib

if __name__ == "__main__":
    OpenSubdiv_clib = load_library()

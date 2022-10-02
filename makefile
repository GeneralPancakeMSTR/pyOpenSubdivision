default:
	g++ ctypes_subdivider.cpp -losdGPU -losdCPU -o ctypes_OpenSubdiv.so -fPIC -shared

executable:
	g++ ctypes_subdivider.cpp -losdGPU -losdCPU -o ctypes_OpenSubdiv

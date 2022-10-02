#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

// This actually works? 
// https://stackoverflow.com/a/25155315/2391876
#ifdef _WIN32 // This is what Visual Studio defines 
#define DLLEXPORT __declspec(dllexport)
// This resolves the "M_PI (pi) is not defined properly" error. 
// (https://stackoverflow.com/questions/6563810/m-pi-works-with-math-h-but-not-with-cmath-in-visual-studio)
// There may be better approaches to this, but I'm not sure what. 
// I also really feel like this should not be necessary, see OpenSubDiv -> CMakeLists.txt -> /D_USE_MATH_DEFINES. 
#define _USE_MATH_DEFINES
#include <math.h>
#elif __linux__
#define DLLEXPORT 
#elif __APPLE__
// [ ] Implement 
#endif

//---------------- Compile Instructions ----------------

//-------- Windows - Visual Studio --------
// 1) Download, compile, and install OpenSubdiv 
//   -Install git 
//   -Install CMake 
//   -Install GLFW (https://www.glfw.org/download) (I don't think you actually need this, but it might make certain parts smoother?)
//     - Download the windows 64 precompiled binaries. 
//     - In the unzipped folder, make a new directory 'lib', put all the lib files for your Visual Studio version in there, e.g. move everything in /lib-vc2022 to /lib
//   - Clone opensubdiv repo. 
//   - cd into OpenSubdiv repo, mkdir build, cd build.  
//   - Build Step 1, Initial build: 
//
//       cmake ^
//           -G "Visual Studio 15 2017 Win64" ^
//           -D NO_PTEX=1 -D NO_DOC=1 ^
//           -D NO_OMP=1 -D NO_TBB=1 -D NO_CUDA=1 -D NO_OPENCL=1 -D NO_CLEW=1 ^
//           -D "GLFW_LOCATION=*YOUR GLFW INSTALL LOCATION*" ^
//           ..
//
//   - You MUST specify the architecture (x64), or else it tries to build with x86 Windows libraries, or something. 
//   - See: https://github.com/PixarAnimationStudios/OpenSubdiv/issues/1245
//   - DO NOT have MSYS installed. Somehow CMake seeks it out and finds it, and the build process doesn't go well. 
//   - The exact build command I used was (vary appropriately for paths, e.g. glfw, and Visual Studio Version) 
// 
//      cmake ^ -DCMAKE_GENERATOR_PLATFORM=x64 -G "Visual Studio 17 2022" ^ -D NO_PTEX=1 -D NO_DOC=1 ^ -D NO_OMP=1 -D NO_TBB=1 -D NO_CUDA=1 -D NO_OPENCL=1 -D NO_CLEW=1 ^ -D "GLFW_LOCATION=C:/Users/<username>/Desktop/cpp/glfw-3.3.7.bin.WIN64/glfw-3.3.7.bin.WIN64" ^ ..
// 
//   - Build Step 2, Install build (run this in an administrator console): 
// 
//       cmake --build . --config Release --target install
// 
//   - Should create bin, include, and lib directores under C:\Program Files\OpenSubdiv
//
// 2) Create and Configure Visual Studio Project 
//  - Create a new blank C++ project in Visual Studio (Note: NOT visual studio CODE, but actually Microsoft Visual Studio)
//  - Create a blank Source.cpp file under "Source Files"
//  - Configure the solution properties (All builds, All Platforms):
//      - Add OpenSubdiv include to additional include directories: 
//          - Properties -> C/C++ -> General -> Additional Include Directores -> C:\Program Files\OpenSubdiv\include
//      - Add OpenSubdiv libs to additional Library directories:
//          - Properties -> Linker -> General -> Additional Library Directories -> C:\Program Files\OpenSubdiv\lib
//      - Add OpenSubdiv library binaries to linker's Additional Dependencies (https://stackoverflow.com/questions/42867030/c-dll-unresolved-external-symbol/42867190#42867190):
//          - Properties -> Linker -> Input -> Additional Dependencies -> osdCPU.lib;osdGPU.lib
//          - This resolves the "unresolved external" at compile time.
//      - Compile DLL: Properties -> Configuration Properties -> Configuration Type -> Dynamic Library (.dll)
//  - You also need to prefix all of the C-wrapped functions (everything in extern "C") with '__declspec(dllexport)', 
//      otherwise python won't be able to find the functions from the imported .dll. I'm 100% sure why this isn't necessary on linux. 

//---------------- Vertex container implementation. ----------------
struct Vertex {
    // Minimal required interface ----------------------
    Vertex() { }

    Vertex(Vertex const& src) {
        _position[0] = src._position[0];
        _position[1] = src._position[1];
        _position[2] = src._position[2];
    }

    void Clear(void* = 0) {
        _position[0] = _position[1] = _position[2] = 0.0f;
    }

    void AddWithWeight(Vertex const& src, float weight) {
        _position[0] += weight * src._position[0];
        _position[1] += weight * src._position[1];
        _position[2] += weight * src._position[2];
    }

    void SetPosition(float x, float y, float z) {
        _position[0] = x;
        _position[1] = y;
        _position[2] = z;
    }

    const float* GetPosition() const {
        return _position;
    }

private:
    float _position[3];
};

//---------------- OpenSubdiv ----------------
#include <opensubdiv/far/topologyDescriptor.h>
#include <opensubdiv/far/primvarRefiner.h>
using namespace OpenSubdiv;
class subdivider {
private:
    void reset() {
        // Need to do this otherwise these values end up growing as you do subdivisions on top of each other
        new_vertices.clear();
        new_edges.clear();
        edge_list.clear();
        new_faces.clear();
    }
    
    void add_edge(int& origin, int& endpoint) {
        bool origin_search_for_endpoint = false;
        for (int i = 0; i < new_edges[origin].size(); i++) {
            if (new_edges[origin][i] == endpoint) {
                // std::cout << edges[origin][i] << std::endl;   
                origin_search_for_endpoint = true;
                break;
            }
        }

        bool endpoint_search_for_origin = false;
        if (not origin_search_for_endpoint) {
            for (int i = 0; i < new_edges[endpoint].size(); i++) {
                if (new_edges[endpoint][i] == origin) {
                    // std::cout << edges[endpoint][i] << std::endl;
                    endpoint_search_for_origin = true;
                    break;
                }
            }
        }

        if (not origin_search_for_endpoint && not endpoint_search_for_origin) {
            new_edges[origin].push_back({ endpoint });
        }
    }

    std::vector<std::vector<int>> edges_to_list() {
        for (int i = 0; i < new_edges.size(); i++) {
            for (int j = 0; j < new_edges[i].size(); j++) {
                edge_list.push_back(std::vector<int>({ i,new_edges[i][j] }));
            }
        }
        return edge_list;
    }

public:
    subdivider() {        
        nn_verts = 0;
        nn_edges = 0;
        nn_faces = 0;
        new_vertices.clear();
        new_edges.clear();
        edge_list.clear();
        new_faces.clear();
    }

    int maxlevel = 0; 
    int verbose = false; 

    // outgoing topology 
    int nn_verts;
    int nn_edges;
    int nn_faces;
    std::vector<std::vector<float>> new_vertices;
    std::vector<std::vector<int>> new_edges;
    std::vector<std::vector<int>> edge_list;
    std::vector<std::vector<int>> new_faces;

    // ---------------- Configure ----------------
    void settings(int maxlevel,int verbose){
        if(maxlevel < 0){
            maxlevel = 0;
        }
        this->maxlevel = maxlevel;
        this->verbose = verbose;
    }

    // ---------------- Misc ----------------
    void print_settings(){
        std::cout << maxlevel << std::endl;
        std::cout << verbose << std::endl;
    }

    // ---------------- Return new mesh info ----------------
    std::vector<int> refinement_info() {
        std::vector<int> info;
        info.reserve(3);
        info.push_back(nn_verts);
        info.push_back(nn_edges);
        info.push_back(nn_faces);
        return info;
    }

    // ---------------- Return New Vertices ----------------
    void return_new_vertices(float py_new_vertices[][3]) {
        for (int i = 0; i < nn_verts; i++) {
            py_new_vertices[i][0] = new_vertices[i][0];
            py_new_vertices[i][1] = new_vertices[i][1];
            py_new_vertices[i][2] = new_vertices[i][2];
        }
    }

    // ---------------- Return New Edges ----------------
    void return_new_edges(int py_new_edges[][2]) {
        for (int i = 0; i < nn_edges; i++) {
            py_new_edges[i][0] = edge_list[i][0];
            py_new_edges[i][1] = edge_list[i][1];      
        }
    }
    // ---------------- Return New Faces ----------------
    // void return_new_faces(int **py_new_faces) {
    void return_new_faces(int py_new_faces[][4]) {    
        for (int i = 0; i < nn_faces; i++) {
            py_new_faces[i][0] = new_faces[i][0];
            py_new_faces[i][1] = new_faces[i][1];
            py_new_faces[i][2] = new_faces[i][2];
            py_new_faces[i][3] = new_faces[i][3];
        }

        // std::vector<int*> new_faces_vector;
        
        // for(int i=0;i<new_faces.size();i++){
        //     new_faces_vector.push_back(new_faces[i].data());
        // };

        // py_new_faces = new_faces_vector.data();

        // if(1){        
        //     for(int i=0;i<new_faces.size();i++){
        //         for(int j=0;j<new_faces[i].size();j++){
        //             std::cout << py_new_faces[i][j] << " ";
        //         }
        //         std::cout << std::endl;
        //     };
        // };
    }

    // ---------------- Only Create edges from faces ----------------
    void edges_only(int n_verts, int n_faces, int* faceVerts, int* vertsPerFace){
        // ---- New Edges and Faces ----
        int origin = 0;
        int endpoint = 0;
        // Each vert MAY be connected to another 
        new_edges.resize(n_verts);

        int arr_pos = 0;
        for(int i=0;i<n_faces;i++){            
            for(int j=0;j<vertsPerFace[i];j++){
                origin = faceVerts[arr_pos+j];
                endpoint = faceVerts[arr_pos+(j+1)%vertsPerFace[i]];
                // if(verbose){
                //     printf("%d->%d ", origin, endpoint);
                // }
                add_edge(origin, endpoint);
            }
            arr_pos = arr_pos + vertsPerFace[i];
            // if(verbose){std::cout << std::endl;}
        }

        edge_list = edges_to_list();
        nn_edges = edge_list.size();
    }

    // ---------------- Refine Topology ----------------
    void refine_topology(int n_verts, int n_faces, float vertices[][3], int* faceVerts, int* vertsPerFace) {
        reset();

        if(verbose){                
            std::cout << "maxlevel " << maxlevel << std::endl;
        }

        if(maxlevel == 0){
            edges_only(n_verts, n_faces, faceVerts, vertsPerFace);
            if(verbose){
                std::cout << "New Vertices " << n_verts << std::endl;
                for(int i=0;i<n_verts;i++){
                    printf("v %f %f %f\n", vertices[i][0], vertices[i][1], vertices[i][2]);
                }
                for(int i = 0; i < edge_list.size(); i++) {
                    printf("e %d %d\n", edge_list[i][0], edge_list[i][1]);
                }
                int arr_pos = 0;
                for(int i=0;i<n_faces;i++){
                    std::cout << "f ";
                    for(int j=0;j<vertsPerFace[i];j++){
                        // Legacy OBJ vert inds start at 1 (see end of this function)
                        std::cout << faceVerts[arr_pos+j]+1 << " ";
                    }
                    arr_pos = arr_pos + vertsPerFace[i];
                    std::cout << std::endl;
                }
                return;
            }
            return;
        }        

        typedef Far::TopologyDescriptor Descriptor;
        Descriptor desc;

        desc.numVertices = n_verts;
        desc.numFaces = n_faces;
        desc.vertIndicesPerFace = faceVerts;
        desc.numVertsPerFace = vertsPerFace;

        // -------- Configure Refiner --------
        Sdc::SchemeType type = OpenSubdiv::Sdc::SCHEME_CATMARK;
        Sdc::Options options;
        options.SetVtxBoundaryInterpolation(Sdc::Options::VTX_BOUNDARY_EDGE_ONLY);

        // Instantiate a Far::TopologyRefiner from the descriptor (and refinement options) 
        Far::TopologyRefiner* refiner = Far::TopologyRefinerFactory<Descriptor>::Create(desc, Far::TopologyRefinerFactory<Descriptor>::Options(type, options));

        // Uniformly refine the topology up to "maxlevel" 
        refiner->RefineUniform(Far::TopologyRefiner::UniformOptions(maxlevel));

        // -------- Vertices --------
        std::vector<Vertex> vbuffer(refiner->GetNumVerticesTotal());
        Vertex* verts_course = &vbuffer[0];        

        for (int i = 0; i < desc.numVertices; i++) {
            verts_course[i].SetPosition(vertices[i][0], vertices[i][1], vertices[i][2]);
        }

        

        // -------- Interpolate vertex primvar data --------
        Far::PrimvarRefiner primvarRefiner(*refiner);
        Vertex* src = verts_course;

        for (int level = 1; level <= maxlevel; ++level) {
            Vertex* dst = src + refiner->GetLevel(level - 1).GetNumVertices();
            primvarRefiner.Interpolate(level, src, dst);
            src = dst;
        }

        // -------- Set Results --------
        // ---- New Vertices ----
        // This renames refiner->GetLevel(maxlevel) basically (to refLastLevel)
        Far::TopologyLevel const& refLastLevel = refiner->GetLevel(maxlevel); // refLastLevel = address of refiner->GetLevel(maxlevel)
        // int nn_verts = refLastLevel.GetNumVertices();
        nn_verts = refLastLevel.GetNumVertices();

        int firstOfLastVerts = refiner->GetNumVerticesTotal() - nn_verts;

        new_vertices.reserve(nn_verts);
        for (int i = 0; i < nn_verts; i++) {
            float const* pos = verts_course[firstOfLastVerts + i].GetPosition();
            // if (verbose) {
            //     printf("v %f %f %f\n", pos[0], pos[1], pos[2]);
            // }
            new_vertices.push_back(std::vector<float>(pos, pos + 3));
        }

        // ---- New Edges and Faces ----
        int origin = 0;
        int endpoint = 0;
        // Each vert MAY be connected to another 
        // new_edges.reserve(nn_verts); // This would be nice but actually complicates things 
        new_edges.resize(nn_verts);

        // int nn_faces = refLastLevel.GetNumFaces();
        nn_faces = refLastLevel.GetNumFaces();
        new_faces.reserve(nn_faces);

        for (int i = 0; i < nn_faces; i++) {
            Far::ConstIndexArray fverts = refLastLevel.GetFaceVertices(i);
            // All refined CatMark faces should be quads.
            // Only true if maxlevel > 0, though. 
            assert(fverts.size() == 4); 
            new_faces.push_back(std::vector<int>({ fverts[0],fverts[1],fverts[2],fverts[3] }));

            for (int j = 0; j < fverts.size(); j++) {
                origin = fverts[j];
                endpoint = fverts[(j + 1) % fverts.size()];
                add_edge(origin, endpoint);
            }
        }

        edge_list = edges_to_list();
        nn_edges = edge_list.size();

        if (verbose) {
            std::cout << "New Vertices " << nn_verts << std::endl;

            // This outputs "legacy" obj format (at least, as Blender calls it)
            // I don't know what the modern format is supposed to look like, 
            // but with this version, vertex indices start at 1, NOT 0. 
            // NOTE also that Blender applies the Z-up, Y-forward convention,
            // so remember to set that when importing the obj. 
            // Otherwise, it comes out rotated.             
            for (int i = 0; i < nn_verts; i++) {
                printf("v %f %f %f\n", new_vertices[i][0],new_vertices[i][1],new_vertices[i][2]);
            }
            // for (int i = 0; i < new_edges.size(); i++) {
            //     std::cout << i << ": ";
            //     for (int j = 0; j < new_edges[i].size(); j++) {
            //         std::cout << new_edges[i][j] << " ";
            //     }
            //     std::cout << std::endl;
            // }
            for (int i = 0; i < edge_list.size(); i++) {
                printf("e %d %d\n", edge_list[i][0], edge_list[i][1]);
            }
            for (int i = 0; i < nn_faces; i++) {
                printf("f %d %d %d %d\n", new_faces[i][0]+1, new_faces[i][1]+1, new_faces[i][2]+1, new_faces[i][3]+1);
            }
        }
    }
};

int main(int argc, char** argv) {
    // Example usage: ./ctypes_OpenSubdiv -l 3 -v
    // Defaults 
    int subdivision_level = 0; 
    int verbose = false; 

    for(int i = 0; i < argc; i++){        
        std::map<std::string,int> arg_map; 
        arg_map.insert(std::pair<std::string,int>("-l",1));
        arg_map.insert(std::pair<std::string,int>("-v",2));

        switch(arg_map[argv[i]]) {
            case 1:                                          
                // subdivision_level = std::stoi(argv[i+1]); // Windows doesn't like stoi *shrugs*
                subdivision_level = std::atoi(argv[i+1]);
                i++;
                break;
            case 2: 
                verbose = true; 
                break;
        }
    }

    // Cube geometry from catmark_cube.h
    // static float g_verts[8][3] = { { -0.5f, -0.5f,  0.5f },
    //                             {  0.5f, -0.5f,  0.5f },
    //                             { -0.5f,  0.5f,  0.5f },
    //                             {  0.5f,  0.5f,  0.5f },
    //                             { -0.5f,  0.5f, -0.5f },
    //                             {  0.5f,  0.5f, -0.5f },
    //                             { -0.5f, -0.5f, -0.5f },
    //                             {  0.5f, -0.5f, -0.5f } };

    // static int g_nverts = 8, g_nfaces = 6;

    // static int g_vertsperface[6] = { 4, 4, 4, 4, 4, 4 };

    // static int g_vertIndices[24] = { 0, 1, 3, 2,
    //                                 2, 3, 5, 4,
    //                                 4, 5, 7, 6,
    //                                 6, 7, 1, 0,
    //                                 1, 7, 5, 3,
    //                                 6, 0, 2, 4 };

    // Triangle array (for testing non-quads)
    // static float g_verts[10][3]  = { {-0.75, -1.7320507764816284, 0.0},
    //                                 {-2.25, -0.8660253882408142, 0.0},
    //                                 {-2.25, 0.8660253882408142, 0.0},
    //                                 {0.75, 0.8660253882408142, 0.0},
    //                                 {-0.75, -1.1102230246251565e-16, 0.0},
    //                                 {-0.75, 1.7320507764816284, 0.0},
    //                                 {2.25, -1.7320507764816284, 0.0},
    //                                 {0.75, -0.8660253882408142, 0.0},
    //                                 {2.25, 1.7320507764816284, 0.0},
    //                                 {2.25, 1.1102230246251565e-16, 0.0} };

    // static int g_nverts = 10, g_nfaces = 9;

    // static int g_vertsperface[9] = {3, 3, 3, 3, 3, 3, 3, 3, 3};

    // static int g_vertIndices[27] = { 4, 0, 1, 
    //                                 1, 2, 4, 
    //                                 5, 4, 2, 
    //                                 0, 4, 7, 
    //                                 3, 7, 4, 
    //                                 4, 5, 3, 
    //                                 9, 6, 7, 
    //                                 7, 3, 9, 
    //                                 8, 9, 3 };

    // Ngons 
    // static float g_verts[33][3] = { {8.00f, 0.00f, 0.00f},
    // {5.66f, 5.66f, 0.00f},
    // {0.00f, 8.00f, 0.00f},
    // {-5.66f, 5.66f, 0.00f},
    // {-8.00f, 0.00f, 0.00f},
    // {-5.66f, -5.66f, 0.00f},
    // {-0.00f, -8.00f, 0.00f},
    // {5.66f, -5.66f, 0.00f},
    // {7.00f, 0.00f, 0.00f},
    // {4.36f, 5.47f, 0.00f},
    // {-1.56f, 6.82f, 0.00f},
    // {-6.31f, 3.04f, 0.00f},
    // {-6.31f, -3.04f, 0.00f},
    // {-1.56f, -6.82f, 0.00f},
    // {4.36f, -5.47f, 0.00f},
    // {6.00f, 0.00f, 0.00f},
    // {3.00f, 5.20f, 0.00f},
    // {-3.00f, 5.20f, 0.00f},
    // {-6.00f, 0.00f, 0.00f},
    // {-3.00f, -5.20f, 0.00f},
    // {3.00f, -5.20f, 0.00f},
    // {5.00f, 0.00f, 0.00f},
    // {1.55f, 4.76f, 0.00f},
    // {-4.05f, 2.94f, 0.00f},
    // {-4.05f, -2.94f, 0.00f},
    // {1.55f, -4.76f, 0.00f},
    // {4.00f, 0.00f, 0.00f},
    // {0.00f, 4.00f, 0.00f},
    // {-4.00f, 0.00f, 0.00f},
    // {-0.00f, -4.00f, 0.00f},
    // {3.00f, 0.00f, 0.00f},
    // {-1.50f, 2.60f, 0.00f},
    // {-1.50f, -2.60f, 0.00f} };

    // static int g_nverts = 33, g_nfaces = 6;

    // static int g_vertsperface[6] = {8, 7, 6, 5, 4, 3};

    // static int g_vertIndices[33] = {
    // 0, 1, 2, 3, 4, 5, 6, 7,
    // 8, 9, 10, 11, 12, 13, 14,
    // 15, 16, 17, 18, 19, 20,
    // 21, 22, 23, 24, 25,
    // 26, 27, 28, 29,
    // 30, 31, 32 };

    // ngons_2
    static float g_verts[7][3] = { {-1.00f, -1.00f, 0.00f},
    {1.00f, -1.00f, 0.00f},
    {-1.00f, 1.00f, 0.00f},
    {1.00f, 1.00f, 0.00f},
    {1.73f, 0.16f, 1.75f},
    {-1.73f, 1.00f, 1.75f},
    {0.00f, 1.00f, 2.51f} };

    static int g_nverts = 7, g_nfaces = 3;

    static int g_vertsperface[3] = {5, 4, 3};

    static int g_vertIndices[12] = {
    2, 3, 4, 6, 5,
    0, 1, 3, 2,
    4, 3, 1 };

    subdivider subdivider_instance;    
    subdivider_instance.settings(subdivision_level,verbose);
    subdivider_instance.print_settings();

    subdivider_instance.refine_topology(g_nverts, g_nfaces, g_verts, g_vertIndices, g_vertsperface);

    return 0;
}

extern "C"
{
    subdivider subdivider_new;    

    DLLEXPORT void subdivider_settings(int maxlevel, int verbose) {subdivider_new.settings(maxlevel,verbose);}

    // DLLEXPORT void subdivider_refine_topology(int maxlevel, int verbose, int n_verts, int n_faces, float vertices[][3], int* faceVerts, int* vertsPerFace) { subdivider_new.refine_topology(maxlevel, verbose, n_verts, n_faces, vertices, faceVerts, vertsPerFace); }

    DLLEXPORT void subdivider_refine_topology(int n_verts, int n_faces, float vertices[][3], int* faceVerts, int* vertsPerFace) { subdivider_new.refine_topology(n_verts, n_faces, vertices, faceVerts, vertsPerFace); }    

    DLLEXPORT int nn_verts() { return subdivider_new.nn_verts; }
    DLLEXPORT int nn_edges() { return subdivider_new.nn_edges; }
    DLLEXPORT int nn_faces() { return subdivider_new.nn_faces; }

    DLLEXPORT void new_vertices(float py_new_vertices[][3]) { subdivider_new.return_new_vertices(py_new_vertices); }
    DLLEXPORT void new_edges(int py_new_edges[][2]) { subdivider_new.return_new_edges(py_new_edges); }
    DLLEXPORT void new_faces(int py_new_faces[][4]) { subdivider_new.return_new_faces(py_new_faces); }

    // This technically works but doesn't return anything back to python,
    // even though I would expect py_new_faces to be passed by reference. 
    // Confused. 
    // Separate issue that each row of the matrix can have different length
    // (each face may have an arbitrary number of verts),
    // which seems really hard to implement on the ctypes side, if possible at all. 
    // DLLEXPORT void new_faces(int **py_new_faces) { subdivider_new.return_new_faces(py_new_faces); }    

}
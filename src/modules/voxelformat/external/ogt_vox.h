/*
    opengametools vox file reader/writer/merger - v0.997 - MIT license - Justin Paver, Oct 2019

    This is a single-header-file library that provides easy-to-use
    support for reading MagicaVoxel .vox files into structures that
    are easy to dereference and extract information from. It also
    supports writing back out to .vox file from those structures.

    Please see the MIT license information at the end of this file.

    Also, please consider sharing any improvements you make.

    For more information and more tools, visit:
      https://github.com/jpaver/opengametools

    HOW TO COMPILE THIS LIBRARY

    1.  To compile this library, do this in *one* C or C++ file:
        #define OGT_VOX_IMPLEMENTATION
        #include "ogt_vox.h"

    2. From any other module, it is sufficient to just #include this as usual:
        #include "ogt_vox.h"

    HOW TO READ A VOX SCENE (See demo_vox.cpp)

    1. load a .vox file off disk into a memory buffer.

    2. construct a scene from the memory buffer:
       ogt_vox_scene* scene = ogt_vox_read_scene(buffer, buffer_size);

    3. use the scene members to extract the information you need. eg.
       printf("# of layers: %u\n", scene->num_layers );

    4. destroy the scene:
       ogt_vox_destroy_scene(scene);

    HOW TO MERGE MULTIPLE VOX SCENES (See merge_vox.cpp)

    1. construct multiple scenes from files you want to merge.

        // read buffer1/buffer_size1 from "test1.vox"
        // read buffer2/buffer_size2 from "test2.vox"
        // read buffer3/buffer_size3 from "test3.vox"
        ogt_vox_scene* scene1 = ogt_vox_read_scene(buffer1, buffer_size1);
        ogt_vox_scene* scene2 = ogt_vox_read_scene(buffer2, buffer_size2);
        ogt_vox_scene* scene3 = ogt_vox_read_scene(buffer3, buffer_size3);

    2. construct a merged scene

        const ogt_vox_scene* scenes[] = {scene1, scene2, scene3};
        ogt_vox_scene* merged_scene = ogt_vox_merge_scenes(scenes, 3, NULL, 0);

    3. save out the merged scene

        uint8_t* out_buffer = ogt_vox_write_scene(merged_scene, &out_buffer_size);
        // save out_buffer to disk as a .vox file (it has length out_buffer_size)

    4. destroy the merged scene:

        ogt_vox_destroy_scene(merged_scene);

    EXPLANATION OF SCENE ELEMENTS:

    A ogt_vox_scene comprises primarily a set of instances, models, layers and a palette.

    A ogt_vox_palette contains a set of 256 colors that is used for the scene.
    Each color is represented by a 4-tuple called an ogt_vox_rgba which contains red,
    green, blue and alpha values for the color.

    A ogt_vox_model is a 3-dimensional grid of voxels, where each of those voxels
    is represented by an 8-bit color index. Voxels are arranged in order of increasing
    X then increasing Y then increasing Z.

    Given the x,y,z values for a voxel within the model dimensions, the voxels index
    in the grid can be obtained as follows:

        voxel_index = x + (y * model->size_x) + (z * model->size_x * model->size_y)

    The index is only valid if the coordinate x,y,z satisfy the following conditions:
            0 <= x < model->size_x -AND-
            0 <= y < model->size_y -AND-
            0 <= z < model->size_z

    A voxels color index can be obtained as follows:

        uint8_t color_index = model->voxel_data[voxel_index];

    If color_index == 0, the voxel is not solid and can be skipped,
    If color_index != 0, the voxel is solid and can be used to lookup the color in the palette:

        ogt_vox_rgba color = scene->palette.color[ color_index]

    A ogt_vox_instance is an individual placement of a voxel model within the scene. Each
    instance has a transform that determines its position and orientation within the scene,
    but it also has an index that specifies which model the instance uses for its shape. It
    is expected that there is a many-to-one mapping of instances to models.

    An ogt_vox_layer is used to conceptually group instances. Each instance indexes the
    layer that it belongs to, but the layer itself has its own name and hidden/shown state.

    EXPLANATION OF MERGED SCENES:

    A merged scene contains all the models and all the scene instances from
    each of the scenes that were passed into it.

    The merged scene will have a combined palette of all the source scene
    palettes by trying to match existing colors exactly, and falling back
    to an RGB-distance matched color when all 256 colors in the merged
    scene palette has been allocated.

    You can explicitly control up to 255 merge palette colors by providing
    those colors to ogt_vox_merge_scenes in the required_colors parameters eg.

        const ogt_vox_palette palette;  // load this via .vox or procedurally or whatever
        const ogt_vox_scene* scenes[] = {scene1, scene2, scene3};
        // palette.color[0] is always the empty color which is why we pass 255 colors starting from index 1 only:
        ogt_vox_scene* merged_scene = ogt_vox_merge_scenes(scenes, 3, &palette.color[1], 255);

    EXPLANATION OF MODEL PIVOTS

    If a voxel model grid has dimension size.xyz in terms of number of voxels, the centre pivot
    for that model is located at floor( size.xyz / 2).

    eg. for a 3x4x1 voxel model, the pivot would be at (1,2,0), or the X in the below ascii art.

           4 +-----+-----+-----+
             |  .  |  .  |  .  |
           3 +-----+-----+-----+
             |  .  |  .  |  .  |
           2 +-----X-----+-----+
             |  .  |  .  |  .  |
           1 +-----+-----+-----+
             |  .  |  .  |  .  |
           0 +-----+-----+-----+
             0     1     2     3

     An example model in this grid form factor might look like this:

           4 +-----+-----+-----+
             |  .  |  .  |  .  |
           3 +-----+-----+-----+
                   |  .  |
           2       X-----+
                   |  .  |
           1       +-----+
                   |  .  |
           0       +-----+
             0     1     2     3

     If you were to generate a mesh from this, clearly each vertex and each face would be on an integer
     coordinate eg. 1, 2, 3 etc. while the centre of each grid location (ie. the . in the above diagram)
     will be on a coordinate that is halfway between integer coordinates. eg. 1.5, 2.5, 3.5 etc.

     To ensure your mesh is properly centered such that instance transforms are correctly applied, you
     want the pivot to be treated as if it were (0,0,0) in model space. To achieve this, simply
     subtract the pivot from any geometry that is generated (eg. vertices in a mesh).

     For the 3x4x1 voxel model above, doing this would look like this:

           2 +-----+-----+-----+
             |  .  |  .  |  .  |
           1 +-----+-----+-----+
                   |  .  |
           0       X-----+
                   |  .  |
          -1       +-----+
                   |  .  |
          -2       +-----+
            -1     0     1     2

    To replace asserts within this library with your own implementation, simply #define ogt_assert before defining your implementation
    eg.
        #include "my_assert.h"
        #define ogt_assert(condition, message_str)    my_assert(condition, message_str)

        #define OGT_VOX_IMPLEMENTATION
        #include "path/to/ogt_vox.h"

    ogt_vox is little-endian by default, but it does support big-endian if OGT_VOX_BIGENDIAN_SWAP32(x) is #defined
    to a function that can swap byte order within a uint32_t word before the implementation. eg.

        #define OGT_VOX_BIGENDIAN_SWAP32(x)  __builtin_swap32(x)  // linux/gcc
        #define OGT_VOX_IMPLEMENTATION
        #include "path/to/ogt_vox.h"
*/
#ifndef OGT_VOX_H__
#define OGT_VOX_H__

#if _MSC_VER == 1400
    // VS2005 doesn't have inttypes or stdint so we just define what we need here.
    typedef unsigned char uint8_t;
    typedef signed int    int32_t;
    typedef unsigned int  uint32_t;
    #ifndef UINT32_MAX
        #define UINT32_MAX	((uint32_t)0xFFFFFFFF)
    #endif
    #ifndef INT32_MAX
        #define INT32_MAX	((int32_t)0x7FFFFFFF)
    #endif
    #ifndef UINT8_MAX
        #define UINT8_MAX	((uint8_t)0xFF)
    #endif
#elif defined(_MSC_VER)
    // general VS*
    #include <inttypes.h>
#elif SDL_PLATFORM_APPLE
    // general Apple compiler
    #include <stdint.h>
    #include <limits.h>
    #include <stdlib.h> // for size_t
#elif defined(__GNUC__)
    // any GCC*
    #include <inttypes.h>
    #include <stdlib.h> // for size_t
#else
    #error some fixup needed for this platform?
#endif

#ifdef OGT_VOX_BIGENDIAN_SWAP32
    // host is big-endian, so we byte-swap
    #define _vox_htole32(x)  OGT_VOX_BIGENDIAN_SWAP32((x))
    #define _vox_le32toh(x)  OGT_VOX_BIGENDIAN_SWAP32((x))
#else
    // host is little-endian (default)
    #define _vox_htole32(x)  (x)
    #define _vox_le32toh(x)  (x)
#endif

    // denotes an invalid group index. Usually this is only applicable to the scene's root group's parent.
    static const uint32_t k_invalid_group_index = UINT32_MAX;

    // color
    typedef struct ogt_vox_rgba
    {
        uint8_t r,g,b,a;            // red, green, blue and alpha components of a color.
    } ogt_vox_rgba;

    // column-major 4x4 matrix
    typedef struct ogt_vox_transform
    {
        float m00, m01, m02, m03;   // column 0 of 4x4 matrix, 1st three elements = x axis vector, last element always 0.0
        float m10, m11, m12, m13;   // column 1 of 4x4 matrix, 1st three elements = y axis vector, last element always 0.0
        float m20, m21, m22, m23;   // column 2 of 4x4 matrix, 1st three elements = z axis vector, last element always 0.0
        float m30, m31, m32, m33;   // column 3 of 4x4 matrix. 1st three elements = translation vector, last element always 1.0
    } ogt_vox_transform;

    ogt_vox_transform ogt_vox_transform_get_identity();
    ogt_vox_transform ogt_vox_transform_multiply(const ogt_vox_transform & a, const ogt_vox_transform & b);

    // a palette of colors
    typedef struct ogt_vox_palette
    {
        ogt_vox_rgba color[256];      // palette of colors. use the voxel indices to lookup color from the palette.
    } ogt_vox_palette;

    // Extended Material Chunk MATL types
    enum ogt_matl_type
    {
        ogt_matl_type_diffuse = 0, // diffuse is default
        ogt_matl_type_metal   = 1,
        ogt_matl_type_glass   = 2,
        ogt_matl_type_emit    = 3,
        ogt_matl_type_blend   = 4,
        ogt_matl_type_media   = 5,
    };

    enum ogt_cam_mode
    {
        ogt_cam_mode_perspective  = 0,
        ogt_cam_mode_free         = 1,
        ogt_cam_mode_pano         = 2,
        ogt_cam_mode_orthographic = 3,
        ogt_cam_mode_isometric    = 4,
        ogt_cam_mode_unknown      = 5
    };

    // Content Flags for ogt_vox_matl values for a given material
    static const uint32_t k_ogt_vox_matl_have_metal  = 1 << 0;
    static const uint32_t k_ogt_vox_matl_have_rough  = 1 << 1;
    static const uint32_t k_ogt_vox_matl_have_spec   = 1 << 2;
    static const uint32_t k_ogt_vox_matl_have_ior    = 1 << 3;
    static const uint32_t k_ogt_vox_matl_have_att    = 1 << 4;
    static const uint32_t k_ogt_vox_matl_have_flux   = 1 << 5;
    static const uint32_t k_ogt_vox_matl_have_emit   = 1 << 6;
    static const uint32_t k_ogt_vox_matl_have_ldr    = 1 << 7;
    static const uint32_t k_ogt_vox_matl_have_trans  = 1 << 8;
    static const uint32_t k_ogt_vox_matl_have_alpha  = 1 << 9;
    static const uint32_t k_ogt_vox_matl_have_d      = 1 << 10;
    static const uint32_t k_ogt_vox_matl_have_sp     = 1 << 11;
    static const uint32_t k_ogt_vox_matl_have_g      = 1 << 12;
    static const uint32_t k_ogt_vox_matl_have_media  = 1 << 13;

    // media type for blend, glass and cloud materials
    enum ogt_media_type {
        ogt_media_type_absorb,  // Absorb media
        ogt_media_type_scatter, // Scatter media
        ogt_media_type_emit,    // Emissive media
        ogt_media_type_sss,     // Subsurface scattering media
    };

    // Extended Material Chunk MATL information
    typedef struct ogt_vox_matl
    {
        uint32_t       content_flags; // set of k_ogt_vox_matl_* OR together to denote contents available
        ogt_media_type media_type;    // media type for blend, glass and cloud materials
        ogt_matl_type  type;
        float          metal;
        float          rough;         // roughness
        float          spec;          // specular
        float          ior;           // index of refraction
        float          att;           // attenuation
        float          flux;          // radiant flux (power)
        float          emit;          // emissive
        float          ldr;           // low dynamic range
        float          trans;         // transparency
        float          alpha;
        float          d;             // density
        float          sp;
        float          g;
        float          media;
    } ogt_vox_matl;

    // Extended Material Chunk MATL array of materials
    typedef struct ogt_vox_matl_array
    {
        ogt_vox_matl matl[256];      // extended material information from Material Chunk MATL
    } ogt_vox_matl_array;

    typedef struct ogt_vox_cam
    {
        uint32_t     camera_id;
        ogt_cam_mode mode;
        float        focus[3];    // the target position
        float        angle[3];    // rotation in degree
        float        radius;
        float        frustum;
        int          fov;         // angle in degree
    } ogt_vox_cam;

    // a 3-dimensional model of voxels
    typedef struct ogt_vox_model
    {
        uint32_t       size_x;        // number of voxels in the local x dimension
        uint32_t       size_y;        // number of voxels in the local y dimension
        uint32_t       size_z;        // number of voxels in the local z dimension
        uint32_t       voxel_hash;    // hash of the content of the grid.
        const uint8_t* voxel_data;    // grid of voxel data comprising color indices in x -> y -> z order. a color index of 0 means empty, all other indices mean solid and can be used to index the scene's palette to obtain the color for the voxel.
    } ogt_vox_model;

    // a keyframe for animation of a transform
    typedef struct ogt_vox_keyframe_transform {
        uint32_t          frame_index;
        ogt_vox_transform transform;
    } ogt_vox_keyframe_transform;

    // a keyframe for animation of a model
    typedef struct ogt_vox_keyframe_model {
        uint32_t frame_index;
        uint32_t model_index;
    } ogt_vox_keyframe_model;

    // an animated transform
    typedef struct ogt_vox_anim_transform {
        const ogt_vox_keyframe_transform* keyframes;
        uint32_t                          num_keyframes;
        bool                              loop;
    } ogt_vox_anim_transform;

    // an animated model
    typedef struct ogt_vox_anim_model {
        const ogt_vox_keyframe_model* keyframes;
        uint32_t                      num_keyframes;
        bool                          loop;
    } ogt_vox_anim_model;

    // an instance of a model within the scene
    typedef struct ogt_vox_instance
    {
        const char*            name;                   // name of the instance if there is one, will be NULL otherwise.
        ogt_vox_transform      transform;              // orientation and position of this instance on first frame of the scene. This is relative to its group local transform if group_index is not 0
        uint32_t               model_index;            // index of the model used by this instance on the first frame of the scene. used to lookup the model in the scene's models[] array.
        uint32_t               layer_index;            // index of the layer used by this instance. used to lookup the layer in the scene's layers[] array.
        uint32_t               group_index;            // this will be the index of the group in the scene's groups[] array. If group is zero it will be the scene root group and the instance transform will be a world-space transform, otherwise the transform is relative to the group.
        bool                   hidden;                 // whether this instance is individually hidden or not. Note: the instance can also be hidden when its layer is hidden, or if it belongs to a group that is hidden.
        ogt_vox_anim_transform transform_anim;         // animation for the transform
        ogt_vox_anim_model     model_anim;             // animation for the model_index
    } ogt_vox_instance;

    // describes a layer within the scene
    typedef struct ogt_vox_layer
    {
        const char*  name;               // name of this layer if there is one, will be NULL otherwise.
        ogt_vox_rgba color;              // color of the layer.
        bool         hidden;             // whether this layer is hidden or not.
    } ogt_vox_layer;

    // describes a group within the scene
    typedef struct ogt_vox_group
    {
        const char*            name;                    // name of the group if there is one, will be NULL otherwise
        ogt_vox_transform      transform;               // transform of this group relative to its parent group (if any), otherwise this will be relative to world-space.
        uint32_t               parent_group_index;      // if this group is parented to another group, this will be the index of its parent in the scene's groups[] array, otherwise this group will be the scene root group and this value will be k_invalid_group_index
        uint32_t               layer_index;             // which layer this group belongs to. used to lookup the layer in the scene's layers[] array.
        bool                   hidden;                  // whether this group is hidden or not.
        ogt_vox_anim_transform transform_anim;          // animated transform data
    } ogt_vox_group;

    // the scene parsed from a .vox file.
    typedef struct ogt_vox_scene
    {
        uint32_t                num_models;     // number of models within the scene.
        uint32_t                num_instances;  // number of instances in the scene (on anim frame 0)
        uint32_t                num_layers;     // number of layers in the scene
        uint32_t                num_groups;     // number of groups in the scene
        const ogt_vox_model**   models;         // array of models. size is num_models
        const ogt_vox_instance* instances;      // array of instances. size is num_instances
        const ogt_vox_layer*    layers;         // array of layers. size is num_layers
        const ogt_vox_group*    groups;         // array of groups. size is num_groups
        ogt_vox_palette         palette;        // the palette for this scene
        ogt_vox_matl_array      materials;      // the extended materials for this scene
        uint32_t                num_cameras;    // number of cameras for this scene
        const ogt_vox_cam*      cameras;        // the cameras for this scene
    } ogt_vox_scene;

    // allocate memory function interface. pass in size, and get a pointer to memory with at least that size available.
    typedef void* (*ogt_vox_alloc_func)(size_t size);

    // free memory function interface. pass in a pointer previously allocated and it will be released back to the system managing memory.
    typedef void  (*ogt_vox_free_func)(void* ptr);

    // override the default scene memory allocator if you need to control memory precisely.
    void  ogt_vox_set_memory_allocator(ogt_vox_alloc_func alloc_func, ogt_vox_free_func free_func);
    void* ogt_vox_malloc(size_t size);
    void  ogt_vox_free(void* mem);

    // progress feedback function with option to cancel a ogt_vox_write_scene operation. Percentage complete is approximately given by: 100.0f * progress.
    typedef bool  (*ogt_vox_progress_callback_func)(float progress, void* user_data);

    // set the progress callback function and user data to pass to it
    void  ogt_vox_set_progress_callback_func(ogt_vox_progress_callback_func progress_callback_func, void* user_data);


    // flags for ogt_vox_read_scene_with_flags
    static const uint32_t k_read_scene_flags_groups                      = 1 << 0; // if not specified, all instance transforms will be flattened into world space. If specified, will read group information and keep all transforms as local transform relative to the group they are in.
    static const uint32_t k_read_scene_flags_keyframes                   = 1 << 1; // if specified, all instances and groups will contain keyframe data.
    static const uint32_t k_read_scene_flags_keep_empty_models_instances = 1 << 2; // if specified, all empty models and instances referencing those will be kept rather than culled.
    static const uint32_t k_read_scene_flags_keep_duplicate_models       = 1 << 3; // if specified, we do not de-duplicate models.

    // creates a scene from a vox file within a memory buffer of a given size.
    // you can destroy the input buffer once you have the scene as this function will allocate separate memory for the scene objecvt.
    const ogt_vox_scene* ogt_vox_read_scene(const uint8_t* buffer, uint32_t buffer_size);

    // just like ogt_vox_read_scene, but you can additionally pass a union of k_read_scene_flags
    const ogt_vox_scene* ogt_vox_read_scene_with_flags(const uint8_t* buffer, uint32_t buffer_size, uint32_t read_flags);

    // destroys a scene object to release its memory.
    void ogt_vox_destroy_scene(const ogt_vox_scene* scene);

    // writes the scene to a new buffer and returns the buffer size. free the buffer with ogt_vox_free
    uint8_t* ogt_vox_write_scene(const ogt_vox_scene* scene, uint32_t* buffer_size);

    // merges the specified scenes together to create a bigger scene. Merged scene can be destroyed using ogt_vox_destroy_scene
    // If you require specific colors in the merged scene palette, provide up to and including 255 of them via required_colors/required_color_count.
    ogt_vox_scene* ogt_vox_merge_scenes(const ogt_vox_scene** scenes, uint32_t scene_count, const ogt_vox_rgba* required_colors, const uint32_t required_color_count);

    // sample the model index for a given instance at the given frame
    uint32_t          ogt_vox_sample_instance_model(const ogt_vox_instance* instance, uint32_t frame_index);

    // samples the transform for an instance at a given frame.
    //   ogt_vox_sample_instance_transform_global returns the transform in world space (aka global)
    //   ogt_vox_sample_instance_transform_local returns the transform relative to its parent group
    ogt_vox_transform ogt_vox_sample_instance_transform_global(const ogt_vox_instance* instance, uint32_t frame_index, const ogt_vox_scene* scene);
    ogt_vox_transform ogt_vox_sample_instance_transform_local(const ogt_vox_instance* instance, uint32_t frame_index);

    // sample the transform for a group at a given frame
    //  ogt_vox_sample_group_transform_global returns the transform in world space (aka global)
    //  ogt_vox_sample_group_transform_local returns the transform relative to its parent group
    ogt_vox_transform ogt_vox_sample_group_transform_global(const ogt_vox_group* group, uint32_t frame_index, const ogt_vox_scene* scene);
    ogt_vox_transform ogt_vox_sample_group_transform_local(const ogt_vox_group* group, uint32_t frame_index);

#endif // OGT_VOX_H__

//-----------------------------------------------------------------------------------------------------------------
//
// If you're only interested in using this library, everything you need is above this point.
// If you're interested in how this library works, everything you need is below this point.
//
//-----------------------------------------------------------------------------------------------------------------
#ifdef OGT_VOX_IMPLEMENTATION
    // callers can override asserts in ogt_vox by defining their own macro before the implementation
#ifndef ogt_assert
    #include <assert.h>
    #define ogt_assert(x, msg_str)      do { assert((x) && (msg_str)); } while(0)
#endif
    #include <stdlib.h>
    #include <string.h>
    #include <stdio.h>

    // MAKE_VOX_CHUNK_ID: used to construct a literal to describe a chunk in a .vox file.
    #define MAKE_VOX_CHUNK_ID(c0,c1,c2,c3)     ( (c0<<0) | (c1<<8) | (c2<<16) | (c3<<24) )

    static const uint32_t CHUNK_ID_VOX_ = MAKE_VOX_CHUNK_ID('V','O','X',' ');
    static const uint32_t CHUNK_ID_MAIN = MAKE_VOX_CHUNK_ID('M','A','I','N');
    static const uint32_t CHUNK_ID_SIZE = MAKE_VOX_CHUNK_ID('S','I','Z','E');
    static const uint32_t CHUNK_ID_XYZI = MAKE_VOX_CHUNK_ID('X','Y','Z','I');
    static const uint32_t CHUNK_ID_RGBA = MAKE_VOX_CHUNK_ID('R','G','B','A');
    static const uint32_t CHUNK_ID_nTRN = MAKE_VOX_CHUNK_ID('n','T','R','N');
    static const uint32_t CHUNK_ID_nGRP = MAKE_VOX_CHUNK_ID('n','G','R','P');
    static const uint32_t CHUNK_ID_nSHP = MAKE_VOX_CHUNK_ID('n','S','H','P');
    static const uint32_t CHUNK_ID_IMAP = MAKE_VOX_CHUNK_ID('I','M','A','P');
    static const uint32_t CHUNK_ID_LAYR = MAKE_VOX_CHUNK_ID('L','A','Y','R');
    static const uint32_t CHUNK_ID_MATL = MAKE_VOX_CHUNK_ID('M','A','T','L');
    static const uint32_t CHUNK_ID_MATT = MAKE_VOX_CHUNK_ID('M','A','T','T');
    static const uint32_t CHUNK_ID_rOBJ = MAKE_VOX_CHUNK_ID('r','O','B','J');
    static const uint32_t CHUNK_ID_rCAM = MAKE_VOX_CHUNK_ID('r','C','A','M');

    static const uint32_t NAME_MAX_LEN     = 256;       // max name len = 255 plus 1 for null terminator
    static const uint32_t CHUNK_HEADER_LEN = 12;        // 4 bytes for each of: chunk_id, chunk_size, chunk_child_size

    // Some older .vox files will not store a palette, in which case the following palette will be used!
    static const uint8_t k_default_vox_palette[256 * 4] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcc, 0xff, 0xff, 0xff, 0x99, 0xff, 0xff, 0xff, 0x66, 0xff, 0xff, 0xff, 0x33, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xcc, 0xff, 0xff, 0xff, 0xcc, 0xcc, 0xff,
        0xff, 0xcc, 0x99, 0xff, 0xff, 0xcc, 0x66, 0xff, 0xff, 0xcc, 0x33, 0xff, 0xff, 0xcc, 0x00, 0xff, 0xff, 0x99, 0xff, 0xff, 0xff, 0x99, 0xcc, 0xff, 0xff, 0x99, 0x99, 0xff, 0xff, 0x99, 0x66, 0xff,
        0xff, 0x99, 0x33, 0xff, 0xff, 0x99, 0x00, 0xff, 0xff, 0x66, 0xff, 0xff, 0xff, 0x66, 0xcc, 0xff, 0xff, 0x66, 0x99, 0xff, 0xff, 0x66, 0x66, 0xff, 0xff, 0x66, 0x33, 0xff, 0xff, 0x66, 0x00, 0xff,
        0xff, 0x33, 0xff, 0xff, 0xff, 0x33, 0xcc, 0xff, 0xff, 0x33, 0x99, 0xff, 0xff, 0x33, 0x66, 0xff, 0xff, 0x33, 0x33, 0xff, 0xff, 0x33, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xcc, 0xff,
        0xff, 0x00, 0x99, 0xff, 0xff, 0x00, 0x66, 0xff, 0xff, 0x00, 0x33, 0xff, 0xff, 0x00, 0x00, 0xff, 0xcc, 0xff, 0xff, 0xff, 0xcc, 0xff, 0xcc, 0xff, 0xcc, 0xff, 0x99, 0xff, 0xcc, 0xff, 0x66, 0xff,
        0xcc, 0xff, 0x33, 0xff, 0xcc, 0xff, 0x00, 0xff, 0xcc, 0xcc, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0xff, 0xcc, 0xcc, 0x99, 0xff, 0xcc, 0xcc, 0x66, 0xff, 0xcc, 0xcc, 0x33, 0xff, 0xcc, 0xcc, 0x00, 0xff,
        0xcc, 0x99, 0xff, 0xff, 0xcc, 0x99, 0xcc, 0xff, 0xcc, 0x99, 0x99, 0xff, 0xcc, 0x99, 0x66, 0xff, 0xcc, 0x99, 0x33, 0xff, 0xcc, 0x99, 0x00, 0xff, 0xcc, 0x66, 0xff, 0xff, 0xcc, 0x66, 0xcc, 0xff,
        0xcc, 0x66, 0x99, 0xff, 0xcc, 0x66, 0x66, 0xff, 0xcc, 0x66, 0x33, 0xff, 0xcc, 0x66, 0x00, 0xff, 0xcc, 0x33, 0xff, 0xff, 0xcc, 0x33, 0xcc, 0xff, 0xcc, 0x33, 0x99, 0xff, 0xcc, 0x33, 0x66, 0xff,
        0xcc, 0x33, 0x33, 0xff, 0xcc, 0x33, 0x00, 0xff, 0xcc, 0x00, 0xff, 0xff, 0xcc, 0x00, 0xcc, 0xff, 0xcc, 0x00, 0x99, 0xff, 0xcc, 0x00, 0x66, 0xff, 0xcc, 0x00, 0x33, 0xff, 0xcc, 0x00, 0x00, 0xff,
        0x99, 0xff, 0xff, 0xff, 0x99, 0xff, 0xcc, 0xff, 0x99, 0xff, 0x99, 0xff, 0x99, 0xff, 0x66, 0xff, 0x99, 0xff, 0x33, 0xff, 0x99, 0xff, 0x00, 0xff, 0x99, 0xcc, 0xff, 0xff, 0x99, 0xcc, 0xcc, 0xff,
        0x99, 0xcc, 0x99, 0xff, 0x99, 0xcc, 0x66, 0xff, 0x99, 0xcc, 0x33, 0xff, 0x99, 0xcc, 0x00, 0xff, 0x99, 0x99, 0xff, 0xff, 0x99, 0x99, 0xcc, 0xff, 0x99, 0x99, 0x99, 0xff, 0x99, 0x99, 0x66, 0xff,
        0x99, 0x99, 0x33, 0xff, 0x99, 0x99, 0x00, 0xff, 0x99, 0x66, 0xff, 0xff, 0x99, 0x66, 0xcc, 0xff, 0x99, 0x66, 0x99, 0xff, 0x99, 0x66, 0x66, 0xff, 0x99, 0x66, 0x33, 0xff, 0x99, 0x66, 0x00, 0xff,
        0x99, 0x33, 0xff, 0xff, 0x99, 0x33, 0xcc, 0xff, 0x99, 0x33, 0x99, 0xff, 0x99, 0x33, 0x66, 0xff, 0x99, 0x33, 0x33, 0xff, 0x99, 0x33, 0x00, 0xff, 0x99, 0x00, 0xff, 0xff, 0x99, 0x00, 0xcc, 0xff,
        0x99, 0x00, 0x99, 0xff, 0x99, 0x00, 0x66, 0xff, 0x99, 0x00, 0x33, 0xff, 0x99, 0x00, 0x00, 0xff, 0x66, 0xff, 0xff, 0xff, 0x66, 0xff, 0xcc, 0xff, 0x66, 0xff, 0x99, 0xff, 0x66, 0xff, 0x66, 0xff,
        0x66, 0xff, 0x33, 0xff, 0x66, 0xff, 0x00, 0xff, 0x66, 0xcc, 0xff, 0xff, 0x66, 0xcc, 0xcc, 0xff, 0x66, 0xcc, 0x99, 0xff, 0x66, 0xcc, 0x66, 0xff, 0x66, 0xcc, 0x33, 0xff, 0x66, 0xcc, 0x00, 0xff,
        0x66, 0x99, 0xff, 0xff, 0x66, 0x99, 0xcc, 0xff, 0x66, 0x99, 0x99, 0xff, 0x66, 0x99, 0x66, 0xff, 0x66, 0x99, 0x33, 0xff, 0x66, 0x99, 0x00, 0xff, 0x66, 0x66, 0xff, 0xff, 0x66, 0x66, 0xcc, 0xff,
        0x66, 0x66, 0x99, 0xff, 0x66, 0x66, 0x66, 0xff, 0x66, 0x66, 0x33, 0xff, 0x66, 0x66, 0x00, 0xff, 0x66, 0x33, 0xff, 0xff, 0x66, 0x33, 0xcc, 0xff, 0x66, 0x33, 0x99, 0xff, 0x66, 0x33, 0x66, 0xff,
        0x66, 0x33, 0x33, 0xff, 0x66, 0x33, 0x00, 0xff, 0x66, 0x00, 0xff, 0xff, 0x66, 0x00, 0xcc, 0xff, 0x66, 0x00, 0x99, 0xff, 0x66, 0x00, 0x66, 0xff, 0x66, 0x00, 0x33, 0xff, 0x66, 0x00, 0x00, 0xff,
        0x33, 0xff, 0xff, 0xff, 0x33, 0xff, 0xcc, 0xff, 0x33, 0xff, 0x99, 0xff, 0x33, 0xff, 0x66, 0xff, 0x33, 0xff, 0x33, 0xff, 0x33, 0xff, 0x00, 0xff, 0x33, 0xcc, 0xff, 0xff, 0x33, 0xcc, 0xcc, 0xff,
        0x33, 0xcc, 0x99, 0xff, 0x33, 0xcc, 0x66, 0xff, 0x33, 0xcc, 0x33, 0xff, 0x33, 0xcc, 0x00, 0xff, 0x33, 0x99, 0xff, 0xff, 0x33, 0x99, 0xcc, 0xff, 0x33, 0x99, 0x99, 0xff, 0x33, 0x99, 0x66, 0xff,
        0x33, 0x99, 0x33, 0xff, 0x33, 0x99, 0x00, 0xff, 0x33, 0x66, 0xff, 0xff, 0x33, 0x66, 0xcc, 0xff, 0x33, 0x66, 0x99, 0xff, 0x33, 0x66, 0x66, 0xff, 0x33, 0x66, 0x33, 0xff, 0x33, 0x66, 0x00, 0xff,
        0x33, 0x33, 0xff, 0xff, 0x33, 0x33, 0xcc, 0xff, 0x33, 0x33, 0x99, 0xff, 0x33, 0x33, 0x66, 0xff, 0x33, 0x33, 0x33, 0xff, 0x33, 0x33, 0x00, 0xff, 0x33, 0x00, 0xff, 0xff, 0x33, 0x00, 0xcc, 0xff,
        0x33, 0x00, 0x99, 0xff, 0x33, 0x00, 0x66, 0xff, 0x33, 0x00, 0x33, 0xff, 0x33, 0x00, 0x00, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xcc, 0xff, 0x00, 0xff, 0x99, 0xff, 0x00, 0xff, 0x66, 0xff,
        0x00, 0xff, 0x33, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xcc, 0xff, 0xff, 0x00, 0xcc, 0xcc, 0xff, 0x00, 0xcc, 0x99, 0xff, 0x00, 0xcc, 0x66, 0xff, 0x00, 0xcc, 0x33, 0xff, 0x00, 0xcc, 0x00, 0xff,
        0x00, 0x99, 0xff, 0xff, 0x00, 0x99, 0xcc, 0xff, 0x00, 0x99, 0x99, 0xff, 0x00, 0x99, 0x66, 0xff, 0x00, 0x99, 0x33, 0xff, 0x00, 0x99, 0x00, 0xff, 0x00, 0x66, 0xff, 0xff, 0x00, 0x66, 0xcc, 0xff,
        0x00, 0x66, 0x99, 0xff, 0x00, 0x66, 0x66, 0xff, 0x00, 0x66, 0x33, 0xff, 0x00, 0x66, 0x00, 0xff, 0x00, 0x33, 0xff, 0xff, 0x00, 0x33, 0xcc, 0xff, 0x00, 0x33, 0x99, 0xff, 0x00, 0x33, 0x66, 0xff,
        0x00, 0x33, 0x33, 0xff, 0x00, 0x33, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xcc, 0xff, 0x00, 0x00, 0x99, 0xff, 0x00, 0x00, 0x66, 0xff, 0x00, 0x00, 0x33, 0xff, 0xee, 0x00, 0x00, 0xff,
        0xdd, 0x00, 0x00, 0xff, 0xbb, 0x00, 0x00, 0xff, 0xaa, 0x00, 0x00, 0xff, 0x88, 0x00, 0x00, 0xff, 0x77, 0x00, 0x00, 0xff, 0x55, 0x00, 0x00, 0xff, 0x44, 0x00, 0x00, 0xff, 0x22, 0x00, 0x00, 0xff,
        0x11, 0x00, 0x00, 0xff, 0x00, 0xee, 0x00, 0xff, 0x00, 0xdd, 0x00, 0xff, 0x00, 0xbb, 0x00, 0xff, 0x00, 0xaa, 0x00, 0xff, 0x00, 0x88, 0x00, 0xff, 0x00, 0x77, 0x00, 0xff, 0x00, 0x55, 0x00, 0xff,
        0x00, 0x44, 0x00, 0xff, 0x00, 0x22, 0x00, 0xff, 0x00, 0x11, 0x00, 0xff, 0x00, 0x00, 0xee, 0xff, 0x00, 0x00, 0xdd, 0xff, 0x00, 0x00, 0xbb, 0xff, 0x00, 0x00, 0xaa, 0xff, 0x00, 0x00, 0x88, 0xff,
        0x00, 0x00, 0x77, 0xff, 0x00, 0x00, 0x55, 0xff, 0x00, 0x00, 0x44, 0xff, 0x00, 0x00, 0x22, 0xff, 0x00, 0x00, 0x11, 0xff, 0xee, 0xee, 0xee, 0xff, 0xdd, 0xdd, 0xdd, 0xff, 0xbb, 0xbb, 0xbb, 0xff,
        0xaa, 0xaa, 0xaa, 0xff, 0x88, 0x88, 0x88, 0xff, 0x77, 0x77, 0x77, 0xff, 0x55, 0x55, 0x55, 0xff, 0x44, 0x44, 0x44, 0xff, 0x22, 0x22, 0x22, 0xff, 0x11, 0x11, 0x11, 0xff, 0x00, 0x00, 0x00, 0xff,
    };

    // internal math/helper utilities
    static inline uint32_t _vox_max(uint32_t a, uint32_t b) {
        return (a > b) ? a : b;
    }
    static inline uint32_t _vox_min(uint32_t a, uint32_t b) {
        return (a < b) ? a : b;
    }

    // string utilities
    #ifdef _MSC_VER
        #define _vox_str_scanf(str,...)      sscanf_s(str,__VA_ARGS__)
        #define _vox_strcasecmp(a,b)         _stricmp(a,b)
        #define _vox_strcmp(a,b)             strcmp(a,b)
        #define _vox_strlen(a)               strlen(a)
        #define _vox_sprintf(str,str_max,fmt,...)    sprintf_s(str, str_max, fmt, __VA_ARGS__)
    #else
        #define _vox_str_scanf(str,...)      sscanf(str,__VA_ARGS__)
        #define _vox_strcasecmp(a,b)         strcasecmp(a,b)
        #define _vox_strcmp(a,b)             strcmp(a,b)
        #define _vox_strlen(a)               strlen(a)
        #define _vox_sprintf(str,str_max,fmt,...)    snprintf(str, str_max, fmt, __VA_ARGS__)
    #endif

    // Like strcpy, but will only copy the amount of characters from src that can fit in dst.
    // Will assert if too many characters are in src.
    template <size_t SIZE>
    inline void _vox_strcpy_static(char (&dst)[SIZE], const char* src){
        size_t storage_needed = _vox_strlen(src) + 1; // +1 for zero terminator
        ogt_assert(storage_needed <= SIZE, "strcpy of src into dst would exceed storage in dst");
        memcpy(dst, src, storage_needed <= SIZE ? storage_needed : SIZE);
        dst[SIZE-1] = 0; // make sure we're always zero terminated.
    }

    // 3d vector utilities
    struct vec3 {
        float x, y, z;
    };
    static inline vec3 vec3_make(float x, float y, float z) { vec3 v; v.x = x; v.y = y; v.z = z; return v; }
    static inline vec3 vec3_negate(const vec3& v) { vec3 r; r.x = -v.x;  r.y = -v.y; r.z = -v.z; return r; }

    // API for emulating file transactions on an in-memory buffer of data.
    struct _vox_file {
        const  uint8_t* buffer;       // source buffer data
        const uint32_t  buffer_size;  // size of the data in the buffer
        uint32_t        offset;       // current offset in the buffer data.
    };

    static uint32_t _vox_file_bytes_remaining(const _vox_file* fp) {
        if (fp->offset < fp->buffer_size) {
            return fp->buffer_size - fp->offset;
        } else {
            return 0;
        }
    }

    static bool _vox_file_read(_vox_file* fp, void* data, uint32_t data_size) {
        size_t data_to_read = _vox_min(_vox_file_bytes_remaining(fp), data_size);
        memcpy(data, &fp->buffer[fp->offset], data_to_read);
        fp->offset += data_size;
        return data_to_read == data_size;
    }

    static bool _vox_file_read_uint32(_vox_file* fp, uint32_t* data) {
        bool ret = _vox_file_read(fp, data, sizeof(*data));
        if (ret) {
            *data = _vox_le32toh(*data);
        }
        return ret;
    }

    static bool _vox_file_read_int32(_vox_file* fp, int32_t* data) {
        bool ret = _vox_file_read(fp, data, sizeof(*data));
        if (ret) {
            *data = _vox_le32toh(*data);
        }
        return ret;
    }

    static bool _vox_file_read_float(_vox_file* fp, float* data) {
        bool ret = _vox_file_read(fp, data, sizeof(*data));
        if (ret) {
            union {
                uint32_t u;
                float f;
            } bs;
            bs.f = *data;
            bs.u = _vox_le32toh(bs.u);
            *data = bs.f;
        }
        return ret;
    }

    static void _vox_file_seek_forwards(_vox_file* fp, uint32_t offset) {
        fp->offset += _vox_min(offset, _vox_file_bytes_remaining(fp));
    }

    static const void* _vox_file_data_pointer(const _vox_file* fp) {
        return &fp->buffer[fp->offset];
    }

    // hash utilities
    static uint32_t _vox_hash(const uint8_t* data, uint32_t data_size) {
        uint32_t hash = 0;
        for (uint32_t i = 0; i < data_size; i++)
            hash = data[i] + (hash * 65559);
        return hash;
    }

    // memory allocation utils.
    static void* _ogt_priv_alloc_default(size_t size) { return malloc(size); }
    static void  _ogt_priv_free_default(void* ptr)    { free(ptr); }
    static ogt_vox_alloc_func g_alloc_func = _ogt_priv_alloc_default; // default function for allocating
    static ogt_vox_free_func  g_free_func = _ogt_priv_free_default;   // default  function for freeing.

    // set the provided allocate/free functions if they are non-null, otherwise reset to default allocate/free functions
    void ogt_vox_set_memory_allocator(ogt_vox_alloc_func alloc_func, ogt_vox_free_func free_func)
    {
        ogt_assert((alloc_func && free_func) ||                             // both alloc/free must be non-NULL -OR-
            (!alloc_func && !free_func), "mixed alloc/free functions");     // both alloc/free must be NULL. No mixing 'n matching.
        if (alloc_func && free_func) {
            g_alloc_func = alloc_func;
            g_free_func = free_func;
        }
        else  {
            // reset to default allocate/free functions.
            g_alloc_func = _ogt_priv_alloc_default;
            g_free_func = _ogt_priv_free_default;
        }
    }

    static void* _vox_malloc(size_t size) {
        return size ? g_alloc_func(size) : NULL;
    }

    static void* _vox_calloc(size_t size) {
        void* pMem = _vox_malloc(size);
        if (pMem)
            memset(pMem, 0, size);
        return pMem;
    }

    static void _vox_free(void* old_ptr) {
        if (old_ptr)
            g_free_func(old_ptr);
    }

    static void* _vox_realloc(void* old_ptr, size_t old_size, size_t new_size) {
        // early out if new size is non-zero and no resize is required.
        if (new_size && old_size >= new_size)
            return old_ptr;

        // memcpy from the old ptr only if both sides are valid.
        void* new_ptr = _vox_malloc(new_size);
        if (new_ptr) {
            // copy any existing elements over
            if (old_ptr && old_size)
                memcpy(new_ptr, old_ptr, old_size);
            // zero out any new tail elements
            ogt_assert(new_size > old_size, "_vox_realloc error"); // this should be guaranteed by the _vox_realloc early out case above.
            uintptr_t new_tail_ptr = (uintptr_t)new_ptr + old_size;
            memset((void*)new_tail_ptr, 0, new_size - old_size);
        }
        if (old_ptr)
            _vox_free(old_ptr);
        return new_ptr;
    }

    // std::vector-style allocator, which use client-provided allocation functions.
    template <class T> struct _vox_array {
        _vox_array() : data(NULL), capacity(0), count(0) { }
        ~_vox_array() {
            _vox_free(data);
            data = NULL;
            count = 0;
            capacity = 0;
        }
        void reserve(size_t new_capacity) {
            data = (T*)_vox_realloc(data, capacity * sizeof(T), new_capacity * sizeof(T));
            capacity = new_capacity;
        }
        void grow_to_fit_index(size_t index) {
            if (index >= count)
                resize(index + 1);
        }
        void resize(size_t new_count) {
            if (new_count > capacity)
            {
                size_t new_capacity = capacity ? (capacity * 3) >> 1 : 2;   // grow by 50% each time, otherwise start at 2 elements.
                new_capacity = new_count > new_capacity ? new_count : new_capacity; // ensure fits new_count
                reserve(new_capacity);
                ogt_assert(capacity >= new_count, "failed to resize array");
            }
            count = new_count;
        }
        void push_back(const T & new_element) {
            if (count == capacity) {
                size_t new_capacity = capacity ? (capacity * 3) >> 1 : 2;   // grow by 50% each time, otherwise start at 2 elements.
                reserve(new_capacity);
                ogt_assert(capacity > count, "failed to push_back in array");
            }
            data[count++] = new_element;
        }
        void pop_back() {
            ogt_assert(count > 0, "cannot pop_back on empty array");
            count--;
        }
        const T & peek_back(size_t back_offset = 0) const {
            ogt_assert(back_offset < count, "can't peek back further than the number of elements in an array");
            size_t index = count - 1 - back_offset;
            return data[index];
        }

        void push_back_many(const T * new_elements, size_t num_elements) {
            if (count + num_elements > capacity) {
                size_t new_capacity = capacity + num_elements;
                new_capacity = new_capacity ? (new_capacity * 3) >> 1 : 2;   // grow by 50% each time, otherwise start at 2 elements.
                reserve(new_capacity);
                ogt_assert(capacity >= (count + num_elements), "failed to push_back_many in array");
            }
            for (size_t i = 0; i < num_elements; i++)
                data[count + i] = new_elements[i];
            count += num_elements;
        }
        T* alloc_many(size_t num_elements) {
            if (count + num_elements > capacity) {
                size_t new_capacity = capacity + num_elements;
                new_capacity = new_capacity ? (new_capacity * 3) >> 1 : 2;   // grow by 50% each time, otherwise start at 2 elements.
                reserve(new_capacity);
                ogt_assert(capacity >= (count + num_elements), "failed to push_back_many in array");
            }
            T* ret = &data[count];
            count += num_elements;
            return ret;
        }

        // returns the index that it was inserted at.
        uint32_t insert_unique_sorted( const T & value, uint32_t start_index ) {
            for (uint32_t i = start_index; i < (uint32_t)count; i++) {
                if (data[i] == value)
                    return i;
                if (data[i] >= value) {
                    resize(count+1);
                    for (size_t j = count-1; j > i; j--)
                        data[j] = data[j-1];
                    data[i] = value;
                    return i;
                }
            }
            push_back(value);
            return (uint32_t)(count-1);
        }

        size_t size() const {
            return count;
        }
        T& operator[](size_t index) {
            ogt_assert(index < count, "index out of bounds");
            return data[index];
        }
        const T& operator[](size_t index) const {
            ogt_assert(index < count, "index out of bounds");
            return data[index];
        }
        T*     data;      // data for the array
        size_t capacity;  // capacity of the array
        size_t count;      // size of the array
    };

    // a growing array where data is suballocated.
    class _vox_suballoc_array {
    public:
        _vox_suballoc_array()  {
            data.reserve(1024);
            // push a sentinel character into this array. This allows clients to keep an
            // offset rather than a pointer and still allow an offset of 0 to mean invalid.
            data.push_back('X');
        }

        void reserve(size_t new_capacity) {
            data.reserve(new_capacity);
        }

        size_t size() const {
            return data.size();
        }

        // gets the offset of a pointer that was allocated within this array.
        size_t offset_of(void* ptr) const {
            size_t unaligned_data = (size_t)&data[0];
            size_t unaligned_ptr  = (size_t)ptr;
            ogt_assert(unaligned_ptr >= unaligned_data && unaligned_ptr < (unaligned_data + data.size()), "provided ptr is out of bounds in this array");
            return unaligned_ptr - unaligned_data;
        }

        // gets a typed pointer given an offset into the block
        template <class T>
        const T * get_ptr(size_t offset) const {
            ogt_assert(0 == offset % sizeof(T), "offset is not properly aligned for this datatype");
            return (T*)&data[offset];
        }

        // gets a mutable typed pointer given an offset into the block
        template <class T>
        T * get_ptr(size_t offset) {
            ogt_assert(0 == offset % sizeof(T), "offset is not properly aligned for this datatype");
            return (T*)&data[offset];
        }

        // allocates num_bytes of memory with optionally specified base offset alignment.
        void* alloc(size_t num_bytes, size_t align=0) {
            if (align > 1 && 0 != (data.size() % align)) {
                size_t padding = align - (data.size() % align);
                data.alloc_many(padding);
            }
            return data.alloc_many(num_bytes);
        }

        // allocates and returns a pointer to many elements of the specified type.
        // if align != 0, will use that alignment, otherwise will align to the size of the type T.
        template <class T>
        T* alloc_many(size_t num_elements, size_t align=0) {
            // if alignment is specified use that, otherwise default alignment to the size fo the type T
            align = align == 0 ? sizeof(T) : align;
            return (T*)alloc(sizeof(T) * num_elements, align);
        }

        // returns the offset in the memory blob that the data was pushed to.
        template <class T>
        size_t push_back_many(const T * new_elements, size_t num_elements, size_t align=0) {
            T * mem = alloc_many<T>(num_elements, align);
            if (!mem)
                return 0;
            memcpy(mem, new_elements, sizeof(T)*num_elements);
            return offset_of(mem);
        }

        // returns the offset in the memory blob that the string was pushed to.
        size_t push_string(const char* str) {
            size_t str_size = _vox_strlen(str) + 1; // +1 for terminator
            return push_back_many(str, str_size);
        }

    private:
        _vox_array<char> data;
    };

    // progress callback function.
    static ogt_vox_progress_callback_func g_progress_callback_func = NULL;
    static void* g_progress_callback_user_data = NULL;

    // set the progress callback function.
    void  ogt_vox_set_progress_callback_func(ogt_vox_progress_callback_func progress_callback_func, void* user_data)
    {
        g_progress_callback_func = progress_callback_func;
        g_progress_callback_user_data = user_data;
    }

    // matrix utilities
    ogt_vox_transform ogt_vox_transform_get_identity() {
        ogt_vox_transform t;
        t.m00 = 1.0f; t.m01 = 0.0f; t.m02 = 0.0f; t.m03 = 0.0f;
        t.m10 = 0.0f; t.m11 = 1.0f; t.m12 = 0.0f; t.m13 = 0.0f;
        t.m20 = 0.0f; t.m21 = 0.0f; t.m22 = 1.0f; t.m23 = 0.0f;
        t.m30 = 0.0f; t.m31 = 0.0f; t.m32 = 0.0f; t.m33 = 1.0f;
        return t;
    }

    ogt_vox_transform ogt_vox_transform_multiply(const ogt_vox_transform& a, const ogt_vox_transform& b) {
        ogt_vox_transform r;
        r.m00 = (a.m00 * b.m00) + (a.m01 * b.m10) + (a.m02 * b.m20) + (a.m03 * b.m30);
        r.m01 = (a.m00 * b.m01) + (a.m01 * b.m11) + (a.m02 * b.m21) + (a.m03 * b.m31);
        r.m02 = (a.m00 * b.m02) + (a.m01 * b.m12) + (a.m02 * b.m22) + (a.m03 * b.m32);
        r.m03 = (a.m00 * b.m03) + (a.m01 * b.m13) + (a.m02 * b.m23) + (a.m03 * b.m33);
        r.m10 = (a.m10 * b.m00) + (a.m11 * b.m10) + (a.m12 * b.m20) + (a.m13 * b.m30);
        r.m11 = (a.m10 * b.m01) + (a.m11 * b.m11) + (a.m12 * b.m21) + (a.m13 * b.m31);
        r.m12 = (a.m10 * b.m02) + (a.m11 * b.m12) + (a.m12 * b.m22) + (a.m13 * b.m32);
        r.m13 = (a.m10 * b.m03) + (a.m11 * b.m13) + (a.m12 * b.m23) + (a.m13 * b.m33);
        r.m20 = (a.m20 * b.m00) + (a.m21 * b.m10) + (a.m22 * b.m20) + (a.m23 * b.m30);
        r.m21 = (a.m20 * b.m01) + (a.m21 * b.m11) + (a.m22 * b.m21) + (a.m23 * b.m31);
        r.m22 = (a.m20 * b.m02) + (a.m21 * b.m12) + (a.m22 * b.m22) + (a.m23 * b.m32);
        r.m23 = (a.m20 * b.m03) + (a.m21 * b.m13) + (a.m22 * b.m23) + (a.m23 * b.m33);
        r.m30 = (a.m30 * b.m00) + (a.m31 * b.m10) + (a.m32 * b.m20) + (a.m33 * b.m30);
        r.m31 = (a.m30 * b.m01) + (a.m31 * b.m11) + (a.m32 * b.m21) + (a.m33 * b.m31);
        r.m32 = (a.m30 * b.m02) + (a.m31 * b.m12) + (a.m32 * b.m22) + (a.m33 * b.m32);
        r.m33 = (a.m30 * b.m03) + (a.m31 * b.m13) + (a.m32 * b.m23) + (a.m33 * b.m33);
        return r;
    }

    // dictionary utilities
    static const uint32_t k_vox_max_dict_buffer_size = 4096;
    static const uint32_t k_vox_max_dict_key_value_pairs = 256;
    struct _vox_dictionary {
        const char* keys[k_vox_max_dict_key_value_pairs];
        const char* values[k_vox_max_dict_key_value_pairs];
        uint32_t    num_key_value_pairs;
        char        buffer[k_vox_max_dict_buffer_size + 4];    // max 4096, +4 for safety
        uint32_t    buffer_mem_used;
    };

    static bool _vox_file_read_dict(_vox_dictionary * dict, _vox_file * fp) {
        uint32_t num_pairs_to_read = 0;
        _vox_file_read_uint32(fp, &num_pairs_to_read);
        ogt_assert(num_pairs_to_read <= k_vox_max_dict_key_value_pairs, "max key/value pairs exceeded in dictionary");

        dict->buffer_mem_used = 0;
        dict->num_key_value_pairs = 0;
        for (uint32_t i = 0; i < num_pairs_to_read; i++) {
            // get the size of the key string
            uint32_t key_string_size = 0;
            if (!_vox_file_read_uint32(fp, &key_string_size))
                return false;
            // allocate space for the key, and read it in.
            if (dict->buffer_mem_used + key_string_size > k_vox_max_dict_buffer_size)
                return false;
            char* key = &dict->buffer[dict->buffer_mem_used];
            dict->buffer_mem_used += key_string_size + 1;    // + 1 for zero terminator
            if (!_vox_file_read(fp, key, key_string_size))
                return false;
            key[key_string_size] = 0;    // zero-terminate
            ogt_assert(_vox_strlen(key) == key_string_size, "key size sanity check failed");

            // get the size of the value string
            uint32_t value_string_size = 0;
            if (!_vox_file_read_uint32(fp, &value_string_size))
                return false;
            // allocate space for the value, and read it in.
            if (dict->buffer_mem_used + value_string_size > k_vox_max_dict_buffer_size)
                return false;
            char* value = &dict->buffer[dict->buffer_mem_used];
            dict->buffer_mem_used += value_string_size + 1;    // + 1 for zero terminator
            if (!_vox_file_read(fp, value, value_string_size))
                return false;
            value[value_string_size] = 0;    // zero-terminate
            ogt_assert(_vox_strlen(value) == value_string_size, "value size sanity check failed");
            // now assign it in the dictionary
            dict->keys[dict->num_key_value_pairs] = key;
            dict->values[dict->num_key_value_pairs] = value;
            dict->num_key_value_pairs++;
        }

        return true;
    }

    // helper for looking up in the dictionary
    static const char* _vox_dict_get_value_as_string(const _vox_dictionary* dict, const char* key_to_find, const char* default_value = NULL) {
        for (uint32_t i = 0; i < dict->num_key_value_pairs; i++)
            if (_vox_strcasecmp(dict->keys[i], key_to_find) == 0)
                return dict->values[i];
        return default_value;
    }

    static bool _vox_dict_get_value_as_bool(const _vox_dictionary* dict, const char* key_to_find, bool default_value) {
        const char* str = _vox_dict_get_value_as_string(dict, key_to_find, NULL);
        if (!str)
            return default_value;
        return str[0] == '1' ? true : false;
    }

    static uint32_t _vox_dict_get_value_as_uint32(const _vox_dictionary* dict, const char* key_to_find, uint32_t default_value) {
        const char* str = _vox_dict_get_value_as_string(dict, key_to_find, NULL);
        if (!str)
            return default_value;
        uint32_t value;
        _vox_str_scanf(str, "%i", &value);
        return value;
    }

    // lookup table for _vox_make_transform_from_dict_strings
    static const vec3 k_vectors[4] = {
    vec3_make(1.0f, 0.0f, 0.0f),
    vec3_make(0.0f, 1.0f, 0.0f),
    vec3_make(0.0f, 0.0f, 1.0f),
    vec3_make(0.0f, 0.0f, 0.0f)    // invalid!
    };

    // lookup table for _vox_make_transform_from_dict_strings
    static const uint32_t k_row2_index[] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, 2, UINT32_MAX, 1, 0, UINT32_MAX };


    static ogt_vox_transform _vox_make_transform_from_dict_strings(const char* rotation_string, const char* translation_string) {
        ogt_vox_transform transform = ogt_vox_transform_get_identity();

        if (rotation_string != NULL) {
            // compute the per-row indexes into k_vectors[] array.
            // unpack rotation bits.
            //  bits  : meaning
            //  0 - 1 : index of the non-zero entry in the first row
            //  2 - 3 : index of the non-zero entry in the second row
            uint32_t packed_rotation_bits = atoi(rotation_string);
            uint32_t row0_vec_index = (packed_rotation_bits >> 0) & 3;
            uint32_t row1_vec_index = (packed_rotation_bits >> 2) & 3;
            uint32_t row2_vec_index = k_row2_index[(1 << row0_vec_index) | (1 << row1_vec_index)];    // process of elimination to determine row 2 index based on row0/row1 being one of {0,1,2} choose 2.
            ogt_assert(row2_vec_index != UINT32_MAX, "invalid packed rotation bits");

            // unpack rotation bits for vector signs
            //  bits  : meaning
            //  4     : the sign in the first row  (0 : positive; 1 : negative)
            //  5     : the sign in the second row (0 : positive; 1 : negative)
            //  6     : the sign in the third row  (0 : positive; 1 : negative)
            vec3 row0 = k_vectors[row0_vec_index];
            vec3 row1 = k_vectors[row1_vec_index];
            vec3 row2 = k_vectors[row2_vec_index];
            if (packed_rotation_bits & (1 << 4))
                row0 = vec3_negate(row0);
            if (packed_rotation_bits & (1 << 5))
                row1 = vec3_negate(row1);
            if (packed_rotation_bits & (1 << 6))
                row2 = vec3_negate(row2);

            // magicavoxel stores rows, we need columns, so we do the swizzle here into columns
            transform.m00 = row0.x; transform.m01 = row1.x; transform.m02 = row2.x;
            transform.m10 = row0.y; transform.m11 = row1.y; transform.m12 = row2.y;
            transform.m20 = row0.z; transform.m21 = row1.z; transform.m22 = row2.z;
        }

        if (translation_string != NULL) {
            int32_t x = 0;
            int32_t y = 0;
            int32_t z = 0;
            _vox_str_scanf(translation_string, "%i %i %i", &x, &y, &z);
            transform.m30 = (float)x;
            transform.m31 = (float)y;
            transform.m32 = (float)z;
        }
        return transform;
    }

    enum _vox_scene_node_type
    {
        k_nodetype_invalid   = 0,    // has not been parsed yet.
        k_nodetype_group     = 1,
        k_nodetype_transform = 2,
        k_nodetype_shape     = 3,
    };

    struct _vox_keyframe_transform {
        uint32_t          frame_index;
        ogt_vox_transform frame_transform;
    };

    struct _vox_keyframe_shape {
        uint32_t frame_index;
        uint32_t model_index;
    };

    struct _vox_scene_node_ {
        _vox_scene_node_type node_type;    // only gets assigned when this has been parsed, otherwise will be k_nodetype_invalid
        union {
            // used only when node_type == k_nodetype_transform
            struct {
                char              name[NAME_MAX_LEN];
                ogt_vox_transform transform;                // root transform (always the first anim frame transform in the case of animated transform)
                uint32_t          child_node_id;
                uint32_t          layer_id;
                bool              hidden;
                uint32_t          num_keyframes;           // number of key frames in this transform
                size_t            keyframe_offset;         // offset in misc_data array where the _vox_keyframe_transform data is stored.
                bool              loop;                    // keyframes are marked as looping
            } transform;
            // used only when node_type == k_nodetype_group
            struct {
                uint32_t first_child_node_id_index; // the index of the first child node ID within the ChildNodeID array
                uint32_t num_child_nodes;           // number of child node IDs starting at the first index
            } group;
            // used only when node_type == k_nodetype_shape
            struct {
                uint32_t model_id;                 // always the first model_id in the case of an animated shape
                uint32_t num_keyframes;            // number of key frames in this transform
                size_t   keyframe_offset;          // offset in misc_data array where the _vox_keyframe_shape data is stored
                bool     loop;                     // keyframes are marked as looping
            } shape;
        } u;
    };

    static void clear_anim_transform(ogt_vox_anim_transform* anim) {
        anim->num_keyframes = 0;
        anim->keyframes     = NULL;
        anim->loop          = false;
    }

    static void clear_anim_model(ogt_vox_anim_model* anim) {
        anim->num_keyframes = 0;
        anim->keyframes     = NULL;
        anim->loop          = false;
    }

    static void generate_instances_for_node(
        _vox_array<const _vox_scene_node_*> & stack, const _vox_array<_vox_scene_node_> & nodes, uint32_t node_index, const _vox_array<uint32_t> & child_id_array, const _vox_array<ogt_vox_model*> & model_ptrs,
        _vox_array<ogt_vox_instance> & instances, _vox_suballoc_array & misc_data, _vox_array<ogt_vox_group>& groups, uint32_t group_index, bool generate_keyframes)
    {
        const _vox_scene_node_* node = &nodes[node_index];
        switch (node->node_type)
        {
            case k_nodetype_transform:
            {
                stack.push_back(node);
                generate_instances_for_node(stack, nodes, node->u.transform.child_node_id, child_id_array, model_ptrs, instances, misc_data, groups, group_index,  generate_keyframes);
                stack.pop_back();
                break;
            }
            case k_nodetype_group:
            {
                // create a new group only if we're generating groups.
                uint32_t next_group_index = 0;
                {
                    const _vox_scene_node_* last_transform = stack.peek_back(0);
                    ogt_assert(last_transform->node_type == k_nodetype_transform, "expected transform node prior to group node");

                    next_group_index = (uint32_t)groups.size();
                    ogt_vox_group group;
                    group.parent_group_index = group_index;
                    group.transform          = last_transform->u.transform.transform;
                    group.hidden             = last_transform->u.transform.hidden;
                    group.layer_index        = last_transform->u.transform.layer_id;
                    group.name               = 0;
                    const char* transform_last_name = last_transform->u.transform.name;
                    if (transform_last_name && transform_last_name[0]) {
                        group.name = (const char*)misc_data.push_string(transform_last_name);
                    }
                    clear_anim_transform(&group.transform_anim);
                    if (generate_keyframes) {
                        group.transform_anim.num_keyframes = last_transform->u.transform.num_keyframes;
                        group.transform_anim.keyframes     = (const ogt_vox_keyframe_transform*)(last_transform->u.transform.keyframe_offset);
                        group.transform_anim.loop          = last_transform->u.transform.loop;
                    }
                    groups.push_back(group);
                }

                stack.push_back(node);
                const uint32_t* child_node_ids = (const uint32_t*)& child_id_array[node->u.group.first_child_node_id_index];
                for (uint32_t i = 0; i < node->u.group.num_child_nodes; i++) {
                    generate_instances_for_node(stack, nodes, child_node_ids[i], child_id_array, model_ptrs, instances, misc_data, groups, next_group_index, generate_keyframes);
                }
                stack.pop_back();
                break;
            }
            case k_nodetype_shape:
            {
                ogt_assert(node->u.shape.model_id < model_ptrs.size(), "unexpected model id for shape node");
                if (node->u.shape.model_id < model_ptrs.size() &&    // model ID is valid
                    model_ptrs[node->u.shape.model_id] != NULL )     // model is non-NULL.
                {
                    const _vox_scene_node_* last_transform = stack.peek_back(0);
                    const _vox_scene_node_* last_group     = stack.peek_back(1);
                    (void)last_group;
                    ogt_assert(last_transform->node_type == k_nodetype_transform, "parent node type to a shape node must be a transform node");
                    ogt_assert(last_group->node_type == k_nodetype_group, "grandparent node type to a shape node must be a group node");

                    ogt_vox_instance new_instance;
                    new_instance.model_index = node->u.shape.model_id;
                    new_instance.transform   = last_transform->u.transform.transform;
                    new_instance.layer_index = last_transform->u.transform.layer_id;
                    new_instance.group_index = group_index;
                    new_instance.hidden      = last_transform->u.transform.hidden;
                    // if we got a transform name, allocate space in misc_data for it and keep track of the index
                    // within string data. This will be patched to a real pointer at the very end.
                    new_instance.name = 0;
                    const char* transform_last_name = last_transform->u.transform.name;
                    if (transform_last_name && transform_last_name[0]) {
                        new_instance.name = (const char*)misc_data.push_string(transform_last_name);
                    }
                    // generate keyframes if necessary.
                    clear_anim_transform(&new_instance.transform_anim);
                    clear_anim_model(&new_instance.model_anim);
                    if (generate_keyframes) {
                        new_instance.model_anim.num_keyframes     = node->u.shape.num_keyframes;
                        new_instance.model_anim.keyframes         = (const ogt_vox_keyframe_model*)(node->u.shape.keyframe_offset);
                        new_instance.model_anim.loop              = node->u.shape.loop;
                        new_instance.transform_anim.num_keyframes = last_transform->u.transform.num_keyframes;
                        new_instance.transform_anim.keyframes     = (const ogt_vox_keyframe_transform*)(last_transform->u.transform.keyframe_offset);
                        new_instance.transform_anim.loop          = last_transform->u.transform.loop;
                    }

                    // create the instance
                    instances.push_back(new_instance);
                }
                break;
            }
            default:
            {
                ogt_assert(0, "unhandled node type");
            }
        }
    }

    // returns true if the 2 models are content-wise identical.
    static bool _vox_models_are_equal(const ogt_vox_model* lhs, const ogt_vox_model* rhs) {
        // early out: if hashes don't match, they can't be equal
        // if hashes match, they might be equal OR there might be a hash collision.
        if (lhs->voxel_hash != rhs->voxel_hash)
            return false;
        // early out: if size of voxels in the model's grid don't match, they can't be equal.
        if (lhs->size_x != rhs->size_x || lhs->size_y != rhs->size_y || lhs->size_z != rhs->size_z )
            return false;
        // Finally, we know their hashes are the same, and their dimensions are the same
        // but they are only equal if they have exactly the same voxel data.
        uint32_t num_voxels_lhs = lhs->size_x * lhs->size_y * lhs->size_z;
        return memcmp(lhs->voxel_data, rhs->voxel_data, num_voxels_lhs) == 0 ? true : false;
    }

    // wraps the specified frame_index such that it is inclusively between the first and last loop frames
    uint32_t compute_looped_frame_index(uint32_t first_loop_frame, uint32_t last_loop_frame, uint32_t frame_index)
    {
        uint32_t loop_len = 1 + last_loop_frame - first_loop_frame;
        uint32_t looped_frame_index;
        if (frame_index >= first_loop_frame) {
            uint32_t frames_since_first_frame = frame_index - first_loop_frame;
            looped_frame_index = first_loop_frame + (frames_since_first_frame % loop_len);
        }
        else {
            uint32_t frames_since_first_frame = (first_loop_frame - frame_index - 1);
            looped_frame_index = last_loop_frame - (frames_since_first_frame % loop_len);
        }
        ogt_assert(looped_frame_index >= first_loop_frame && looped_frame_index <= last_loop_frame, "bug in looping logic!");
        return looped_frame_index;
    }

    static ogt_vox_transform sample_keyframe_transform(const ogt_vox_keyframe_transform* keyframes, uint32_t num_keyframes, bool loop, uint32_t frame_index)
    {
        ogt_assert(num_keyframes >= 1, "need at least one keyframe to sample");
        if (loop) {
            frame_index = compute_looped_frame_index(keyframes[0].frame_index, keyframes[num_keyframes-1].frame_index, frame_index);
        }
        if (frame_index <= keyframes[0].frame_index)
            return keyframes[0].transform;
        if (frame_index >= keyframes[num_keyframes-1].frame_index)
            return keyframes[num_keyframes-1].transform;
        for (int32_t f = (int32_t)num_keyframes-2; f >= 0; f--) {
            if (frame_index >= keyframes[f].frame_index) {
                // orientation always snaps
                uint32_t next_frame = keyframes[f+1].frame_index;
                uint32_t curr_frame = keyframes[f+0].frame_index;
                float t = (frame_index - curr_frame)  / (float)(next_frame - curr_frame);
                float t_inv = 1.0f - t;
                // orientation always snaps to the earlier frame
                ogt_vox_transform curr_transform = keyframes[f+0].transform;
                // position gets interpolated with rounding towards zero - TODO(jpaver) or should it be -INF?
                const ogt_vox_transform& next_transform = keyframes[f+1].transform;
                curr_transform.m30 = (float)(int32_t)((next_transform.m30 * t) + (curr_transform.m30 * t_inv));
                curr_transform.m31 = (float)(int32_t)((next_transform.m31 * t) + (curr_transform.m31 * t_inv));
                curr_transform.m32 = (float)(int32_t)((next_transform.m32 * t) + (curr_transform.m32 * t_inv));
                return curr_transform;

            }
        }
        ogt_assert(0, "shouldn't reach here");
        return keyframes[0].transform;
    }

    ogt_vox_transform ogt_vox_sample_anim_transform(const ogt_vox_anim_transform* anim, uint32_t frame_index)
    {
        return sample_keyframe_transform(anim->keyframes, anim->num_keyframes, anim->loop, frame_index);
    }

    uint32_t ogt_vox_sample_anim_model(const ogt_vox_anim_model* anim, uint32_t frame_index)
    {
        ogt_assert(anim->num_keyframes >= 1, "need at least one keyframe to sample");
        if (anim->loop) {
            frame_index = compute_looped_frame_index(anim->keyframes[0].frame_index, anim->keyframes[anim->num_keyframes-1].frame_index, frame_index);
        }
        if (frame_index <= anim->keyframes[0].frame_index)
            return anim->keyframes[0].model_index;
        if (frame_index >=anim-> keyframes[anim->num_keyframes-1].frame_index)
            return anim->keyframes[anim->num_keyframes-1].model_index;
        for (int32_t f = (int32_t)anim->num_keyframes-2; f >= 0; f--) {
            if (frame_index >= anim->keyframes[f].frame_index) {
                return anim->keyframes[f+0].model_index;
            }
        }
        ogt_assert(0, "shouldn't reach here");
        return anim->keyframes[0].model_index;
    }

    // computes the flattened transform for an instance on a given frame (pass the scene so that group transform hierarchy can also be considered)
    ogt_vox_transform ogt_vox_sample_group_transform_global(const ogt_vox_group* group, uint32_t frame_index, const ogt_vox_scene* scene)
    {
        ogt_vox_transform flattened_transform = ogt_vox_sample_group_transform_local(group, frame_index);
        uint32_t group_index = group->parent_group_index;
        while (group_index != k_invalid_group_index) {
            group = &scene->groups[group_index];
            ogt_vox_transform group_transform = ogt_vox_sample_group_transform_local(group, frame_index);
            flattened_transform = ogt_vox_transform_multiply(flattened_transform, group_transform);
            group_index = group->parent_group_index;
        }
        return flattened_transform;
    }
    // computes the global transform of a group on a given frame (global = flattened = world space)
    ogt_vox_transform ogt_vox_sample_instance_transform_global(const ogt_vox_instance* instance, uint32_t frame_index, const ogt_vox_scene* scene)
    {
        ogt_vox_transform final_transform = ogt_vox_sample_instance_transform_local(instance, frame_index);
        uint32_t group_index = instance->group_index;
        if (group_index == k_invalid_group_index)
            return final_transform;
        const ogt_vox_group* group = &scene->groups[group_index];
        return ogt_vox_transform_multiply(final_transform, ogt_vox_sample_group_transform_global(group, frame_index, scene));
    }

    uint32_t ogt_vox_sample_instance_model(const ogt_vox_instance* instance, uint32_t frame_index)
    {
        return instance->model_anim.num_keyframes ? ogt_vox_sample_anim_model(&instance->model_anim, frame_index) : instance->model_index;
    }

    ogt_vox_transform ogt_vox_sample_instance_transform_local(const ogt_vox_instance* instance, uint32_t frame_index)
    {
        return instance->transform_anim.num_keyframes ? ogt_vox_sample_anim_transform(&instance->transform_anim, frame_index) : instance->transform;
    }

    ogt_vox_transform ogt_vox_sample_group_transform_local(const ogt_vox_group* group, uint32_t frame_index)
    {
        return group->transform_anim.num_keyframes ? ogt_vox_sample_anim_transform(&group->transform_anim, frame_index) : group->transform;
    }

    const ogt_vox_scene* ogt_vox_read_scene_with_flags(const uint8_t * buffer, uint32_t buffer_size, uint32_t read_flags) {
        _vox_file file = { buffer, buffer_size, 0 };
        _vox_file* fp = &file;

        // parsing state/context
        _vox_array<ogt_vox_model*>   model_ptrs;
        _vox_array<_vox_scene_node_> nodes;
        _vox_array<ogt_vox_instance> instances;
        _vox_array<ogt_vox_cam>      cameras;
        _vox_suballoc_array           misc_data;
        _vox_array<ogt_vox_layer>    layers;
        _vox_array<ogt_vox_group>    groups;
        _vox_array<uint32_t>         child_ids;
        ogt_vox_palette              palette;
        ogt_vox_matl_array           materials;
        _vox_dictionary              dict;
        uint32_t                     size_x = 0;
        uint32_t                     size_y = 0;
        uint32_t                     size_z = 0;
        uint8_t                      index_map[256];
        bool                         found_index_map_chunk = false;

        // size some of our arrays to prevent resizing during the parsing for smallish cases.
        model_ptrs.reserve(64);
        instances.reserve(256);
        cameras.reserve(4);
        child_ids.reserve(256);
        nodes.reserve(16);
        layers.reserve(8);
        groups.reserve(0);
        misc_data.reserve(1024);

        // push a sentinel character into these datastructures. This allows us to keep indexes
        // rather than pointers into data-structures that grow, and still allow an index of 0
        // to means invalid
        child_ids.push_back(UINT32_MAX);

        // copy the default palette into the scene. It may get overwritten by a palette chunk later
        memcpy(&palette, k_default_vox_palette, sizeof(ogt_vox_palette));

        // zero initialize materials (this sets valid defaults)
        memset(&materials, 0, sizeof(materials));

        // load and validate fileheader and file version.
        uint32_t file_header = 0;
        uint32_t file_version = 0;
        _vox_file_read_uint32(fp, &file_header);
        _vox_file_read_uint32(fp, &file_version);
        if (file_header != CHUNK_ID_VOX_ || (file_version != 150 && file_version != 200))
            return NULL;

        // parse chunks until we reach the end of the file/buffer
        while (_vox_file_bytes_remaining(fp) >= sizeof(uint32_t) * 3)
        {
            // read the fields common to all chunks
            uint32_t chunk_id         = 0;
            uint32_t chunk_size       = 0;
            uint32_t chunk_child_size = 0;
            _vox_file_read_uint32(fp, &chunk_id);
            _vox_file_read_uint32(fp, &chunk_size);
            _vox_file_read_uint32(fp, &chunk_child_size);

            // process the chunk.
            switch (chunk_id)
            {
                case CHUNK_ID_MAIN:
                {
                    break;
                }
                case CHUNK_ID_SIZE:
                {
                    ogt_assert(chunk_size == CHUNK_HEADER_LEN && chunk_child_size == 0, "unexpected chunk size for SIZE chunk");
                    _vox_file_read_uint32(fp, &size_x);
                    _vox_file_read_uint32(fp, &size_y);
                    _vox_file_read_uint32(fp, &size_z);
                    ogt_assert(size_x && size_y && size_z, "SIZE chunk has zero size");
                    break;
                }
                case CHUNK_ID_XYZI:
                {
                    ogt_assert(size_x && size_y && size_z, "expected a SIZE chunk before XYZI chunk");
                    // read the number of voxels to process for this moodel
                    uint32_t num_voxels_in_chunk = 0;
                    _vox_file_read_uint32(fp, &num_voxels_in_chunk);
                    if (num_voxels_in_chunk != 0 || (read_flags & k_read_scene_flags_keep_empty_models_instances)) {
                        uint32_t voxel_count = size_x * size_y * size_z;
                        ogt_vox_model * model = (ogt_vox_model*)_vox_calloc(sizeof(ogt_vox_model) + voxel_count);        // 1 byte for each voxel
                        if (!model)
                            return NULL;
                        uint8_t * voxel_data = (uint8_t*)&model[1];

                        // insert the model into the model array
                        model_ptrs.push_back(model);

                        // now setup the model
                        model->size_x = size_x;
                        model->size_y = size_y;
                        model->size_z = size_z;
                        model->voxel_data = voxel_data;

                        // setup some strides for computing voxel index based on x/y/z
                        const uint32_t k_stride_x = 1;
                        const uint32_t k_stride_y = size_x;
                        const uint32_t k_stride_z = size_x * size_y;

                        // read this many voxels and store it in voxel data.
                        const uint8_t * packed_voxel_data = (const uint8_t*)_vox_file_data_pointer(fp);
                        const uint32_t voxels_to_read = _vox_min(_vox_file_bytes_remaining(fp) / 4, num_voxels_in_chunk);
                        for (uint32_t i = 0; i < voxels_to_read; i++) {
                            uint8_t x = packed_voxel_data[i * 4 + 0];
                            uint8_t y = packed_voxel_data[i * 4 + 1];
                            uint8_t z = packed_voxel_data[i * 4 + 2];
                            uint8_t color_index = packed_voxel_data[i * 4 + 3];
                            ogt_assert(x < size_x && y < size_y && z < size_z, "invalid data in XYZI chunk");
                            voxel_data[(x * k_stride_x) + (y * k_stride_y) + (z * k_stride_z)] = color_index;
                        }
                        _vox_file_seek_forwards(fp, num_voxels_in_chunk * 4);
                        // compute the hash of the voxels in this model-- used to accelerate duplicate models checking.
                        model->voxel_hash = _vox_hash(voxel_data, size_x * size_y * size_z);
                    }
                    else {
                        model_ptrs.push_back(NULL);
                    }
                    break;
                }
                case CHUNK_ID_RGBA:
                {
                    ogt_assert(chunk_size == sizeof(palette), "unexpected chunk size for RGBA chunk");
                    _vox_file_read(fp, &palette, sizeof(palette));
                    break;
                }
                case CHUNK_ID_nTRN:
                {
                    uint32_t node_id = 0;
                    _vox_file_read_uint32(fp, &node_id);

                    // Parse the node dictionary, which can contain:
                    //   _name:   string
                    //   _hidden: 0/1
                    //   _loop:   0/1
                    _vox_file_read_dict(&dict, fp);
                    char node_name[NAME_MAX_LEN];
                    _vox_strcpy_static(node_name, _vox_dict_get_value_as_string(&dict, "_name", ""));
                    bool hidden = _vox_dict_get_value_as_bool(&dict, "_hidden", false);
                    bool loop = _vox_dict_get_value_as_bool(&dict, "_loop", false);

                    // get other properties.
                    uint32_t child_node_id = 0, reserved_id = 0, layer_id = 0, num_frames = 0;
                    _vox_file_read_uint32(fp, &child_node_id);
                    _vox_file_read_uint32(fp, &reserved_id);
                    _vox_file_read_uint32(fp, &layer_id);
                    _vox_file_read_uint32(fp, &num_frames);
                    // ogt_assert(reserved_id == UINT32_MAX, "unexpected values for reserved_id in nTRN chunk");
                    ogt_assert(num_frames > 0, "must have at least 1 frame in nTRN chunk");

                    // make space in misc_data array for the number of transforms we'll need for this node
                    ogt_vox_keyframe_transform* keyframes = misc_data.alloc_many<ogt_vox_keyframe_transform>(num_frames);
                    size_t keyframe_offset = misc_data.offset_of(keyframes);

                    for (uint32_t i = 0; i < num_frames; i++) {
                        // Parse the frame dictionary that contains:
                        //   _r : int8 ROTATION (c)
                        //   _t : int32x3 translation
                        //   _f : uint32_t frame index
                        _vox_file_read_dict(&dict, fp);
                        const char* rotation_value    = _vox_dict_get_value_as_string(&dict, "_r");
                        const char* translation_value = _vox_dict_get_value_as_string(&dict, "_t");
                        keyframes[i].transform        = _vox_make_transform_from_dict_strings(rotation_value, translation_value);
                        keyframes[i].frame_index      = _vox_dict_get_value_as_uint32(&dict, "_f", 0);
                    }
                    // setup the transform node.
                    {
                        nodes.grow_to_fit_index(node_id);
                        _vox_scene_node_* transform_node = &nodes[node_id];
                        transform_node->node_type                   = k_nodetype_transform;
                        transform_node->u.transform.child_node_id   = child_node_id;
                        transform_node->u.transform.layer_id        = layer_id;
                        transform_node->u.transform.transform       = keyframes[0].transform;
                        transform_node->u.transform.hidden          = hidden;
                        transform_node->u.transform.num_keyframes   = num_frames;
                        transform_node->u.transform.keyframe_offset = keyframe_offset;
                        transform_node->u.transform.loop            = loop;
                        _vox_strcpy_static(transform_node->u.transform.name, node_name);
                    }
                    break;
                }
                case CHUNK_ID_nGRP:
                {
                    uint32_t node_id = 0;
                    _vox_file_read_uint32(fp, &node_id);

                    // parse the node dictionary - data is unused.
                    _vox_file_read_dict(&dict, fp);

                    // setup the group node
                    nodes.grow_to_fit_index(node_id);
                    _vox_scene_node_* group_node = &nodes[node_id];
                    group_node->node_type = k_nodetype_group;
                    group_node->u.group.first_child_node_id_index = 0;
                    group_node->u.group.num_child_nodes           = 0;

                    // setup all child scene nodes to point back to this node.
                    uint32_t num_child_nodes = 0;
                    _vox_file_read_uint32(fp, &num_child_nodes);

                    // allocate space for all the child node IDs
                    if (num_child_nodes) {
                        size_t prior_size = child_ids.size();
                        ogt_assert(prior_size > 0, "prior_size sanity test failed"); // should be guaranteed by the sentinel we reserved at the very beginning.
                        child_ids.resize(prior_size + num_child_nodes);
                        for (uint32_t i = 0; i < num_child_nodes; ++i) {
                            _vox_file_read_uint32(fp, &child_ids[prior_size + i]);
                        }
                        group_node->u.group.first_child_node_id_index = (uint32_t)prior_size;
                        group_node->u.group.num_child_nodes = num_child_nodes;
                    }
                    break;
                }
                case CHUNK_ID_nSHP:
                {
                    uint32_t node_id = 0;
                    _vox_file_read_uint32(fp, &node_id);

                    // parse the node dictionary
                    _vox_file_read_dict(&dict, fp);
                    bool loop = _vox_dict_get_value_as_bool(&dict, "_loop", false);

                    uint32_t num_models = 0;
                    _vox_file_read_uint32(fp, &num_models);
                    ogt_assert(num_models > 0, "must have at least 1 frame in nSHP chunk"); // must be 1 according to the spec.

                    ogt_vox_keyframe_model* keyframes = misc_data.alloc_many<ogt_vox_keyframe_model>(num_models);
                    size_t keyframe_offset = misc_data.offset_of(keyframes);

                    for (uint32_t i = 0; i < num_models; i++) {
                        // read model id
                        _vox_file_read_uint32(fp, &keyframes[i].model_index);
                        ogt_assert(keyframes[i].model_index < model_ptrs.size(), "nSHP chunk references model_id that we have not loaded yet");
                        // read frame id
                        _vox_file_read_dict(&dict, fp);
                        keyframes[i].frame_index = _vox_dict_get_value_as_uint32(&dict, "_f", 0);
                    }

                    // setup the shape node
                    nodes.grow_to_fit_index(node_id);
                    _vox_scene_node_* shape_node = &nodes[node_id];
                    shape_node->node_type = k_nodetype_shape;
                    shape_node->u.shape.model_id        = keyframes[0].model_index;
                    shape_node->u.shape.num_keyframes   = num_models;
                    shape_node->u.shape.keyframe_offset = keyframe_offset;
                    shape_node->u.shape.loop            = loop;
                    break;
                }
                case CHUNK_ID_IMAP:
                {
                    ogt_assert(chunk_size == 256, "unexpected chunk size for IMAP chunk");
                    _vox_file_read(fp, index_map, 256);
                    found_index_map_chunk = true;
                    break;
                }
                case CHUNK_ID_LAYR:
                {
                    int32_t layer_id = 0;
                    int32_t reserved_id = 0;
                    _vox_file_read_int32(fp, &layer_id);
                    _vox_file_read_dict(&dict, fp);
                    _vox_file_read_int32(fp, &reserved_id);
                    ogt_assert(reserved_id == -1, "unexpected value for reserved_id in LAYR chunk");

                    layers.grow_to_fit_index(layer_id);
                    layers[layer_id].name   = NULL;
                    layers[layer_id].color = {255, 255, 255, 255};
                    layers[layer_id].hidden = false;

                    // if we got a layer name from the LAYR dictionary, allocate space in misc_data for it and keep track of the index
                    // within string data. This will be patched to a real pointer at the very end.
                    const char* name_string = _vox_dict_get_value_as_string(&dict, "_name", NULL);
                    if (name_string) {
                        layers[layer_id].name = (const char*)misc_data.push_string(name_string);
                    }
                    layers[layer_id].hidden = _vox_dict_get_value_as_bool(&dict, "_hidden", false);

                    const char* color_string = _vox_dict_get_value_as_string(&dict, "_color", NULL);
                    if (color_string) {
                        uint32_t r,g,b;
                        _vox_str_scanf(color_string, "%u %u %u", &r, &g, &b);
                        layers[layer_id].color.r = (uint8_t)r;
                        layers[layer_id].color.g = (uint8_t)g;
                        layers[layer_id].color.b = (uint8_t)b;
                    }

                    break;
                }
                case CHUNK_ID_MATL:
                {
                    int32_t material_id = 0;
                    _vox_file_read_int32(fp, &material_id);
                    material_id = material_id & 0xFF; // incoming material 256 is material 0
                    _vox_file_read_dict(&dict, fp);
                    const char* type_string = _vox_dict_get_value_as_string(&dict, "_type", NULL);
                    if (type_string) {
                        if (0 == _vox_strcmp(type_string,"_diffuse")) {
                            materials.matl[material_id].type = ogt_matl_type_diffuse;
                        }
                        else if (0 == _vox_strcmp(type_string,"_metal")) {
                            materials.matl[material_id].type = ogt_matl_type_metal;
                        }
                        else if (0 == _vox_strcmp(type_string,"_glass")) {
                            materials.matl[material_id].type = ogt_matl_type_glass;
                        }
                        else if (0 == _vox_strcmp(type_string,"_emit")) {
                            materials.matl[material_id].type = ogt_matl_type_emit;
                        }
                        else if (0 == _vox_strcmp(type_string,"_blend")) {
                            materials.matl[material_id].type = ogt_matl_type_blend;
                        }
                        else if (0 == _vox_strcmp(type_string,"_media")) {
                            materials.matl[material_id].type = ogt_matl_type_media;
                        }
                    }
                    materials.matl[material_id].media_type = ogt_media_type_absorb;
                    const char* media_type_string = _vox_dict_get_value_as_string(&dict, "_media_type", NULL);
                    if (media_type_string) {
                        if (0 == _vox_strcmp(media_type_string,"_scatter")) {
                           materials.matl[material_id].media_type = ogt_media_type_scatter;
                        }
                        else if (0 == _vox_strcmp(media_type_string,"_emit")) {
                            materials.matl[material_id].media_type = ogt_media_type_emit;
                        }
                        else if (0 == _vox_strcmp(media_type_string,"_sss")) {
                            materials.matl[material_id].media_type = ogt_media_type_sss;
                        }
                    }

                    const char* metal_string = _vox_dict_get_value_as_string(&dict, "_metal", NULL);
                    if (metal_string) {
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_metal;
                        materials.matl[material_id].metal = (float)atof(metal_string);
                    }
                    const char* rough_string = _vox_dict_get_value_as_string(&dict, "_rough", NULL);
                    if (rough_string) {
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_rough;
                        materials.matl[material_id].rough = (float)atof(rough_string);
                    }
                    const char* spec_string = _vox_dict_get_value_as_string(&dict, "_spec", NULL);
                    if (spec_string) {
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_spec;
                        materials.matl[material_id].spec = (float)atof(spec_string);
                    }
                    const char* ior_string = _vox_dict_get_value_as_string(&dict, "_ior", NULL);
                    if (ior_string) {
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_ior;
                        materials.matl[material_id].ior = (float)atof(ior_string);
                    }
                    const char* ri_string = _vox_dict_get_value_as_string(&dict, "_ri", NULL);
                    if (ri_string) {
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_ior;
                        materials.matl[material_id].ior = (float)atof(ri_string) - 1.0f;
                    }
                    const char* att_string = _vox_dict_get_value_as_string(&dict, "_att", NULL);
                    if (att_string) {
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_att;
                        materials.matl[material_id].att = (float)atof(att_string);
                    }
                    const char* flux_string = _vox_dict_get_value_as_string(&dict, "_flux", NULL);
                    if (flux_string) {
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_flux;
                        materials.matl[material_id].flux = (float)atof(flux_string);
                    }
                    const char* emit_string = _vox_dict_get_value_as_string(&dict, "_emit", NULL);
                    if (emit_string) {
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_emit;
                        materials.matl[material_id].emit = (float)atof(emit_string);
                    }
                    const char* ldr_string = _vox_dict_get_value_as_string(&dict, "_ldr", NULL);
                    if (ldr_string) {
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_ldr;
                        materials.matl[material_id].ldr = (float)atof(ldr_string);
                    }
                    const char* trans_string = _vox_dict_get_value_as_string(&dict, "_trans", NULL);
                    if (trans_string) {
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_trans;
                        materials.matl[material_id].trans = (float)atof(trans_string);
                    }
                    const char* alpha_string = _vox_dict_get_value_as_string(&dict, "_alpha", NULL);
                    if (alpha_string) {
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_alpha;
                        materials.matl[material_id].alpha = (float)atof(alpha_string);
                    }
                    const char* d_string = _vox_dict_get_value_as_string(&dict, "_d", NULL);
                    if (d_string) {
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_d;
                        materials.matl[material_id].d = (float)atof(d_string);
                    }
                    const char* sp_string = _vox_dict_get_value_as_string(&dict, "_sp", NULL);
                    if (sp_string) {
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_sp;
                        materials.matl[material_id].sp = (float)atof(sp_string);
                    }
                    const char* g_string = _vox_dict_get_value_as_string(&dict, "_g", NULL);
                    if (g_string) {
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_g;
                        materials.matl[material_id].g = (float)atof(g_string);
                    }
                    const char* media_string = _vox_dict_get_value_as_string(&dict, "_media", NULL);
                    if (media_string) {
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_media;
                        materials.matl[material_id].media = (float)atof(media_string);
                    }
                    break;
                }
                case CHUNK_ID_MATT:
                {
                    int32_t material_id = 0;
                    _vox_file_read_int32(fp, &material_id);
                    material_id = material_id & 0xFF; // incoming material 256 is material 0

                    // 0 : diffuse
                    // 1 : metal
                    // 2 : glass
                    // 3 : emissive
                    int32_t material_type = 0;
                    _vox_file_read_int32(fp, &material_type);

                    // diffuse  : 1.0
                    // metal    : (0.0 - 1.0] : blend between metal and diffuse material
                    // glass    : (0.0 - 1.0] : blend between glass and diffuse material
                    // emissive : (0.0 - 1.0] : self-illuminated material
                    float material_weight = 0.0f;
                    _vox_file_read_float(fp, &material_weight);

                    // bit(0) : Plastic
                    // bit(1) : Roughness
                    // bit(2) : Specular
                    // bit(3) : IOR
                    // bit(4) : Attenuation
                    // bit(5) : Power
                    // bit(6) : Glow
                    // bit(7) : isTotalPower (*no value)
                    uint32_t property_bits = 0u;
                    _vox_file_read_uint32(fp, &property_bits);

                    materials.matl[material_id].type = (ogt_matl_type)material_type;
                    switch (material_type) {
                    case ogt_matl_type_diffuse:
                        break;
                    case ogt_matl_type_metal:
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_metal;
                        materials.matl[material_id].metal = material_weight;
                        break;
                    case ogt_matl_type_glass:
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_trans;
                        materials.matl[material_id].trans = material_weight;
                        break;
                    case ogt_matl_type_emit:
                        materials.matl[material_id].content_flags |= k_ogt_vox_matl_have_emit;
                        materials.matl[material_id].emit = material_weight;
                        break;
                    }

                    ogt_assert(chunk_size >= 16u, "unexpected chunk size for MATT chunk");
                    const uint32_t remaining = chunk_size - 16u;
                    _vox_file_seek_forwards(fp, remaining);
                    break;
                }
                case CHUNK_ID_rCAM:
                {
                    ogt_vox_cam camera;
                    memset(&camera, 0, sizeof(camera));
                    _vox_file_read_uint32(fp, &camera.camera_id);
                    _vox_file_read_dict(&dict, fp);

                    camera.mode = ogt_cam_mode_unknown;
                    const char* mode_string = _vox_dict_get_value_as_string(&dict, "_mode", NULL);
                    if (mode_string) {
                        if (!_vox_strcmp(mode_string, "pers")) {
                            camera.mode = ogt_cam_mode_perspective;
                        } else if (!_vox_strcmp(mode_string, "free")) {
                            camera.mode = ogt_cam_mode_free;
                        } else if (!_vox_strcmp(mode_string, "pano")) {
                            camera.mode = ogt_cam_mode_pano;
                        } else if (!_vox_strcmp(mode_string, "iso")) {
                            camera.mode = ogt_cam_mode_isometric;
                        } else if (!_vox_strcmp(mode_string, "orth")) {
                            camera.mode = ogt_cam_mode_orthographic;
                        }
                    }
                    const char* focus_string = _vox_dict_get_value_as_string(&dict, "_focus", NULL);
                    if (focus_string) {
                        _vox_str_scanf(focus_string, "%f %f %f", &camera.focus[0], &camera.focus[1], &camera.focus[2]);
                    }
                    const char* angle_string = _vox_dict_get_value_as_string(&dict, "_angle", NULL);
                    if (angle_string) {
                        _vox_str_scanf(angle_string, "%f %f %f", &camera.angle[0], &camera.angle[1], &camera.angle[2]);
                    }
                    const char* radius_string = _vox_dict_get_value_as_string(&dict, "_radius", NULL);
                    if (radius_string) {
                        _vox_str_scanf(radius_string, "%f", &camera.radius);
                    }
                    const char* frustum_string = _vox_dict_get_value_as_string(&dict, "_frustum", NULL);
                    if (frustum_string) {
                        _vox_str_scanf(frustum_string, "%f", &camera.frustum);
                    }
                    const char* fov_string = _vox_dict_get_value_as_string(&dict, "_fov", NULL);
                    if (fov_string) {
                        _vox_str_scanf(angle_string, "%i", &camera.fov);
                    }

                    cameras.push_back(camera);
                    break;
                }
                // we don't handle rOBJ (just a dict of render settings), so we just skip the chunk payload.
                case CHUNK_ID_rOBJ:
                default:
                {
                    _vox_file_seek_forwards(fp, chunk_size);
                    break;
                }
            } // end switch

            if (g_progress_callback_func) {
                // we indicate progress as 0.8f * amount of buffer read + 0.2f at end after processing
                if (!g_progress_callback_func(0.8f*(float)(fp->offset)/(float)(fp->buffer_size), g_progress_callback_user_data))
                {
                    return 0;
                }
            }
        }

        // ok, now that we've parsed all scene nodes - walk the scene hierarchy, and generate instances
        // we can't do this while parsing chunks unfortunately because some chunks reference chunks
        // that are later in the file than them.
        if (nodes.size()) {
            bool generate_groups    = read_flags & k_read_scene_flags_groups ? true : false;
            bool generate_keyframes = read_flags & k_read_scene_flags_keyframes ? true : false;
            // if we're not reading scene-embedded groups, we generate only one and then flatten all instance transforms.

            _vox_array< const _vox_scene_node_*> stack;
            stack.reserve(64);
            generate_instances_for_node(stack, nodes, 0, child_ids, model_ptrs, instances, misc_data, groups, k_invalid_group_index, generate_keyframes);

            // if caller doesn't want groups, we flatten out transforms for all instances and parent them to a single group
            if (!generate_groups) {
                // flatten all keyframes on instances.
                if (generate_keyframes) {
                    _vox_array<uint32_t> frame_indices;
                    frame_indices.reserve(256);
                    for (uint32_t i = 0; i < instances.size(); i++) {
                        ogt_vox_instance* instance = &instances[i];
                        // populate frame_indices with those that are used by the instance or any parent group of the instance.
                        {
                            frame_indices.resize(0);
                            // first populate frame_indices with the keyframes on the instance itself
                            uint32_t start_index = 0;
                            const ogt_vox_keyframe_transform* instance_keyframes = misc_data.get_ptr<ogt_vox_keyframe_transform>((size_t)instance->transform_anim.keyframes);
                            for (uint32_t f = 0; f < instance->transform_anim.num_keyframes; f++) {
                                start_index = frame_indices.insert_unique_sorted(instance_keyframes[f].frame_index, start_index);
                            }
                            // now populate frame_index with the keyframes on any parent groups.
                            uint32_t group_index = instance->group_index;
                            while (group_index != k_invalid_group_index) {
                                const ogt_vox_group* group = &groups[group_index];
                                const ogt_vox_keyframe_transform* group_keyframes = misc_data.get_ptr<ogt_vox_keyframe_transform>((size_t)group->transform_anim.keyframes);
                                start_index = 0;
                                for (uint32_t f = 0; f < group->transform_anim.num_keyframes; f++) {
                                    start_index = frame_indices.insert_unique_sorted(group_keyframes[f].frame_index, start_index);
                                }
                                group_index = group->parent_group_index;
                            }
                        }
                        // use the ordered frame indices to sample the flattened transform from the keyframes from the instance and all its parent groups
                        ogt_vox_keyframe_transform* new_keyframes = misc_data.alloc_many<ogt_vox_keyframe_transform>(frame_indices.size());
                        size_t new_keyframe_offset = misc_data.offset_of(new_keyframes);
                        for (uint32_t f = 0; f < frame_indices.size(); f++) {
                            uint32_t frame_index = frame_indices[f];
                            const ogt_vox_keyframe_transform* instance_keyframes = misc_data.get_ptr<ogt_vox_keyframe_transform>((size_t)instance->transform_anim.keyframes);
                            ogt_vox_transform flattened_transform = sample_keyframe_transform(instance_keyframes, instance->transform_anim.num_keyframes, instance->transform_anim.loop, frame_index);
                            uint32_t group_index = instance->group_index;
                            while (group_index != k_invalid_group_index) {
                                const ogt_vox_group* group = &groups[group_index];
                                const ogt_vox_keyframe_transform* group_keyframes = misc_data.get_ptr<ogt_vox_keyframe_transform>((size_t)group->transform_anim.keyframes);
                                ogt_vox_transform group_transform = sample_keyframe_transform(group_keyframes, group->transform_anim.num_keyframes, group->transform_anim.loop, frame_index);
                                flattened_transform = ogt_vox_transform_multiply(flattened_transform, group_transform);
                                group_index = groups[group_index].parent_group_index;
                            }
                            new_keyframes[f].frame_index = frame_index;
                            new_keyframes[f].transform   = flattened_transform;
                        }
                        instance->transform_anim.num_keyframes = (uint32_t)frame_indices.size();
                        instance->transform_anim.keyframes     = (ogt_vox_keyframe_transform*)new_keyframe_offset;
                    }
                }

                // now flatten instance transforms if there is no group hierarchy
                for (uint32_t i = 0; i < instances.size(); i++) {
                    ogt_vox_instance* instance = &instances[i];
                    ogt_vox_transform flattened_transform = instance->transform;
                    uint32_t group_index = instance->group_index;
                    while (group_index != k_invalid_group_index) {
                        flattened_transform = ogt_vox_transform_multiply(flattened_transform, groups[group_index].transform);
                        group_index = groups[group_index].parent_group_index;
                    }
                    instance->transform = flattened_transform;
                    instance->group_index = 0;
                }
                // add just a single parent group.
                groups.resize(0);
                ogt_vox_group root_group;
                root_group.name                         = 0;
                root_group.transform                    = ogt_vox_transform_get_identity();
                root_group.parent_group_index           = k_invalid_group_index;
                root_group.layer_index                  = 0;
                root_group.hidden                       = false;
                clear_anim_transform(&root_group.transform_anim);
                groups.push_back(root_group);
            }
        }
        else if (model_ptrs.size() == 1) {
            // add a single instance
            ogt_vox_instance new_instance;
            new_instance.model_index                = 0;
            new_instance.group_index                = 0;
            new_instance.transform                  = ogt_vox_transform_get_identity();
            new_instance.layer_index                = 0;
            new_instance.name                       = 0;
            new_instance.hidden                     = false;
            clear_anim_transform(&new_instance.transform_anim);
            clear_anim_model(&new_instance.model_anim);
            instances.push_back(new_instance);
            // adds a single group
            ogt_vox_group new_group;
            new_group.hidden = false;
            new_group.layer_index = 0;
            new_group.transform = ogt_vox_transform_get_identity();
            new_group.parent_group_index = k_invalid_group_index;
            clear_anim_transform(&new_group.transform_anim);
            new_group.name = 0;
            groups.push_back(new_group);
        }

        // if we didn't get a layer chunk -- just create a default layer.
        if (layers.size() == 0) {
            // go through all instances and ensure they are only mapped to layer 0
            for (uint32_t i = 0; i < instances.size(); i++)
                instances[i].layer_index = 0;
            // add a single layer
            ogt_vox_layer new_layer;
            new_layer.hidden = false;
            new_layer.name   = NULL;
            layers.push_back(new_layer);
        }

        // To support index-level assumptions (eg. artists using top 16 colors for color/palette cycling,
        // other ranges for emissive etc), we must ensure the order of colors that the artist sees in the
        // magicavoxel tool matches the actual index we'll end up using here. Unfortunately, magicavoxel
        // does an unexpected thing when remapping colors in the editor using ctrl+drag within the palette.
        // Instead of remapping all indices in all models, it just keeps track of a display index to actual
        // palette map and uses that to show reordered colors in the palette window. This is how that
        // map works:
        //   displaycolor[k] = paletteColor[imap[k]]
        // To ensure our indices are in the same order as displayed by magicavoxel within the palette
        // window, we apply the mapping from the IMAP chunk both to the color palette and indices within each
        // voxel model.
        if (found_index_map_chunk)
        {
            // the imap chunk maps from display index to actual index.
            // generate an inverse index map (maps from actual index to display index)
            uint8_t index_map_inverse[256];
            for (uint32_t i = 0; i < 256; i++) {
                index_map_inverse[index_map[i]] = (uint8_t)i;
            }

            // reorder colors in the palette so the palette contains colors in display order
            ogt_vox_palette old_palette = palette;
            for (uint32_t i = 0; i < 256; i++) {
                uint32_t remapped_index = (index_map[i] + 255) & 0xFF;
                palette.color[i] = old_palette.color[remapped_index];
            }

            // reorder materials
            ogt_vox_matl_array old_materials = materials;
            for (uint32_t i = 0; i < 256; i++) {
                uint32_t remapped_i = (i + 255) & 0xFF;
                uint32_t remapped_index = index_map[remapped_i];
                materials.matl[i] = old_materials.matl[remapped_index];
            }

            // ensure that all models are remapped so they are using display order palette indices.
            for (uint32_t i = 0; i < model_ptrs.size(); i++) {
                ogt_vox_model* model = model_ptrs[i];
                if (model) {
                    uint32_t num_voxels = model->size_x * model->size_y * model->size_z;
                    uint8_t* voxels = (uint8_t*)&model[1];
                    for (uint32_t j = 0; j < num_voxels; j++)
                        voxels[j] = 1 + index_map_inverse[voxels[j]];
                }
            }
        }

        // rotate the scene palette now so voxel indices can just map straight into the palette
        {
            ogt_vox_rgba last_color = palette.color[255];
            for (uint32_t i = 255; i > 0; i--)
                palette.color[i] = palette.color[i - 1];
            palette.color[0] = last_color;
            palette.color[0].a = 0;  // alpha is zero for the 0th color as that color index represents a transparent voxel.
        }

        // check for models that are identical by doing a pair-wise compare. If we find identical
        // models, we'll end up with NULL gaps in the model_ptrs array, but instances will have
        // been remapped to keep the earlier model.
        if (0 == (read_flags & k_read_scene_flags_keep_duplicate_models)) {
            for (uint32_t i = 0; i < model_ptrs.size(); i++) {
                if (!model_ptrs[i])
                    continue;
                for (uint32_t j = i+1; j < model_ptrs.size(); j++) {
                    if (!model_ptrs[j] || !_vox_models_are_equal(model_ptrs[i], model_ptrs[j]))
                        continue;
                    // model i and model j are the same, so free model j and keep model i.
                    _vox_free(model_ptrs[j]);
                    model_ptrs[j] = NULL;
                    // remap all instances that were referring to j to now refer to i.
                    for (uint32_t k = 0; k < instances.size(); k++) {
                        if (instances[k].model_index == j)
                            instances[k].model_index = i;
                        if (instances[k].model_anim.num_keyframes) {
                            ogt_vox_keyframe_model* keyframes = misc_data.get_ptr<ogt_vox_keyframe_model>((size_t)instances[k].model_anim.keyframes);
                            for (uint32_t f = 0; f < instances[k].model_anim.num_keyframes; f++) {
                                if (keyframes[f].model_index == j)
                                    keyframes[f].model_index = i;
                            }
                        }
                    }
                }
            }
        }

        // sometimes a model can be created which has no solid voxels within just due to the
        // authoring flow within magicavoxel. We have already have prevented creation of
        // instances that refer to empty models, but here we want to compact the model_ptrs
        // array such that it contains no more NULL models. This also requires we remap the
        // indices for instances so they continue to refer to their correct models.
        if (0 == (read_flags & k_read_scene_flags_keep_empty_models_instances))
        {
            // first, check to see if we find any empty model. No need to do work otherwise.
            bool found_empty_model = false;
            for (uint32_t i = 0; i < model_ptrs.size() && !found_empty_model; i++) {
                if (model_ptrs[i] == NULL)
                    found_empty_model = true;
            }
            if (found_empty_model) {
                // build a remap table for all instances and simultaneously compact the model_ptrs array.
                uint32_t* model_remap = (uint32_t*)_vox_malloc(model_ptrs.size() * sizeof(uint32_t));
                uint32_t num_output_models = 0;
                for (uint32_t i = 0; i < model_ptrs.size(); i++) {
                    if (model_ptrs[i] != NULL) {
                        model_ptrs[num_output_models] = model_ptrs[i];
                        model_remap[i] = num_output_models;
                        num_output_models++;
                    }
                    else {
                        model_remap[i] = UINT32_MAX;
                    }
                }
                model_ptrs.resize(num_output_models);

                // remap all instances to point to the compacted model index
                for (uint32_t i = 0; i < instances.size(); i++) {
                    uint32_t new_model_index = model_remap[instances[i].model_index];
                    ogt_assert(new_model_index != UINT32_MAX, "invalid model index found in instances array"); // we should have suppressed instances already that point to NULL models.
                    instances[i].model_index = new_model_index;
                }

                // free remap table
                _vox_free(model_remap);
                model_remap = NULL;
            }
        }

        // finally, construct the output scene..
        size_t scene_size = sizeof(ogt_vox_scene) + misc_data.size();
        ogt_vox_scene* scene = (ogt_vox_scene*)_vox_calloc(scene_size);
        {
            // copy name data into the scene
            char* scene_misc_data = (char*)&scene[1];
            memcpy(scene_misc_data, misc_data.get_ptr<char>(0), sizeof(char) * misc_data.size());

            // copy instances over to scene
            size_t num_scene_instances = instances.size();
            ogt_vox_instance* scene_instances = (ogt_vox_instance*)_vox_malloc(sizeof(ogt_vox_instance) * num_scene_instances);
            if (num_scene_instances) {
                memcpy(scene_instances, &instances[0], sizeof(ogt_vox_instance) * num_scene_instances);
            }
            scene->instances = scene_instances;
            scene->num_instances = (uint32_t)instances.size();

            // copy cameras over to scene
            size_t num_scene_cameras = cameras.size();
            ogt_vox_cam* scene_cameras = (ogt_vox_cam*)_vox_malloc(sizeof(ogt_vox_cam) * num_scene_cameras);
            if (num_scene_cameras) {
                memcpy(scene_cameras, &cameras[0], sizeof(ogt_vox_cam) * num_scene_cameras);
            }
            scene->cameras = scene_cameras;
            scene->num_cameras = (uint32_t)cameras.size();

            // copy model pointers over to the scene,
            size_t num_scene_models = model_ptrs.size();
            ogt_vox_model** scene_models = (ogt_vox_model * *)_vox_malloc(sizeof(ogt_vox_model*) * num_scene_models);
            if (num_scene_models)
                memcpy(scene_models, &model_ptrs[0], sizeof(ogt_vox_model*) * num_scene_models);
            scene->models     = (const ogt_vox_model **)scene_models;
            scene->num_models = (uint32_t)num_scene_models;

            // copy layer pointers over to the scene
            size_t num_scene_layers = layers.size();
            ogt_vox_layer* scene_layers = (ogt_vox_layer*)_vox_malloc(sizeof(ogt_vox_layer) * num_scene_layers);
            memcpy(scene_layers, &layers[0], sizeof(ogt_vox_layer) * num_scene_layers);
            scene->layers     = scene_layers;
            scene->num_layers = (uint32_t)num_scene_layers;

            // copy group pointers over to the scene
            size_t num_scene_groups = groups.size();
            ogt_vox_group* scene_groups = num_scene_groups ? (ogt_vox_group*)_vox_malloc(sizeof(ogt_vox_group) * num_scene_groups) : NULL;
            if (num_scene_groups)
                memcpy(scene_groups, &groups[0], sizeof(ogt_vox_group)* num_scene_groups);
            scene->groups     = scene_groups;
            scene->num_groups = (uint32_t)num_scene_groups;

            // now patch up group name pointers to point into the scene string area
            for (uint32_t i = 0; i < num_scene_groups; i++) {
                if (scene_groups[i].name)
                    scene_groups[i].name = scene_misc_data + (size_t)scene_groups[i].name;
                if (scene_groups[i].transform_anim.num_keyframes)
                    scene_groups[i].transform_anim.keyframes = (const ogt_vox_keyframe_transform*)(scene_misc_data + (size_t)scene_groups[i].transform_anim.keyframes);
            }

            // now patch up instance name pointers to point into the scene string area
            for (uint32_t i = 0; i < num_scene_instances; i++) {
                if (scene_instances[i].name)
                    scene_instances[i].name = scene_misc_data + (size_t)scene_instances[i].name;
                if (scene_instances[i].transform_anim.num_keyframes)
                    scene_instances[i].transform_anim.keyframes = (const ogt_vox_keyframe_transform*)(scene_misc_data + (size_t)scene_instances[i].transform_anim.keyframes);
                if (scene_instances[i].model_anim.num_keyframes)
                    scene_instances[i].model_anim.keyframes  = (const ogt_vox_keyframe_model*)(scene_misc_data + (size_t)scene_instances[i].model_anim.keyframes);
            }

            // now patch up layer name pointers to point into the scene string area
            for (uint32_t i = 0; i < num_scene_layers; i++)
                if (scene_layers[i].name)
                    scene_layers[i].name = scene_misc_data + (size_t)scene_layers[i].name;

            // copy the palette.
            scene->palette = palette;

            // copy the materials.
            scene->materials = materials;
        }

        if (g_progress_callback_func) {
            // we indicate progress as complete, but don't check for cancel as finished
            g_progress_callback_func(1.0f, g_progress_callback_user_data);
        }
        return scene;
    }

    const ogt_vox_scene* ogt_vox_read_scene(const uint8_t* buffer, uint32_t buffer_size) {
        return ogt_vox_read_scene_with_flags(buffer, buffer_size, 0);
    }

    void ogt_vox_destroy_scene(const ogt_vox_scene * _scene) {
        ogt_vox_scene* scene = const_cast<ogt_vox_scene*>(_scene);
        // free models from model array
        for (uint32_t i = 0; i < scene->num_models; i++)
            _vox_free((void*)scene->models[i]);
        // free model array itself
        if (scene->models) {
            _vox_free(scene->models);
            scene->models = NULL;
        }
        // free instance array
        if (scene->instances) {
            _vox_free(const_cast<ogt_vox_instance*>(scene->instances));
            scene->instances = NULL;
        }
        // free cameras array
        if (scene->cameras) {
            _vox_free(const_cast<ogt_vox_cam*>(scene->cameras));
            scene->cameras = NULL;
        }
        // free layer array
        if (scene->layers) {
            _vox_free(const_cast<ogt_vox_layer*>(scene->layers));
            scene->layers = NULL;
        }
        // free groups array
        if (scene->groups) {
            _vox_free(const_cast<ogt_vox_group*>(scene->groups));
            scene->groups = NULL;
        }
        // finally, free the scene.
        _vox_free(scene);
    }

    // the vector should be a unit vector aligned along one of the cardinal directions exactly. eg. (1,0,0) or (0, 0, -1)
    // this function returns the non-zero column index in out_index and the returns whether that entry is negative.
    static bool _vox_get_vec3_rotation_bits(const vec3& vec, uint8_t& out_index) {
        const float* f = &vec.x;
        out_index = 3;
        bool is_negative = false;
        for (uint8_t i = 0; i < 3; i++) {
            if (f[i] == 1.0f || f[i] == -1.0f) {
                out_index = i;
                is_negative = f[i] < 0.0f ? true : false;
            }
            else {
                ogt_assert(f[i] == 0.0f, "rotation vector should contain only 0.0f, 1.0f, or -1.0f");
            }
        }
        ogt_assert(out_index != 3, "rotation vector was all zeroes but it should be a cardinal axis vector");
        return is_negative;
    }

    static uint8_t _vox_make_packed_rotation_from_transform(const ogt_vox_transform * transform) {
        // magicavoxel stores rows, and we have columns, so we do the swizzle here into rows
        vec3 row0 = vec3_make(transform->m00, transform->m10, transform->m20);
        vec3 row1 = vec3_make(transform->m01, transform->m11, transform->m21);
        vec3 row2 = vec3_make(transform->m02, transform->m12, transform->m22);
        uint8_t row0_index = 3, row1_index = 3, row2_index = 3;
        bool row0_negative = _vox_get_vec3_rotation_bits(row0, row0_index);
        bool row1_negative = _vox_get_vec3_rotation_bits(row1, row1_index);
        bool row2_negative = _vox_get_vec3_rotation_bits(row2, row2_index);
        ogt_assert(((1 << row0_index) | (1 << row1_index) | (1 << row2_index)) == 7, "non orthogonal rows found in transform"); // check that rows are orthogonal. There must be a non-zero entry in column 0, 1 and 2 across these 3 rows.
        return (row0_index) | (row1_index << 2) | (row0_negative ? 1 << 4 : 0) | (row1_negative ? 1 << 5 : 0) | (row2_negative ? 1 << 6 : 0);
    }

    struct _vox_file_writeable {
        _vox_array<uint8_t> data;
    };

    static void _vox_file_writeable_init(_vox_file_writeable* fp) {
        fp->data.reserve(1024);
    }
    static void _vox_file_write(_vox_file_writeable* fp, const void* data, uint32_t data_size) {
        fp->data.push_back_many((const uint8_t*)data, data_size);
    }
    static void _vox_file_write_uint32(_vox_file_writeable* fp, uint32_t data) {
        data = _vox_htole32(data);
        _vox_file_write(fp, &data, sizeof(data));
    }
    static void _vox_file_write_uint8(_vox_file_writeable* fp, uint8_t data) {
        _vox_file_write(fp, &data, sizeof(data));
    }
    static void _vox_file_write_uint32_at_offset(_vox_file_writeable* fp, uint32_t offset, const uint32_t* data) {
        ogt_assert((offset + sizeof(*data)) <= fp->data.count, "write at offset must not be an append write");
        const uint32_t val = _vox_htole32(*data);
        memcpy(&fp->data[offset], &val, sizeof(*data));
    }
    static uint32_t _vox_file_get_offset(const _vox_file_writeable* fp) {
        return (uint32_t)fp->data.count;
    }
    static uint8_t* _vox_file_get_data(_vox_file_writeable* fp) {
        return &fp->data[0];
    }
    static void _vox_file_write_dict_key_value(_vox_file_writeable* fp, const char* key, const char* value) {
        if (key == NULL || value == NULL)
            return;
        uint32_t key_len   = (uint32_t)_vox_strlen(key);
        uint32_t value_len = (uint32_t)_vox_strlen(value);
        _vox_file_write_uint32(fp, key_len);
        _vox_file_write(fp, key, key_len);
        _vox_file_write_uint32(fp, value_len);
        _vox_file_write(fp, value, value_len);
    }
    static void _vox_file_write_dict_key_value_uint32(_vox_file_writeable* fp, const char* key, uint32_t value) {
        char value_str[64];
        _vox_sprintf(value_str, sizeof(value_str), "%i", value);
        _vox_file_write_dict_key_value(fp, key, value_str);
    }
    static void _vox_file_write_dict_key_value_float(_vox_file_writeable* fp, const char* key, float value) {
        char value_str[64];
        _vox_sprintf(value_str, sizeof(value_str), "%f", value);
        _vox_file_write_dict_key_value(fp, key, value_str);
    }
    static void _vox_file_write_dict_transform(_vox_file_writeable* fp, const ogt_vox_transform* transform) {
        char t_string[65];
        char r_string[65];
        t_string[0] = 0;
        r_string[0] = 0;
        uint8_t packed_rotation_bits = _vox_make_packed_rotation_from_transform(transform);
        _vox_sprintf(t_string, sizeof(t_string), "%i %i %i", (int32_t)transform->m30, (int32_t)transform->m31, (int32_t)transform->m32);
        _vox_sprintf(r_string, sizeof(r_string), "%u", packed_rotation_bits);
        _vox_file_write_dict_key_value(fp, "_r", r_string);
        _vox_file_write_dict_key_value(fp, "_t", t_string);
    }

    static void _vox_file_write_chunk_nTRN(_vox_file_writeable* fp, uint32_t node_id, uint32_t child_node_id, const char* name, bool hidden, const ogt_vox_transform* transform, uint32_t layer_id, const ogt_vox_anim_transform* transform_anim)
    {
        // obtain dictionary string pointers
        const char* hidden_string = hidden ? "1" : NULL;
        const char* loop_string   = transform_anim->loop ? "1" : NULL;

        uint32_t offset_of_chunk_header = _vox_file_get_offset(fp);

        // write the nTRN header
        _vox_file_write_uint32(fp, CHUNK_ID_nTRN);
        _vox_file_write_uint32(fp, 0); // will get patched up later
        _vox_file_write_uint32(fp, 0);

        // write the nTRN payload
        _vox_file_write_uint32(fp, node_id);

        // write the node dictionary
        uint32_t node_dict_keyvalue_count = (name ? 1 : 0) + (hidden_string ? 1 : 0) + (loop_string ? 1 : 0);
        _vox_file_write_uint32(fp, node_dict_keyvalue_count);  // num key values
        _vox_file_write_dict_key_value(fp, "_name",   name);
        _vox_file_write_dict_key_value(fp, "_hidden", hidden_string);
        _vox_file_write_dict_key_value(fp, "_loop",   loop_string);

        // write other properties.
        _vox_file_write_uint32(fp, child_node_id);
        _vox_file_write_uint32(fp, UINT32_MAX); // reserved_id must have all bits set.
        _vox_file_write_uint32(fp, layer_id);
        if (transform_anim->num_keyframes == 0) {
            _vox_file_write_uint32(fp, 1);  // num_frames must be 1
            // write the frame dictionary
            _vox_file_write_uint32(fp, 2);  // 2 key values: "_r", "_t"
            _vox_file_write_dict_transform(fp, transform);
        }
        else {
            _vox_file_write_uint32(fp, transform_anim->num_keyframes);          // num_frames must be 1
            for (uint32_t j = 0; j < transform_anim->num_keyframes; j++) {
                _vox_file_write_uint32(fp, 3);  // 3 key values: "_r", "_t", "_f"
                _vox_file_write_dict_transform(fp, &transform_anim->keyframes[j].transform);
                _vox_file_write_dict_key_value_uint32(fp, "_f", transform_anim->keyframes[j].frame_index);
            }
        }

        // patch up the chunk size in the header
        uint32_t chunk_size = _vox_file_get_offset(fp) - offset_of_chunk_header - CHUNK_HEADER_LEN;
        _vox_file_write_uint32_at_offset(fp, offset_of_chunk_header + 4, &chunk_size);
    }

    // saves the scene out to a buffer that when saved as a .vox file can be loaded with magicavoxel.
    uint8_t* ogt_vox_write_scene(const ogt_vox_scene* scene, uint32_t* buffer_size) {
        _vox_file_writeable file;
        _vox_file_writeable_init(&file);
        _vox_file_writeable* fp = &file;

        // write file header and file version
        _vox_file_write_uint32(fp, CHUNK_ID_VOX_);
        _vox_file_write_uint32(fp, 150);

        // write the main chunk
        _vox_file_write_uint32(fp, CHUNK_ID_MAIN);
        _vox_file_write_uint32(fp, 0);
        _vox_file_write_uint32(fp, 0);  // this main_chunk_child_size will get patched up once everything is written.

        // we need to know how to patch up the main chunk size after we've written everything
        const uint32_t offset_post_main_chunk = _vox_file_get_offset(fp);

        // write out all model chunks
        for (uint32_t i = 0; i < scene->num_models; i++) {
            const ogt_vox_model* model = scene->models[i];
            ogt_assert(model->size_x <= 256 && model->size_y <= 256 && model->size_z <= 256, "model dimensions exceed the limit of 256x256x256");
            // count the number of solid voxels in the grid
            uint32_t num_voxels_in_grid = model->size_x * model->size_y * model->size_z;
            uint32_t num_solid_voxels = 0;
            for (uint32_t voxel_index = 0; voxel_index < num_voxels_in_grid; voxel_index++)
                if (model->voxel_data[voxel_index] != 0)
                    num_solid_voxels++;
            uint32_t chunk_size_xyzi = sizeof(uint32_t) + 4 * num_solid_voxels;

            // write the SIZE chunk header
            _vox_file_write_uint32(fp, CHUNK_ID_SIZE);
            _vox_file_write_uint32(fp, 12);
            _vox_file_write_uint32(fp, 0);

            // write the SIZE chunk payload
            ogt_assert(model->size_x && model->size_y && model->size_z, "model has zero size");
            _vox_file_write_uint32(fp, model->size_x);
            _vox_file_write_uint32(fp, model->size_y);
            _vox_file_write_uint32(fp, model->size_z);

            // write the XYZI chunk header
            _vox_file_write_uint32(fp, CHUNK_ID_XYZI);
            _vox_file_write_uint32(fp, chunk_size_xyzi);
            _vox_file_write_uint32(fp, 0);

            // write out XYZI chunk payload
            _vox_file_write_uint32(fp, num_solid_voxels);
            uint32_t voxel_index = 0;
            for (uint32_t z = 0; z < model->size_z; z++) {
                for (uint32_t y = 0; y < model->size_y; y++) {
                    for (uint32_t x = 0; x < model->size_x; x++, voxel_index++) {
                        uint8_t color_index = model->voxel_data[voxel_index];
                        if (color_index != 0) {
                            _vox_file_write_uint8(fp, (uint8_t)x);
                            _vox_file_write_uint8(fp, (uint8_t)y);
                            _vox_file_write_uint8(fp, (uint8_t)z);
                            _vox_file_write_uint8(fp, color_index);
                        }
                    }
                }
            }

            if (g_progress_callback_func) {
                // we indicate progress as number of models written, with an extra progress value for ending write
                if (!g_progress_callback_func((float)(i + 1)/(float)(scene->num_models + 1), g_progress_callback_user_data))
                {
                    *buffer_size = 0;
                    return NULL; // note: fp will be freed in dtor on exit
                }
            }
        }

        // define our node_id ranges.
        ogt_assert(scene->num_groups > 0, "no groups found in scene");
        uint32_t first_group_transform_node_id    = 0;
        uint32_t first_group_node_id              = first_group_transform_node_id + scene->num_groups;
        uint32_t first_shape_node_id              = first_group_node_id + scene->num_groups;
        uint32_t first_instance_transform_node_id = first_shape_node_id + scene->num_instances;

        // write the nTRN nodes for each of the groups in the scene.
        for (uint32_t group_index = 0; group_index < scene->num_groups; group_index++) {
            const ogt_vox_group* group = &scene->groups[group_index];
            _vox_file_write_chunk_nTRN(fp, first_group_transform_node_id + group_index, first_group_node_id + group_index, group->name, group->hidden, &group->transform, group->layer_index, &group->transform_anim);
        }
        // write the group nodes for each of the groups in the scene
        for (uint32_t group_index = 0; group_index < scene->num_groups; group_index++) {
            // count how many childnodes  there are. This is simply the sum of all
            // groups and instances that have this group as its parent
            uint32_t num_child_nodes = 0;
            for (uint32_t child_group_index = 0; child_group_index < scene->num_groups; child_group_index++)
                if (scene->groups[child_group_index].parent_group_index == group_index)
                    num_child_nodes++;
            for (uint32_t child_instance_index = 0; child_instance_index < scene->num_instances; child_instance_index++)
                if (scene->instances[child_instance_index].group_index == group_index)
                    num_child_nodes++;

            // count number of dictionary items
            const char* hidden_string = scene->groups[group_index].hidden ? "1" : NULL;
            uint32_t group_dict_keyvalue_count = (hidden_string ? 1 : 0);

            // compute the chunk size.
            uint32_t offset_of_chunk_header = _vox_file_get_offset(fp);

            // write the nGRP header
            _vox_file_write_uint32(fp, CHUNK_ID_nGRP);
            _vox_file_write_uint32(fp, 0); // chunk_size will get patched up after.
            _vox_file_write_uint32(fp, 0);
            // write the nGRP payload
            _vox_file_write_uint32(fp, first_group_node_id + group_index);       // node_id
            _vox_file_write_uint32(fp, group_dict_keyvalue_count); // num keyvalue pairs in node dictionary
            _vox_file_write_dict_key_value(fp, "_hidden", hidden_string);
            _vox_file_write_uint32(fp, num_child_nodes);
            // write the child group transform nodes
            for (uint32_t child_group_index = 0; child_group_index < scene->num_groups; child_group_index++)
                if (scene->groups[child_group_index].parent_group_index == group_index)
                    _vox_file_write_uint32(fp, first_group_transform_node_id + child_group_index);
            // write the child instance transform nodes
            for (uint32_t child_instance_index = 0; child_instance_index < scene->num_instances; child_instance_index++)
                if (scene->instances[child_instance_index].group_index == group_index)
                    _vox_file_write_uint32(fp, first_instance_transform_node_id + child_instance_index);

            uint32_t chunk_size = _vox_file_get_offset(fp) - offset_of_chunk_header - CHUNK_HEADER_LEN;
            _vox_file_write_uint32_at_offset(fp, offset_of_chunk_header + 4, &chunk_size);
        }

        // write out an nSHP chunk for each of the instances
        for (uint32_t i = 0; i < scene->num_instances; i++) {
            const ogt_vox_instance* instance = &scene->instances[i];

            uint32_t offset_of_chunk_header = _vox_file_get_offset(fp);
            // write the nSHP chunk header
            _vox_file_write_uint32(fp, CHUNK_ID_nSHP);
            _vox_file_write_uint32(fp, 0); // will get patched up at the end
            _vox_file_write_uint32(fp, 0);
            // write the nSHP chunk payload
            _vox_file_write_uint32(fp, first_shape_node_id + i);    // node_id

            // write the nSHP node dictionary
            const char* loop_string = instance->model_anim.loop ? "1" : NULL;
            uint32_t node_dict_keyvalue_count = (loop_string ? 1 : 0);
            _vox_file_write_uint32(fp, node_dict_keyvalue_count);  // num key values
            _vox_file_write_dict_key_value(fp, "_loop",   loop_string);

            if (instance->model_anim.num_keyframes == 0 ) {
                _vox_file_write_uint32(fp, 1);                      // num_models must be 1
                _vox_file_write_uint32(fp, instance->model_index);  // model_id
                _vox_file_write_uint32(fp, 0);                      // num keyvalue pairs in model dictionary
            }
            else {
                _vox_file_write_uint32(fp, instance->model_anim.num_keyframes);
                for (uint32_t j = 0; j < instance->model_anim.num_keyframes; j++) {
                    _vox_file_write_uint32(fp, instance->model_anim.keyframes[j].model_index); // model_id
                    _vox_file_write_uint32(fp, 1); // num keyvalue pairs in model dictionary
                    _vox_file_write_dict_key_value_uint32(fp, "_f", instance->model_anim.keyframes[j].frame_index);
                }
            }
            // compute and patch up the chunk size in the chunk header
            uint32_t chunk_size = _vox_file_get_offset(fp) - offset_of_chunk_header - CHUNK_HEADER_LEN;
            _vox_file_write_uint32_at_offset(fp, offset_of_chunk_header + 4, &chunk_size);
        }

        // write out a nTRN chunk for all instances - and make them point to the relevant nSHP chunk
        for (uint32_t i = 0; i < scene->num_instances; i++) {
            const ogt_vox_instance* instance = &scene->instances[i];
            uint32_t node_id       = first_instance_transform_node_id + i;
            uint32_t child_node_id = first_shape_node_id + i;
            _vox_file_write_chunk_nTRN(fp, node_id, child_node_id, instance->name, instance->hidden, &instance->transform, instance->layer_index, &instance->transform_anim);
        }

        // write out the rCAM chunks
        for (uint32_t i = 0; i < scene->num_cameras; i++) {
            const ogt_vox_cam* camera = &scene->cameras[i];
            char cam_focus[64] = "";
            char cam_angle[64] = "";
            char cam_radius[32] = "";
            char cam_frustum[32] = "";
            char cam_fov[32] = "";
            const char *cam_mode;
            _vox_sprintf(cam_focus, sizeof(cam_focus), "%.5f %.5f %.5f", camera->focus[0], camera->focus[1], camera->focus[2]);
            _vox_sprintf(cam_angle, sizeof(cam_angle), "%.5f %.5f %.5f", camera->angle[0], camera->angle[1], camera->angle[2]);
            _vox_sprintf(cam_radius, sizeof(cam_radius), "%.5f", camera->radius);
            _vox_sprintf(cam_frustum, sizeof(cam_frustum), "%.5f", camera->frustum);
            _vox_sprintf(cam_fov, sizeof(cam_fov), "%i", camera->fov);

            switch (camera->mode) {
            case ogt_cam_mode_free:
                cam_mode = "free";
                break;
            case ogt_cam_mode_pano:
                cam_mode = "pano";
                break;
            case ogt_cam_mode_isometric:
                cam_mode = "iso";
                break;
            case ogt_cam_mode_orthographic:
                cam_mode = "orth";
                break;
            default:
            case ogt_cam_mode_unknown:
            case ogt_cam_mode_perspective:
                cam_mode = "pers";
                break;
            }

            uint32_t offset_of_chunk_header = _vox_file_get_offset(fp);

            // write the rCAM header
            _vox_file_write_uint32(fp, CHUNK_ID_rCAM);
            _vox_file_write_uint32(fp, 0); // chunk_size will get patched up later
            _vox_file_write_uint32(fp, 0);

            _vox_file_write_uint32(fp, camera->camera_id);
            _vox_file_write_uint32(fp, 6);  // num key values
            _vox_file_write_dict_key_value(fp, "_mode", cam_mode);
            _vox_file_write_dict_key_value(fp, "_focus", cam_focus);
            _vox_file_write_dict_key_value(fp, "_angle", cam_angle);
            _vox_file_write_dict_key_value(fp, "_radius", cam_radius);
            _vox_file_write_dict_key_value(fp, "_frustum", cam_frustum);
            _vox_file_write_dict_key_value(fp, "_fov", cam_fov);

            // compute and patch up the chunk size in the chunk header
            uint32_t chunk_size = _vox_file_get_offset(fp) - offset_of_chunk_header - CHUNK_HEADER_LEN;
            _vox_file_write_uint32_at_offset(fp, offset_of_chunk_header + 4, &chunk_size);
        }

        // write out RGBA chunk for the palette
        {
            // .vox stores palette rotated by 1 color index, so do that now.
            ogt_vox_palette rotated_palette;
            for (uint32_t i = 0; i < 256; i++)
                rotated_palette.color[i] = scene->palette.color[(i + 1) & 255];

            // write the palette chunk header
            _vox_file_write_uint32(fp, CHUNK_ID_RGBA);
            _vox_file_write_uint32(fp, sizeof(ogt_vox_palette));
            _vox_file_write_uint32(fp, 0);
            // write the palette chunk payload
            _vox_file_write(fp, &rotated_palette, sizeof(ogt_vox_palette));
        }

        // write out MATL chunk
        {
            // keep in sync with ogt_matl_type
            static const char *type_str[] = {"_diffuse", "_metal", "_glass", "_emit", "_blend", "_media"};

            for (int32_t i = 0; i < 256; ++i) {
                const ogt_vox_matl &matl = scene->materials.matl[i];
                if (matl.content_flags == 0u) {
                    continue;
                }
                uint32_t matl_dict_keyvalue_count = 1;
                matl_dict_keyvalue_count += (matl.content_flags & k_ogt_vox_matl_have_metal) ? 1 : 0;
                matl_dict_keyvalue_count += (matl.content_flags & k_ogt_vox_matl_have_rough) ? 1 : 0;
                matl_dict_keyvalue_count += (matl.content_flags & k_ogt_vox_matl_have_spec)  ? 1 : 0;
                matl_dict_keyvalue_count += (matl.content_flags & k_ogt_vox_matl_have_ior)   ? 1 : 0;
                matl_dict_keyvalue_count += (matl.content_flags & k_ogt_vox_matl_have_att)   ? 1 : 0;
                matl_dict_keyvalue_count += (matl.content_flags & k_ogt_vox_matl_have_flux)  ? 1 : 0;
                matl_dict_keyvalue_count += (matl.content_flags & k_ogt_vox_matl_have_emit)  ? 1 : 0;
                matl_dict_keyvalue_count += (matl.content_flags & k_ogt_vox_matl_have_ldr)   ? 1 : 0;
                matl_dict_keyvalue_count += (matl.content_flags & k_ogt_vox_matl_have_trans) ? 1 : 0;
                matl_dict_keyvalue_count += (matl.content_flags & k_ogt_vox_matl_have_alpha) ? 1 : 0;
                matl_dict_keyvalue_count += (matl.content_flags & k_ogt_vox_matl_have_d)     ? 1 : 0;
                matl_dict_keyvalue_count += (matl.content_flags & k_ogt_vox_matl_have_sp)    ? 1 : 0;
                matl_dict_keyvalue_count += (matl.content_flags & k_ogt_vox_matl_have_g)     ? 1 : 0;
                matl_dict_keyvalue_count += (matl.content_flags & k_ogt_vox_matl_have_media) ? 1 : 0;

                uint32_t offset_of_chunk_header = _vox_file_get_offset(fp);

                // write the material chunk header
                _vox_file_write_uint32(fp, CHUNK_ID_MATL);
                _vox_file_write_uint32(fp, 0); // chunk_size will get patched up later
                _vox_file_write_uint32(fp, 0);

                _vox_file_write_uint32(fp, i); // material id
                _vox_file_write_uint32(fp, matl_dict_keyvalue_count);
                _vox_file_write_dict_key_value(fp, "_type", type_str[matl.type]);
                if (matl.content_flags & k_ogt_vox_matl_have_metal) {
                    _vox_file_write_dict_key_value_float(fp, "_metal", matl.metal);
                }
                if (matl.content_flags & k_ogt_vox_matl_have_rough) {
                    _vox_file_write_dict_key_value_float(fp, "_rough", matl.rough);
                }
                if (matl.content_flags & k_ogt_vox_matl_have_spec) {
                    _vox_file_write_dict_key_value_float(fp, "_spec", matl.spec);
                }
                if (matl.content_flags & k_ogt_vox_matl_have_ior) {
                    _vox_file_write_dict_key_value_float(fp, "_ior", matl.ior);
                }
                if (matl.content_flags & k_ogt_vox_matl_have_att) {
                    _vox_file_write_dict_key_value_float(fp, "_att", matl.att);
                }
                if (matl.content_flags & k_ogt_vox_matl_have_flux) {
                    _vox_file_write_dict_key_value_float(fp, "_flux", matl.flux);
                }
                if (matl.content_flags & k_ogt_vox_matl_have_emit) {
                    _vox_file_write_dict_key_value_float(fp, "_emit", matl.emit);
                }
                if (matl.content_flags & k_ogt_vox_matl_have_ldr) {
                    _vox_file_write_dict_key_value_float(fp, "_ldr", matl.ldr);
                }
                if (matl.content_flags & k_ogt_vox_matl_have_trans) {
                    _vox_file_write_dict_key_value_float(fp, "_trans", matl.trans);
                }
                if (matl.content_flags & k_ogt_vox_matl_have_alpha) {
                    _vox_file_write_dict_key_value_float(fp, "_alpha", matl.alpha);
                }
                if (matl.content_flags & k_ogt_vox_matl_have_d) {
                    _vox_file_write_dict_key_value_float(fp, "_d", matl.d);
                }
                if (matl.content_flags & k_ogt_vox_matl_have_sp) {
                    _vox_file_write_dict_key_value_float(fp, "_sp", matl.sp);
                }
                if (matl.content_flags & k_ogt_vox_matl_have_g) {
                    _vox_file_write_dict_key_value_float(fp, "_g", matl.g);
                }
                if (matl.content_flags & k_ogt_vox_matl_have_media) {
                    _vox_file_write_dict_key_value_float(fp, "_media", matl.media);
                }
                // compute and patch up the chunk size in the chunk header
                uint32_t chunk_size = _vox_file_get_offset(fp) - offset_of_chunk_header - CHUNK_HEADER_LEN;
                _vox_file_write_uint32_at_offset(fp, offset_of_chunk_header + 4, &chunk_size);
            }
        }

        // write all layer chunks out.
        for (uint32_t i = 0; i < scene->num_layers; i++) {
            char color_string[64];
            _vox_sprintf(color_string, sizeof(color_string), "%u %u %u", scene->layers[i].color.r, scene->layers[i].color.g, scene->layers[i].color.b);
            const char* layer_name_string = scene->layers[i].name;
            const char* hidden_string = scene->layers[i].hidden ? "1" : NULL;
            uint32_t layer_dict_keyvalue_count = 0;
            layer_dict_keyvalue_count += (layer_name_string ? 1 : 0);
            layer_dict_keyvalue_count += (hidden_string ? 1 : 0);
            layer_dict_keyvalue_count += 1; // color_string

            uint32_t offset_of_chunk_header = _vox_file_get_offset(fp);

            // write the layer chunk header
            _vox_file_write_uint32(fp, CHUNK_ID_LAYR);
            _vox_file_write_uint32(fp, 0); // chunk_size will get patched up later
            _vox_file_write_uint32(fp, 0);
            // write the layer chunk payload
            _vox_file_write_uint32(fp, i);                          // layer_id
            _vox_file_write_uint32(fp, layer_dict_keyvalue_count);  // num keyvalue pairs in layer dictionary
            _vox_file_write_dict_key_value(fp, "_name",   layer_name_string);
            _vox_file_write_dict_key_value(fp, "_hidden", hidden_string);
            _vox_file_write_dict_key_value(fp, "_color",  color_string);
            _vox_file_write_uint32(fp, UINT32_MAX);                 // reserved id

            // compute and patch up the chunk size in the chunk header
            uint32_t chunk_size = _vox_file_get_offset(fp) - offset_of_chunk_header - CHUNK_HEADER_LEN;
            _vox_file_write_uint32_at_offset(fp, offset_of_chunk_header + 4, &chunk_size);
        }

        // check that the buffer is not larger than the maximum file size, return nothing if would overflow
        if (fp->data.count > UINT32_MAX ||  (fp->data.count - offset_post_main_chunk) > UINT32_MAX)
        {
            ogt_assert(0, "Generated file size exceeded 4GiB, which is too large for Magicavoxel to parse.");
            *buffer_size = 0;
            return NULL;  // note: fp will be freed in dtor on exit
        }

        // we deliberately don't free the fp->data field, just pass the buffer pointer and size out to the caller
        *buffer_size = (uint32_t)fp->data.count;
        uint8_t* buffer_data = _vox_file_get_data(fp);
        // we deliberately clear this pointer so it doesn't get auto-freed on exiting. The caller will own the memory hereafter.
        fp->data.data = NULL;

        // patch up the main chunk's child chunk size now that we've written everything we're going to write.
        {
            uint32_t* main_chunk_child_size = (uint32_t*)& buffer_data[offset_post_main_chunk - sizeof(uint32_t)];
            *main_chunk_child_size = *buffer_size - offset_post_main_chunk;
        }

        if (g_progress_callback_func) {
            // we indicate progress as number of models written, with an extra progress value for ending write
            g_progress_callback_func(1.0f,g_progress_callback_user_data); // we ignore the return as exiting here anyway
        }

        return buffer_data;
    }

    void* ogt_vox_malloc(size_t size) {
        return _vox_malloc(size);
    }

    void ogt_vox_free(void* mem) {
        _vox_free(mem);
    }

    // compute the minimum and maximum x coordinate within the scene.
    static void compute_scene_bounding_box_x(const ogt_vox_scene * scene, int32_t & out_min_x, int32_t & out_max_x) {
        if (scene->num_instances && scene->num_models)
        {
            // We don't apply orientation to the model dimensions and compute the exact min/max.
            // Instead we just conservatively use the maximum dimension of the model.
            int32_t scene_min_x =  0x7ffffff;
            int32_t scene_max_x = -0x7ffffff;
            for (uint32_t instance_index = 0; instance_index < scene->num_instances; instance_index++) {
                const ogt_vox_instance* instance = &scene->instances[instance_index];
                // compute the instance transform, taking into account the group hierarchy.
                ogt_vox_transform instance_transform = instance->transform;
                uint32_t parent_group_index = instance->group_index;
                while (parent_group_index != k_invalid_group_index) {
                    const ogt_vox_group* group = &scene->groups[parent_group_index];
                    instance_transform = ogt_vox_transform_multiply(instance_transform, group->transform);
                    parent_group_index = group->parent_group_index;
                }

                const ogt_vox_model* model = scene->models[instance->model_index];
                // the instance_transform can be rotated, so we try to figure out whether the
                // model's local x, y or z size is aligned along the world x axis.
                // One of the column vectors of the transform must have a non-zero in its
                // x field and the dimension associated with that column is the correct choice of rus.
                int32_t max_dim = instance_transform.m00 != 0.0f ? model->size_x :
                                  instance_transform.m10 != 0.0f ? model->size_y :
                                  instance_transform.m20 != 0.0f ? model->size_z : model->size_x;
                int32_t half_dim = max_dim / 2;
                int32_t min_x = (int32_t)instance_transform.m30 - half_dim;
                int32_t max_x = (int32_t)instance_transform.m30 + half_dim;
                scene_min_x = min_x < scene_min_x ? min_x : scene_min_x;
                scene_max_x = max_x > scene_max_x ? max_x : scene_max_x;
            }
            // pass out the dimensions.
            out_min_x = scene_min_x;
            out_max_x = scene_max_x;
        }
        else {
            out_min_x = 0;
            out_max_x = 0;
        }
    }

    // returns a mask of which color indices are used by the specified scene.
    // used_mask[0] can be false at the end of this if all models 100% fill their voxel grid with solid voxels, so callers
    // should handle that case properly.
    static void compute_scene_used_color_index_mask(bool* used_mask, const ogt_vox_scene * scene) {
        memset(used_mask, 0, 256);
        for (uint32_t model_index = 0; model_index < scene->num_models; model_index++) {
            const ogt_vox_model* model = scene->models[model_index];
            uint32_t voxel_count = model->size_x * model->size_y * model->size_z;
            for (uint32_t voxel_index = 0; voxel_index < voxel_count; voxel_index++) {
                uint8_t color_index = model->voxel_data[voxel_index];
                used_mask[color_index] = true;
            }
        }
    }

    // finds an exact color in the specified palette if it exists, and UINT32_MAX otherwise
    static uint32_t find_exact_color_in_palette(const ogt_vox_rgba * palette, uint32_t palette_count, const ogt_vox_rgba color_to_find) {
        for (uint32_t color_index = 1; color_index < palette_count; color_index++) {
            const ogt_vox_rgba color_to_match = palette[color_index];
            // we only try to match r,g,b components exactly.
            if (color_to_match.r == color_to_find.r && color_to_match.g == color_to_find.g && color_to_match.b == color_to_find.b)
                return color_index;
        }
        // no exact color found
        return UINT32_MAX;
    }

    // finds the index within the specified palette that is closest to the color we want to find
    static uint32_t find_closest_color_in_palette(const ogt_vox_rgba * palette, uint32_t palette_count, const ogt_vox_rgba color_to_find) {
        // the lower the score the better, so initialize this to the maximum possible score
        int32_t  best_score = INT32_MAX;
        uint32_t best_index = 1;
        // Here we compute a score based on the pythagorean distance between each color in the palette and the color to find.
        // The distance is in R,G,B space, and we choose the color with the lowest score.
        for (uint32_t color_index = 1; color_index < palette_count; color_index++) {
            int32_t r_diff = (int32_t)color_to_find.r - (int32_t)palette[color_index].r;
            int32_t g_diff = (int32_t)color_to_find.g - (int32_t)palette[color_index].g;
            int32_t b_diff = (int32_t)color_to_find.b - (int32_t)palette[color_index].b;
            // There are 2 aspects of our treatment of color here you may want to experiment with:
            // 1. differences in R, differences in G, differences in B are weighted the same rather than perceptually. Different weightings may be better for you.
            // 2. We treat R,G,B as if they are in a perceptually linear within each channel. eg. the differences between
            //    a value of 5 and 8 in any channel is perceptually the same as the difference between 233 and 236 in the same channel.
            int32_t score = (r_diff * r_diff) + (g_diff * g_diff) + (b_diff * b_diff);
            if (score < best_score) {
                best_score = score;
                best_index = color_index;
            }
        }
        ogt_assert(best_score < INT32_MAX, "degenerate palette"); // this might indicate a completely degenerate palette.
        return best_index;
    }

    static void update_master_palette_and_materials_from_scene(ogt_vox_rgba * master_palette, uint32_t & master_palette_count, const ogt_vox_scene * scene, uint32_t * scene_to_master_map, ogt_vox_matl * master_matl) {
        // compute the mask of used colors in the scene.
        bool scene_used_mask[256];
        compute_scene_used_color_index_mask(scene_used_mask, scene);

        // initialize the map that converts from scene color_index to master color_index
        scene_to_master_map[0] = 0;              // zero/empty always maps to zero/empty in the master palette
        for (uint32_t i = 1; i < 256; i++)
            scene_to_master_map[i] = UINT32_MAX; // UINT32_MAX means unassigned

        // for each used color in the scene, now allocate it into the master palette.
        for (uint32_t color_index = 1; color_index < 256; color_index++) {
            if (scene_used_mask[color_index]) {
                const ogt_vox_rgba color = scene->palette.color[color_index];
                const ogt_vox_matl matl = scene->materials.matl[color_index];
                // find the exact color in the master palette. Will be UINT32_MAX if the color doesn't already exist
                uint32_t master_index = find_exact_color_in_palette(master_palette, master_palette_count, color);
                if (master_index == UINT32_MAX) {
                    if (master_palette_count < 256) {
                        // master palette capacity hasn't been exceeded so far, allocate the color to it.
                        master_palette[master_palette_count] = color;
                        master_matl[master_palette_count] = matl;
                        master_index = master_palette_count++;
                    }
                    else {
                        // otherwise, find the color that is perceptually closest to the original color.

                        // TODO(jpaver): It is potentially problematic if we hit this path for a many-scene merge.
                        // Earlier scenes will reserve their colors exactly into the master palette, whereas later
                        // scenes will end up having some of their colors remapped to different colors.

                        // A more holistic approach to color allocation may be necessary here eg.
                        // we might allow the master palette to grow to more than 256 entries, and then use
                        // similarity/frequency metrics to reduce the palette from that down to 256 entries. This
                        // will mean all scenes will have be equally important if they have a high-frequency
                        // usage of a color.
                        master_index = find_closest_color_in_palette(master_palette, master_palette_count, color);
                    }
                }
                // caller needs to know how to map its original color index into the master palette
                scene_to_master_map[color_index] = master_index;
            }
        }
    }

    ogt_vox_scene* ogt_vox_merge_scenes(const ogt_vox_scene** scenes, uint32_t scene_count, const ogt_vox_rgba* required_colors, const uint32_t required_color_count) {
        ogt_assert(required_color_count <= 255, "too many colors specified"); // can't exceed the maximum colors in the master palette plus the empty slot.

        // initialize the master palette. If required colors are specified, map them into the master palette now.
        ogt_vox_rgba  master_palette[256];
        ogt_vox_matl materials[256];
        uint32_t master_palette_count = 1;          // color_index 0 is reserved for empty color!
        memset(&master_palette, 0, sizeof(master_palette));
        memset(&materials, 0, sizeof(materials));
        for (uint32_t required_index = 0; required_index < required_color_count; required_index++)
            master_palette[master_palette_count++] = required_colors[required_index];

        // count the number of required models, instances in the master scene
        uint32_t max_layers = 1;  // we don't actually merge layers. Every instance will be in layer 0.
        uint32_t max_models = 0;
        uint32_t max_instances = 0;
        uint32_t max_groups = 1;  // we add 1 root global group that everything will ultimately be parented to.
        for (uint32_t scene_index = 0; scene_index < scene_count; scene_index++) {
            if (!scenes[scene_index])
                continue;
            max_instances += scenes[scene_index]->num_instances;
            max_models += scenes[scene_index]->num_models;
            max_groups += scenes[scene_index]->num_groups;
        }

        // allocate the master instances array
        ogt_vox_instance* instances     = (ogt_vox_instance*)_vox_malloc(sizeof(ogt_vox_instance) * max_instances);
        ogt_vox_model**   models        = (ogt_vox_model**)_vox_malloc(sizeof(ogt_vox_model*) * max_models);
        ogt_vox_layer*    layers        = (ogt_vox_layer*)_vox_malloc(sizeof(ogt_vox_layer) * max_layers);
        ogt_vox_group*    groups        = (ogt_vox_group*)_vox_malloc(sizeof(ogt_vox_group) * max_groups);
        uint32_t          num_instances = 0;
        uint32_t          num_models    = 0;
        uint32_t          num_layers    = 0;

        // add a single layer.
        layers[num_layers].hidden = false;
        layers[num_layers].color  = {255, 255, 255, 255};
        layers[num_layers].name = "merged";
        num_layers++;

        // magicavoxel expects exactly 1 root group, so if we have multiple scenes with multiple roots,
        // we must ensure all merged scenes are parented to the same root group. Allocate it now for the
        // merged scene.
        uint32_t global_root_group_index = 0;
        {
            ogt_vox_group root_group;
            root_group.name                    = NULL;
            root_group.hidden                  = false;
            root_group.layer_index             = 0;
            root_group.parent_group_index      = k_invalid_group_index;
            root_group.transform               = ogt_vox_transform_get_identity();
            clear_anim_transform(&root_group.transform_anim);
            groups[0] = root_group;
        }
        uint32_t num_groups = 1; // we just wrote the global root group index, so all per-scene groups are allocated after.

        // go ahead and do the merge now!
        _vox_array<uint32_t> per_scene_base_model_index;
        _vox_array<uint32_t> per_scene_base_instance_index;
        size_t misc_data_size = 0;
        int32_t offset_x = 0;
        for (uint32_t scene_index = 0; scene_index < scene_count; scene_index++) {
            const ogt_vox_scene* scene = scenes[scene_index];
            if (!scene)
                continue;

            // update the master palette, and get the map of this scene's color indices into the master palette.
            uint32_t scene_color_index_to_master_map[256];
            update_master_palette_and_materials_from_scene(master_palette, master_palette_count, scene, scene_color_index_to_master_map, materials);

            // cache away the base model index for this scene.
            uint32_t base_model_index = num_models;
            uint32_t base_group_index = num_groups;

            per_scene_base_model_index.push_back(base_model_index);
            per_scene_base_instance_index.push_back(num_instances);

            // create copies of all models that have color indices remapped.
            for (uint32_t model_index = 0; model_index < scene->num_models; model_index++) {
                const ogt_vox_model* model = scene->models[model_index];
                uint32_t voxel_count = model->size_x * model->size_y * model->size_z;
                // clone the model
                ogt_vox_model* override_model = (ogt_vox_model*)_vox_malloc(sizeof(ogt_vox_model) + voxel_count);
                uint8_t * override_voxel_data = (uint8_t*)& override_model[1];

                // remap all color indices in the cloned model so they reference the master palette now!
                for (uint32_t voxel_index = 0; voxel_index < voxel_count; voxel_index++) {
                    uint8_t  old_color_index = model->voxel_data[voxel_index];
                    uint32_t new_color_index = scene_color_index_to_master_map[old_color_index];
                    ogt_assert(new_color_index < 256, "color index out of bounds");
                    override_voxel_data[voxel_index] = (uint8_t)new_color_index;
                }
                // assign the new model.
                *override_model = *model;
                override_model->voxel_data = override_voxel_data;
                override_model->voxel_hash = _vox_hash(override_voxel_data, voxel_count);

                models[num_models++] = override_model;
            }

            // compute the scene bounding box on x dimension. this is used to offset instances
            // and groups in the merged model along X dimension such that they do not overlap
            // with instances from another scene in the merged model.
            int32_t scene_min_x, scene_max_x;
            compute_scene_bounding_box_x(scene, scene_min_x, scene_max_x);
            float scene_offset_x = (float)(offset_x - scene_min_x);

            // each scene has a root group, and it must the 0th group in its local groups[] array,
            ogt_assert(scene->groups[0].parent_group_index == k_invalid_group_index, "first group of scene must not be parented to any other group");
            // create copies of all groups into the merged scene (except the root group from each scene -- which is why we start group_index at 1 here)
            for (uint32_t group_index = 1; group_index < scene->num_groups; group_index++) {
                const ogt_vox_group* src_group = &scene->groups[group_index];
                ogt_assert(src_group->parent_group_index != k_invalid_group_index, "all groups after the first in the scene must be parented to a valid group"); // there can be only 1 root group per scene and it must be the 0th group.
                ogt_vox_group dst_group = *src_group;
                ogt_assert(dst_group.parent_group_index < scene->num_groups, "group index is out of bounds");
                dst_group.layer_index        = 0;
                dst_group.parent_group_index = (dst_group.parent_group_index == 0) ? global_root_group_index : base_group_index + (dst_group.parent_group_index - 1);
                if (dst_group.name)
                    misc_data_size += _vox_strlen(dst_group.name) + 1; // + 1 for zero terminator
                if (dst_group.transform_anim.num_keyframes)
                    misc_data_size += (sizeof(ogt_vox_keyframe_transform) * dst_group.transform_anim.num_keyframes);
                // if this group belongs to the global root group, it must be translated so it doesn't overlap with other scenes.
                if (dst_group.parent_group_index == global_root_group_index)
                    dst_group.transform.m30 += scene_offset_x;
                groups[num_groups++] = dst_group;
            }

            // create copies of all instances (and bias them such that minimum on x starts at zero)
            for (uint32_t instance_index = 0; instance_index < scene->num_instances; instance_index++) {
                const ogt_vox_instance* src_instance = &scene->instances[instance_index];
                ogt_assert(src_instance->group_index < scene->num_groups, "group index is out of bounds");  // every instance must be mapped to a group.
                ogt_vox_instance* dst_instance = &instances[num_instances++];
                *dst_instance = *src_instance;
                dst_instance->layer_index = 0;
                dst_instance->group_index = (dst_instance->group_index == 0) ? global_root_group_index : base_group_index + (dst_instance->group_index - 1);
                dst_instance->model_index += base_model_index;
                if (dst_instance->name)
                    misc_data_size += _vox_strlen(dst_instance->name) + 1; // + 1 for zero terminator
                if (dst_instance->transform_anim.num_keyframes)
                    misc_data_size += (sizeof(ogt_vox_keyframe_transform) * dst_instance->transform_anim.num_keyframes);
                if (dst_instance->model_anim.num_keyframes)
                    misc_data_size += (sizeof(ogt_vox_keyframe_model) * dst_instance->model_anim.num_keyframes);

                // if this instance belongs to the global root group, it must be translated so it doesn't overlap with other scenes.
                if (dst_instance->group_index == global_root_group_index)
                    dst_instance->transform.m30 += scene_offset_x;
            }

            offset_x += (scene_max_x - scene_min_x); // step the width of the scene in x dimension
            offset_x += 4;                           // a margin of this many voxels between scenes
        }

        // fill any unused master palette entries with purple/invalid color.
        const ogt_vox_rgba k_invalid_color = { 255, 0, 255, 255 };  // purple = invalid
        for (uint32_t color_index = master_palette_count; color_index < 256; color_index++)
            master_palette[color_index] = k_invalid_color;

        // assign the master scene on output. misc_data is part of the scene allocation.
        size_t scene_size = sizeof(ogt_vox_scene) + misc_data_size;
        ogt_vox_scene * merged_scene = (ogt_vox_scene*)_vox_calloc(scene_size);

        // copy name and keyframe data into the misc_data section and make instances/groups point to it.
        // This makes the merged model self-contained.
        {
            char* scene_misc_data = (char*)&merged_scene[1];
            for (uint32_t instance_index = 0; instance_index < num_instances; instance_index++) {
                if (instances[instance_index].name) {
                    size_t string_len = _vox_strlen(instances[instance_index].name) + 1; // +1 for zero terminator
                    memcpy(scene_misc_data, instances[instance_index].name, string_len);
                    instances[instance_index].name = scene_misc_data;
                    scene_misc_data += string_len;
                }
                if (instances[instance_index].model_anim.num_keyframes) {
                    ogt_vox_keyframe_model* model_keyframes = (ogt_vox_keyframe_model*)scene_misc_data;
                    uint32_t keyframe_size = sizeof(ogt_vox_keyframe_model) * instances[instance_index].model_anim.num_keyframes;
                    memcpy(scene_misc_data, instances[instance_index].model_anim.keyframes, keyframe_size);
                    instances[instance_index].model_anim.keyframes = model_keyframes;
                    scene_misc_data += keyframe_size;
                    // bias the model_index in model_keyframes back into the range for the scene that this instance came from.
                    for (int32_t scene_index = (int32_t)(per_scene_base_instance_index.size() - 1); scene_index >= 0; scene_index--) {
                        if (instance_index >= per_scene_base_instance_index[scene_index]) {
                            for (uint32_t j = 0; j < instances[instance_index].model_anim.num_keyframes; j++)
                                model_keyframes[j].model_index += per_scene_base_model_index[scene_index];
                            break;
                        }
                    }
                }
                if (instances[instance_index].transform_anim.num_keyframes) {
                    uint32_t keyframe_size = sizeof(ogt_vox_keyframe_transform) * instances[instance_index].transform_anim.num_keyframes;
                    memcpy(scene_misc_data, instances[instance_index].transform_anim.keyframes, keyframe_size);
                    instances[instance_index].transform_anim.keyframes = (ogt_vox_keyframe_transform*)scene_misc_data;
                    scene_misc_data += keyframe_size;
                }
            }
            for (uint32_t group_index = 0; group_index < num_groups; group_index++) {
                if (groups[group_index].name) {
                    size_t string_len = _vox_strlen(groups[group_index].name) + 1; // +1 for zero terminator
                    memcpy(scene_misc_data, groups[group_index].name, string_len);
                    groups[group_index].name = scene_misc_data;
                    scene_misc_data += string_len;
                }
                if (groups[group_index].transform_anim.num_keyframes) {
                    uint32_t keyframe_size = sizeof(ogt_vox_keyframe_transform) * groups[group_index].transform_anim.num_keyframes;
                    memcpy(scene_misc_data, groups[group_index].transform_anim.keyframes, keyframe_size);
                    groups[group_index].transform_anim.keyframes = (ogt_vox_keyframe_transform*)scene_misc_data;
                    scene_misc_data += keyframe_size;
                }
            }

            ogt_assert(scene_misc_data <= ((char*)&merged_scene[1] + misc_data_size), "misc_data allocation was overrun");
        }

        ogt_assert(num_groups <= max_groups, "sanity check failed. we wrote more groups than we allocated");

        memset(merged_scene, 0, sizeof(ogt_vox_scene));
        merged_scene->instances     = instances;
        merged_scene->num_instances = max_instances;
        merged_scene->models        = (const ogt_vox_model * *)models;
        merged_scene->num_models    = max_models;
        merged_scene->layers        = layers;
        merged_scene->num_layers    = max_layers;
        merged_scene->groups        = groups;
        merged_scene->num_groups    = num_groups;
        // copy color palette into the merged scene
        for (uint32_t color_index = 0; color_index < 256; color_index++)
            merged_scene->palette.color[color_index] = master_palette[color_index];
        // copy materials into the merged scene
        for (uint32_t color_index = 0; color_index < 256; color_index++)
            merged_scene-> materials.matl[color_index] = materials[color_index];

        return merged_scene;
    }

    void ogt_vox_test()
    {
        // frame_index looping tests
        {
            const char* test_message = "failed compute_looped_frame_index test";
            (void)test_message;
            // [0,0] = 1 keyframe animation starting at frame 0
            ogt_assert(compute_looped_frame_index( 0,  0, 0 ) == 0, test_message);
            ogt_assert(compute_looped_frame_index( 0,  0, 1 ) == 0, test_message);
            ogt_assert(compute_looped_frame_index( 0,  0, 15) == 0, test_message);
            // [1,1] = 1 keyframe animation starting at frame 1
            ogt_assert(compute_looped_frame_index( 1,  1,  0) == 1, test_message);
            ogt_assert(compute_looped_frame_index( 1,  1,  1) == 1, test_message);
            ogt_assert(compute_looped_frame_index( 1,  1, 15) == 1, test_message);
            // [0,9] = 10 keyframe animation starting at frame 0
            ogt_assert(compute_looped_frame_index( 0,  9,  0) == 0, test_message);
            ogt_assert(compute_looped_frame_index( 0,  9,  4) == 4, test_message);
            ogt_assert(compute_looped_frame_index( 0,  9,  9) == 9, test_message);
            ogt_assert(compute_looped_frame_index( 0,  9, 10) == 0, test_message);
            ogt_assert(compute_looped_frame_index( 0,  9, 11) == 1, test_message);
            ogt_assert(compute_looped_frame_index( 0,  9, 14) == 4, test_message);
            ogt_assert(compute_looped_frame_index( 0,  9, 19) == 9, test_message);
            ogt_assert(compute_looped_frame_index( 0,  9, 21) == 1, test_message);
            // [4,13] = 10 keyframe animation starting at frame 4
            ogt_assert(compute_looped_frame_index(4, 13, 0 ) == 10, test_message);
            ogt_assert(compute_looped_frame_index(4, 13, 3 ) == 13, test_message);
            ogt_assert(compute_looped_frame_index(4, 13, 4 ) == 4,  test_message);
            ogt_assert(compute_looped_frame_index(4, 13, 5 ) == 5,  test_message);
            ogt_assert(compute_looped_frame_index(4, 13, 12) == 12, test_message);
            ogt_assert(compute_looped_frame_index(4, 13, 13) == 13, test_message);
            ogt_assert(compute_looped_frame_index(4, 13, 14) == 4,  test_message);
            ogt_assert(compute_looped_frame_index(4, 13, 21) == 11, test_message);
        }

    }

 #endif // #ifdef OGT_VOX_IMPLEMENTATION

/* -------------------------------------------------------------------------------------------------------------------------------------------------

    MIT License

    Copyright (c) 2019 Justin Paver

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.

------------------------------------------------------------------------------------------------------------------------------------------------- */

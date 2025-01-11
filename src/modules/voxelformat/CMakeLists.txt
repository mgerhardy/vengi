set(LIB voxelformat)
set(EXTERNAL_SRCS
	external/ogt_vox.h
	external/tiny_obj_loader.h
	external/tiny_gltf.h
	external/ufbx.h external/ufbx.c
	external/libvxl.h external/libvxl.c

)
set(SRCS
	${EXTERNAL_SRCS}

	Format.h Format.cpp
	FormatConfig.h FormatConfig.cpp
	FormatThumbnail.h
	VolumeFormat.h VolumeFormat.cpp

	private/aceofspades/AoSVXLFormat.h       private/aceofspades/AoSVXLFormat.cpp
	private/animatoon/AnimaToonFormat.h      private/animatoon/AnimaToonFormat.cpp
	private/benvoxel/BenVoxelFormat.h        private/benvoxel/BenVoxelFormat.cpp
	private/benvoxel/BenBinary.h             private/benvoxel/BenBinary.cpp
	private/benvoxel/BenJson.h               private/benvoxel/BenJson.cpp
	private/benvoxel/BenShared.h             private/benvoxel/BenShared.cpp
	private/benvoxel/SparseVoxelOctree.h     private/benvoxel/SparseVoxelOctree.cpp
	private/benvoxel/Node.h                  private/benvoxel/Node.cpp
	private/benvoxel/Branch.h                private/benvoxel/Branch.cpp
	private/benvoxel/Leaf.h                  private/benvoxel/Leaf.cpp

	private/binvox/BinVoxFormat.h            private/binvox/BinVoxFormat.cpp
	private/chronovox/CSMFormat.h            private/chronovox/CSMFormat.cpp
	private/commandconquer/HVAFormat.h       private/commandconquer/HVAFormat.cpp
	private/commandconquer/VXLFormat.h       private/commandconquer/VXLFormat.cpp
	private/commandconquer/VXLShared.h       private/commandconquer/VXLShared.cpp
	private/cubeworld/CubFormat.h            private/cubeworld/CubFormat.cpp
	private/cubzh/CubzhShared.h
	private/cubzh/CubzhFormat.h              private/cubzh/CubzhFormat.cpp
	private/cubzh/PCubesFormat.h             private/cubzh/PCubesFormat.cpp
	private/cubzh/CubzhB64Format.h           private/cubzh/CubzhB64Format.cpp
	private/goxel/GoxFormat.h                private/goxel/GoxFormat.cpp
	private/image/AsepriteFormat.h           private/image/AsepriteFormat.cpp
	private/image/PNGFormat.h                private/image/PNGFormat.cpp
	private/magicavoxel/XRawFormat.h         private/magicavoxel/XRawFormat.cpp
	private/magicavoxel/VoxFormat.h          private/magicavoxel/VoxFormat.cpp
	private/magicavoxel/MagicaVoxel.h        private/magicavoxel/MagicaVoxel.cpp
	private/mesh/Autodesk3DSFormat.h         private/mesh/Autodesk3DSFormat.cpp
	private/mesh/BlockbenchFormat.h          private/mesh/BlockbenchFormat.cpp
	private/mesh/MeshFormat.h                private/mesh/MeshFormat.cpp
	private/mesh/MeshMaterial.h              private/mesh/MeshMaterial.cpp
	private/mesh/OBJFormat.h                 private/mesh/OBJFormat.cpp
	private/mesh/PLYFormat.h                 private/mesh/PLYFormat.cpp
	private/mesh/STLFormat.h                 private/mesh/STLFormat.cpp
	private/mesh/TextureLookup.h private/mesh/TextureLookup.cpp
	private/mesh/quake/QuakeBSPFormat.h      private/mesh/quake/QuakeBSPFormat.cpp
	private/mesh/quake/MapFormat.h           private/mesh/quake/MapFormat.cpp
	private/mesh/quake/MD2Format.h           private/mesh/quake/MD2Format.cpp
	private/mesh/quake/MDLFormat.h           private/mesh/quake/MDLFormat.cpp
	private/mesh/FBXFormat.h                 private/mesh/FBXFormat.cpp
	private/mesh/GLTFFormat.h                private/mesh/GLTFFormat.cpp
	private/mesh/Polygon.h                   private/mesh/Polygon.cpp
	private/mesh/PosSampling.h               private/mesh/PosSampling.cpp
	private/mesh/MeshTri.h                   private/mesh/MeshTri.cpp
	private/minecraft/DatFormat.h            private/minecraft/DatFormat.cpp
	private/minecraft/MCRFormat.h            private/minecraft/MCRFormat.cpp
	private/minecraft/MTSFormat.h            private/minecraft/MTSFormat.cpp
	private/minecraft/SchematicFormat.h      private/minecraft/SchematicFormat.cpp
	private/minecraft/MinecraftPaletteMap.h  private/minecraft/MinecraftPaletteMap.cpp
	private/minecraft/NamedBinaryTag.h       private/minecraft/NamedBinaryTag.cpp
	private/minecraft/SchematicIntReader.h   private/minecraft/SchematicIntWriter.h
	private/qubicle/QBTFormat.h              private/qubicle/QBTFormat.cpp
	private/qubicle/QBFormat.h               private/qubicle/QBFormat.cpp
	private/qubicle/QBCLFormat.h             private/qubicle/QBCLFormat.cpp
	private/qubicle/QEFFormat.h              private/qubicle/QEFFormat.cpp
	private/rooms/ThingFormat.h              private/rooms/ThingFormat.cpp
	private/rooms/ThingNodeParser.h          private/rooms/ThingNodeParser.cpp
	private/sandbox/VXAFormat.h              private/sandbox/VXAFormat.cpp
	private/sandbox/VXBFormat.h              private/sandbox/VXBFormat.cpp
	private/sandbox/VXCFormat.h              private/sandbox/VXCFormat.cpp
	private/sandbox/VXMFormat.h              private/sandbox/VXMFormat.cpp
	private/sandbox/VXRFormat.h              private/sandbox/VXRFormat.cpp
	private/sandbox/VXTFormat.h              private/sandbox/VXTFormat.cpp
	private/slab6/KVXFormat.h                private/slab6/KVXFormat.cpp
	private/slab6/KV6Format.h                private/slab6/KV6Format.cpp
	private/slab6/SLAB6VoxFormat.h           private/slab6/SLAB6VoxFormat.cpp
	private/slab6/SLABShared.h               private/slab6/SLABShared.cpp
	private/spritestack/SpriteStackFormat.h  private/spritestack/SpriteStackFormat.cpp
	private/sproxel/SproxelFormat.h          private/sproxel/SproxelFormat.cpp
	private/starmade/SMFormat.h              private/starmade/SMFormat.cpp
	private/starmade/SMTPLFormat.h           private/starmade/SMTPLFormat.cpp
	private/starmade/SMPalette.h
	private/vengi/VENGIFormat.h              private/vengi/VENGIFormat.cpp
	private/voxel3d/V3AFormat.h              private/voxel3d/V3AFormat.cpp
	private/voxelbuilder/VBXFormat.h         private/voxelbuilder/VBXFormat.cpp
	private/voxelmax/BinaryPList.h           private/voxelmax/BinaryPList.cpp
	private/voxelmax/VMaxFormat.h            private/voxelmax/VMaxFormat.cpp
)
engine_add_module(TARGET ${LIB} SRCS ${SRCS} DEPENDENCIES scenegraph json metric)

check_c_compiler_flag("-Wno-shadow" C_NO_SHADOW)
if (C_NO_SHADOW)
	foreach (src ${EXTERNAL_SRCS})
		set_property(SOURCE ${src} APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-shadow")
	endforeach()
endif()
check_c_compiler_flag("-Wno-unused-variable" C_NO_UNUSED_VARIABLE)
if (C_NO_UNUSED_VARIABLE)
	foreach (src ${EXTERNAL_SRCS})
		set_property(SOURCE ${src} APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-unused-variable")
	endforeach()
endif()
foreach (src ${EXTERNAL_SRCS})
	engine_source_optimize(${src})
endforeach()

set(TEST_SRCS
	tests/AbstractFormatTest.h tests/AbstractFormatTest.cpp
	tests/AnimaToonFormatTest.cpp
	tests/AoSVXLFormatTest.cpp
	tests/AsepriteFormatTest.cpp
	tests/Autodesk3DSFormatTest.cpp
	tests/BenVoxelFormatTest.cpp
	tests/BinVoxFormatTest.cpp
	tests/BlockbenchFormatTest.cpp
	tests/ConvertTest.cpp
	tests/FormatPaletteTest.cpp
	tests/CSMFormatTest.cpp
	tests/CubFormatTest.cpp
	tests/CubzhFormatTest.cpp
	tests/CubzhB64FormatTest.cpp
	tests/FBXFormatTest.cpp
	tests/GLTFFormatTest.cpp
	tests/GoxFormatTest.cpp
	tests/KVXFormatTest.cpp
	tests/KV6FormatTest.cpp
	tests/MapFormatTest.cpp
	tests/MeshFormatTest.cpp
	tests/MCRFormatTest.cpp
	tests/MD2FormatTest.cpp
	tests/MDLFormatTest.cpp
	tests/OBJFormatTest.cpp
	tests/PLYFormatTest.cpp
	tests/PNGFormatTest.cpp
	tests/QBTFormatTest.cpp
	tests/QBFormatTest.cpp
	tests/QBCLFormatTest.cpp
	tests/QEFFormatTest.cpp
	tests/QuakeBSPFormatTest.cpp
	tests/SchematicFormatTest.cpp
	tests/SpriteStackFormatTest.cpp
	tests/SproxelFormatTest.cpp
	tests/SLABSharedTest.cpp
	tests/SLAB6VoxFormatTest.cpp
	tests/SMFormatTest.cpp
	tests/SMTPLFormatTest.cpp
	tests/STLFormatTest.cpp
	tests/MeshTriTest.cpp
	tests/ThingFormatTest.cpp
	tests/V3AFormatTest.cpp
	tests/VBXFormatTest.cpp
	tests/VolumeFormatTest.cpp
	tests/VoxFormatTest.cpp
	tests/VXBFormatTest.cpp
	tests/VXLFormatTest.cpp
	tests/VXRFormatTest.cpp
	tests/VXMFormatTest.cpp
	tests/VENGIFormatTest.cpp
	tests/VMaxFormatTest.cpp
	tests/XRawFormatTest.cpp

	tests/BinaryPListTest.cpp
	tests/MinecraftPaletteMapTest.cpp
	tests/NamedBinaryTagTest.cpp
	tests/TextureLookupTest.cpp

	tests/TestHelper.cpp tests/TestHelper.h
	tests/8ontop.h
	tests/vox_character.h
	tests/vox_glasses.h
)
set(TEST_FILES
	tests/palette-nippon.png

	tests/sora.ben.json
	tests/sora.ben
	tests/ambient-occlusion.vengi
	tests/qubicle.qb
	tests/qubicle.qbt
	tests/qubicle.qef
	tests/qubicle.qbcl
	tests/testload.qef
	tests/aceofspades.vxl
	tests/chronovox-studio.csm
	tests/cc.vxl
	tests/cc.hva
	tests/hmec.vxl
	tests/hmec.hva
	tests/test.binvox
	tests/test.kvx
	tests/test.kv6
	tests/test2.kv6
	tests/minecraft_110.mca
	tests/minecraft_113.mca
	tests/magicavoxel.vox
	tests/test_material.vox
	tests/cube.mtl
	tests/cube.obj
	tests/cube.ply
	tests/cube.stl
	tests/ascii.stl
	tests/ascii.ply
	tests/cubictest.vox
	tests/testao.vox
	tests/test.gox
	tests/test.map
	tests/test.plist
	tests/palette.settings.vmaxpsb
	tests/0voxel.vmax.zip
	tests/1voxel.vmax.zip
	tests/2voxel.vmax.zip
	tests/5voxel.vmax.zip
	tests/sandbox-block2.vxb
	tests/test.vxm
	tests/test2.vxm
	tests/8ontop.vox
	tests/Isanth_Type-Zero_Bc.sment
	tests/bat_anim.vengi
	tests/cw.cub
	tests/crabby.qbcl
	tests/crabby.vox
	tests/bug393.obj
	tests/kvx_save.qb
	tests/particubes.pcubes
	tests/testrooms.node
	tests/r.0.-2.mca
	tests/r.0.-2.qb
	tests/ufoai.bsp
	tests/fuel_can.md2
	tests/fuel_can.png
	tests/test-fillplane.png
	tests/test-heightmap-dm.png
	tests/test-heightmap.png
	tests/rgb.vxl
	tests/rgb.csv
	tests/rgb.gltf
	tests/rgb.vox
	tests/rgb.gox
	tests/rgb.qb
	tests/rgb.qef
	tests/rgb.vxm
	tests/rgb.cub
	tests/rgb.qbcl
	tests/rgb_small.cub
	tests/rgb_small.qb
	tests/rgb_small.qbcl
	tests/rgb_small.qbt
	tests/rgb_small.vox
	tests/rgb_small.vxm
	tests/slab6_vox_test.kv6
	tests/slab6_vox_test.kvx
	tests/slab6_vox_test.vox
	tests/spritestack.zip
	tests/blockbench_meshtypes.bbmodel
	tests/female_template_head_4_10.bbmodel
	tests/loy_s_goodies_female_template.bbmodel
	tests/test.litematic
	tests/vox_character.vox
	tests/vox_glasses.vox
	tests/voxlap5.kv6
	tests/vox-to-kv6-broken.vox
	tests/testkv6-multiple-slots.vengi
	tests/glTF/cube/Cube_BaseColor.png
	tests/glTF/cube/Cube_MetallicRoughness.png
	tests/glTF/cube/Cube.bin
	tests/glTF/cube/Cube.gltf
	tests/glTF/BoxAnimated.glb
	tests/glTF/lantern/Lantern_baseColor.png
	tests/glTF/lantern/Lantern_emissive.png
	tests/glTF/lantern/Lantern_normal.png
	tests/glTF/lantern/Lantern_roughnessMetallic.png
	tests/glTF/lantern/Lantern.bin
	tests/glTF/lantern/Lantern.gltf
	"tests/e2de1723/e2de1723.Idle.vxa"
	"tests/e2de1723/e2de1723.vxr"
	"tests/e2de1723/Content (1).vxm"
	"tests/e2de1723/Content.vxm"
	"tests/giant_dinosaur/Head_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/Hip_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/L_Arm_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/L_Foot_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/L_ForeArm_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/L_Hand_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/L_Leg_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/L_Thigh_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/Mouth_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/Neck_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/R_Arm_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/Reptiles_Biped_Giant_Dinossaur_V2.Attack 01.vxa"
	"tests/giant_dinosaur/Reptiles_Biped_Giant_Dinossaur_V2.Death 01.vxa"
	"tests/giant_dinosaur/Reptiles_Biped_Giant_Dinossaur_V2.Death Pose 01.vxa"
	"tests/giant_dinosaur/Reptiles_Biped_Giant_Dinossaur_V2.Eat 01.vxa"
	"tests/giant_dinosaur/Reptiles_Biped_Giant_Dinossaur_V2.Eat 02.vxa"
	"tests/giant_dinosaur/Reptiles_Biped_Giant_Dinossaur_V2.Get Hit 01.vxa"
	"tests/giant_dinosaur/Reptiles_Biped_Giant_Dinossaur_V2.Idle 01.vxa"
	"tests/giant_dinosaur/Reptiles_Biped_Giant_Dinossaur_V2.Run 01.vxa"
	"tests/giant_dinosaur/Reptiles_Biped_Giant_Dinossaur_V2.vxr"
	"tests/giant_dinosaur/Reptiles_Biped_Giant_Dinossaur_V2.Walk 01.vxa"
	"tests/giant_dinosaur/R_Foot_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/R_ForeArm_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/R_Hand_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/R_Leg_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/Tail1_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/Tail2_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/Tail3_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/Tail4_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/Torax1_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/giant_dinosaur/Torax2_Reptiles_Biped_Medium_Dinosaur.vxm"
	"tests/Screenshot_2022_Mar_09_13_43_20.png"
	"tests/save_2022_Mar_09_13_43_20_Anima Toon.pscn"
	"tests/save_2022_Mar_09_13_43_20_Anima Toon.scn"
	"tests/Anima Toon anim.scn"
	"tests/Anima Toon.scn"
	tests/voxelbuilder.vbx
	tests/robo.vox
	tests/robo.qb
	tests/chr_knight.qb
	tests/chr_knight.qbcl
	tests/chr_knight.gox
	tests/chr_knight.fbx

	tests/sylvie/02.ase
	tests/sylvie/404.ase
	tests/sylvie/46.ase
	tests/sylvie/line.ase
	tests/sylvie/mapmenu.ase
	tests/libresprite.ase
	tests/libresprite.aseprite
)

gtest_suite_begin(tests-${LIB} TEMPLATE ${ROOT_DIR}/src/modules/core/tests/main.cpp.in)
gtest_suite_sources(tests-${LIB} ${TEST_SRCS})
gtest_suite_files(tests-${LIB} ${TEST_FILES})
gtest_suite_deps(tests-${LIB} ${LIB} test-app video)
gtest_suite_end(tests-${LIB})

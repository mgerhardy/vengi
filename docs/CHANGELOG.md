# Changelog

A more detailed changelog can be found on [github](https://github.com/mgerhardy/vengi/commits/).

Join our [discord server](https://discord.gg/AgjCPXy).

See [the documentation](https://mgerhardy.github.io/vengi/) for further details.

Known [issues](https://github.com/mgerhardy/vengi/issues?q=is%3Aissue+is%3Aopen+label%3Abug).

## 0.0.26 (2023-XX-XX)

General:

   - Fixed pivot handling in VXR format
   - Allow to export the GLTF vertex colors as byte values
   - Added NeuQuant color quantization algorithm
   - Optimized GLTF and MD2 import

VoxEdit:

   - Improved shape handling a lot
   - Added (disabled) pathtracer support (Yocto/GL)
   - Fixed rendering order of overlapping bounding boxes for the active node
   - Added warning popup to split volumes into smaller pieces

## 0.0.25 (2023-07-28)

General:

   - Fixed invalid x axis handling for Sandbox VXM format
   - Fixed pivot handling in VXR/VXA format
   - Support model references to build a complex scene
   - Extended GLTF support to animation import and export as well as normal export
   - Fixed missing base color support for GLTF
   - Support for some parts of VoxelMax format
   - Fixed Sandbox VXA version 3 support
   - Fixed volume rotation issues
   - Fixed volume merging issues
   - Added Quake2 Model (`md2`) support
   - Removed cvar `voxformat_marchingcubes` and replaced with `voxel_meshmode` (set to `1` to use marching cubes)
   - Added new lua script `gradient.lua`
   - Improved `csm` and `nvm` format support
   - Added lua script for generating mazes
   - Added the ability to easily scale a volume up
   - Unified naming of commands and parameters (`layer` is `model` now, ...)
   - Added VoxelBuilder (`vbx`) support
   - Fixed missing group transform handling for MagicaVoxel files
   - Added support for MagicaVoxel `xraw` format
   - Added alpha support for MagicaVoxel materials
   - Added support for loading Sandbox VXM format version 3
   - Added new lua script and extended lua api to replace palettes, resize volumes and move voxels

VoxConvert:

   - Fixed `--input` handling if the value is a directory
   - Added option `--wildcard` to specify a wildcard in cases where the `--input` value is a directory
   - Added `--surface-only` parameter that removes any non surface voxel.
   - Added `--dump-mesh` to also show mesh details (like vertex count).

VoxEdit:

   - Allow to export selections only
   - Started to support different keymaps (blender, qubicle, magicavoxel and vengi own)
   - Added support for multiple animations in one scene
   - Allow to duplicate all scene graph nodes
   - (Re-)implemented WASD controls in camera eye mode
   - Fixed copy&paste errors with multiple selections
   - Updated new scene dialog to include the templates
   - Improved font quality
   - Fixed a few crashes
   - Added hollow functionality to remove invisible voxels
   - Preview of modifier shapes in edit mode

Thumbnailer:

   - Improved camera placement
   - Fixed threading issue with the mesh extraction for the renderer that could lead to black thumbnails

## 0.0.24 (2023-03-12)

General:

   - Support using `palette` cvar as a destination palette again by setting `voxformat_createpalette` to `false`
   - Added new image formats from SOIL2 (DDS, PKM, PVR)
   - Added FBX read support
   - Improved texture lookup for mesh formats
   - Added multiple color reduction algorithms and expose them to the user by the cvar `core_colorreduction`
   - Fixed several issues with AceOfSpades vxl format - switched to libvxl for loading and saving
   - Fixed splitting volumes even if not needed (off-by-one error)
   - Fixed multi-monitor support
   - Fixed colors in the tree generator
   - Improved the key binding handling and made it more flexible
   - Added support for loading minetest mts files
   - Fixed Command&Conquer vxl format writing issue
   - Added own format with the extension `vengi`
   - Added basic alpha support
   - Fixed saving the keybindings with multi click
   - Improved keybinding and ui setting saving (added a version)
   - Fixed invalid transform on re-parenting a node
   - Refactored the file dialog
   - Fixed issue in saving MagicaVoxel vox files under some special conditions
   - Implemented basic transparency support for voxels
   - Fixed invalid clamping for uv based pixel lookup (mesh imports)
   - Update renderer to only use uniform buffers
   - Fixed rendering issue for windows users
   - Fixed fullscreen issues for windows users
   - Extended lua script bindings to allow to render text as voxels
   - Support GLES3 rendering

VoxConvert:

   - Show supported palette and image formats in help screen (`--help`)

VoxEdit:

   - Disable animation window if there are no animations to show
   - Allow to show the color picker in the palette panel
   - Reworked the modifiers panel
   - Made undo/redo more visible
   - Switched the camera modifier to the right side of the viewport
   - Fixed diffuse and ambient color settings mix-up
   - Merged layer and scene graph panel into one
   - Several gizmo related fixes and improvements
   - Added simple UI mode which removes some panels
   - Fixed a few memento (undo/redo) related issues
   - Added templates to the menu (`robo`, `chess`, `head` and `chr_knight`)
   - Allow to control the amount of viewports
   - Allow to define pre-defined region sizes (see `ve_regionsizes` cvar)
   - Some ui actions are only available in edit mode
   - Edit mode has support for the gizmo now - you can shift the volume boundaries
   - Highlight copy&paste volume region
   - Visually disable some buttons if they won't work in the current mode anyway
   - Don't execute actions for hidden nodes
   - New keybindings
   - Allow to sort the palette colors by hue, saturation or brightness
   - Allow to select and drag keyframes in the animation timeline
   - Export animations as AVI
   - Allow to import a whole directory into a scene
   - Allow to select all node types in the scene graph panel
   - Allow to edit node properties
   - Added voxel cursor tooltips about the position in the volume
   - WASM/HTML5 port

## 0.0.23 (2022-12-17)

General:

   - Improved big endian support for voxel formats
   - Improved VXL format default palette support
   - Improved QBCL scene graph support
   - Improved voxelization vertex color support
   - Fixed VOX root node handling
   - Fixed QBCL and GOX thumbnail handling
   - Removed unused code
   - Added support for VXA version 3
   - (Re-)added support for marching cubes
   - Fixed a bug in Ace of Spades VXL loading

VoxEdit:

   - Added support for embedding screenshots in formats that support it
   - Allow to export palettes
   - Allow to change color intensity of the whole palette
   - Allow to voxelize text
   - Fixed orthographic cameras
   - Massive performance increase when using multiple viewports
   - More than 10 times gpu memory reduction
   - Render the camera nodes in scene mode

Thumbnailer:

   - Added support for turntable rendering

## 0.0.22 (2022-10-31)

General:

   - Improved GLTF format support
   - Improved VXL format support
   - Improved Qubicle QB support
   - Fixed block id parsing for StarMade voxel models
   - Major improvements in scene graph transform handling
   - Improved voxelization of meshes with voxels
   - Added kv6 write support
   - Added slab6 vox write support
   - Fixed saving black colors for cubeworld
   - Fixed saving palette index 0 for binvox
   - Fixed Sandbox VXM palette issue
   - Fixed QBCL saving
   - Improved qbt scene graph support
   - Improved vox saving with multiple palettes
   - Improved the file dialog size and special dir handling
   - Improved dark mode support
   - Improved palette support for some formats

Packaging:

   - There is a snap package available now (`io.github.mgerhardy.vengi.voxedit`)

VoxEdit:

   - Fixed layer color selection if you have multiple layers
   - Fixed moving nodes in the scene graph panel
   - Fixed transforms in scene graph mode (translation, rotation)
   - Allow to add group and camera nodes
   - Don't just quit the application if you have unsaved data in your scene

## 0.0.21 (2022-09-05)

General:

   - Added support for minecraft 1.13 region files
   - Added support for loading minecraft level.dat (only with supported region files)
   - Added support for WorldEdit schematics
   - Added support for Minecraft nbt files
   - Added support for StarMade voxel models
   - Added support for Quake1 and UFO:Alien Invasion
   - Reduced memory footprint for voxelizing huge meshes
   - Support wrap mode texture settings for gltf voxelization
   - Improved sanity check for Qubicle Binary format support
   - Fixed texture lookup error in gltf voxelization
   - Extended lua vector bindings and allow to import heightmaps
   - Improved the file dialog filters
   - Added new lua scripts and extended lua integration in voxconvert
   - Added support for RGB (`pal`) and Gimp (`gpl`) palette loading
   - Improved the Command & Conquer VXL format support

VoxEdit:

   - Fixed resetting the camera in eye mode
   - New asset panel
   - Place images, models and colors via drag and drop into the scene
   - Extended scene mode modifiers to allow resizing the volumes (double click in scene mode)
   - Fixed updating the locked axis plane on change
   - Fixed scene graph node panel width
   - Fixed mesh extraction on chunk boundaries
   - Fixed new volume dialog input handling
   - Fixed cursor being invisible with bloom disabled
   - Fixed cursor face being on the wrong side of the voxel at the volume edges
   - Fixed last opened files with spaces in their names
   - Fixed loading files from command line again
   - Allow to select scene graph node from the animation timeline
   - Fixed deleting key frames
   - Improved adding new key frames

## 0.0.20 (2022-06-14)

General:

   - Added support for minecraft schematic
   - Refactored and extended the lua script integration
   - Implemented applying depth/height map to a 2d plane
   - Added support for new magicavoxel format (animations)
   - Preserve node hierarchy when saving vxr
   - GLTF voxelization
   - Allow to enable certain renderer features
   - Expose more noise functions to the lua scripts
   - Expose more volume functions to the lua scripts
   - Allow to delete voxels from within a lua script
   - Improved splitting of volumes (target volume size)
   - Expose more region functions to the lua scripts
   - Added more lua example scripts
   - Improved color sampling for voxelization
   - Started to support different palettes in one scene
   - Fixed vxc support
   - Load the palette from the source file
   - Fixed vxm file path when saving vxr
   - Save vxmc (version 12) now
   - Changed default ambient color and gamma values
   - Improved osx dmg file and app bundles

VoxEdit:

   - Fixed start problems on some systems with multisampled framebuffers
   - Allow to drag and drop colors from the palette
   - Change between the edit and scene mode is now bound to `tab`
   - Updated imgizmo to support clicking the view cube
   - Cursor is no volume anymore but a plane
   - Implemented plane filling
   - Added extrude feature
   - Allow to place a single voxel
   - Fixed keyboard input errors that made the ui unusable
   - Don't reload the last opened file with every start

VoxConvert:

   - Extended `--dump` to also show the key frames and the voxel count
   - Removed `--src-palette` (src palette is always used)

## 0.0.19 (2022-03-27)

General:

   - Replaced minecraft support with own implementation
   - Added support for Sandbox VXA format (via VXR) and improved VXR
   - Allow to change the ui colors via cvar (`ui_style`)
   - Added bloom render support for vox and vxm
   - Added support for loading key frames if the format supports it
   - Improved apple support in file dialog
   - The palette handling was refactored
   - Allow to save the MATL chunk in magicavoxel vox files
   - Ability to scale exported mesh with different values for each axis
   - Added stl voxelization support
   - Allow to modify the camera zoom min/max values
   - Allow to load different sizes for AoS VXL files
   - Lerp the camera zooming
   - Added support for GLTF export
   - Added experimental export support for FBX ascii
   - Increased the max scene graph model nodes from 256 to 1024

VoxEdit:

   - Added new command to fill hollows in models
   - Fixed escape key not closing the dialogs
   - Added support for drag and drop the nodes of the scene graph
   - Scene graph rendering improved
   - Removed noise panel (use the lua scripts for noise support)
   - Fixed a lot of undo/redo cases and improved the test cases a lot
   - Fixed viewport screenshot creation (now also bound to F5)
   - Added dialog to configure the mesh and voxel format settings for loading/saving
   - Improved the palette panel
   - Improved the gizmo for translation and rotation
   - Open in scene mode as default

VoxConvert:

   - Added `--image-as-plane` and `--image-as-heightmap` parameters
   - Allow to create a palette from input files

## 0.0.18 (2022-02-12)

> renamed the github project to **vengi** - the url changed to [https://github.com/mgerhardy/vengi](https://github.com/mgerhardy/vengi).

Build:

   - Removed own cmake unity-build implementation
   - Fixed build when `GAMES` was set to `OFF`

General:

   - Extended qbcl format support
   - Fixed color conversion issue when importing palettes from voxel models
   - Voxelization of obj meshes now also fills the inner parts of the mesh with voxels
   - Fixed magicavoxel pivot issue (sometimes wrong positions)
   - Added support for sandbox vxc format
   - Added support for sandbox vxt format
   - Added new example lua scripts

VoxConvert:

   - `--input` can now also handle directories

VoxEdit:

   - Added context actions to scene graph panel
   - Fixed mouse input issues in fullscreen mode
   - Fixed script editor placement

## 0.0.17 (2022-01-23)

General:

   - Fixed relative path handling for registered paths
   - Stop event loop if window is minimized (reduce cpu usage)
   - Support scene graphs in the voxel formats
   - Fixed a few issues with the magicavoxel vox format (switched to ogt_vox)
   - Load properties from supported voxel formats (vxr, vox, gox)
   - Added support for loading minecraft region files (used enkimi)
   - Fixed vxm pivot and black color issue
   - Added obj voxelization
   - Improved obj export
   - Improved file dialog

VoxConvert:

   - Added `--crop` parameter that reduces the volumes to their real voxel sizes
   - Added `--split` option to cut volumes into smaller pieces
   - Added `--export-models` to export all the models of a scene into single files
   - Added `--dump` to dump the scene graph of the input file
   - Added `--resize` to resize the volumes by the given x, y and z values

VoxEdit:

   - Fixed torus shape
   - Added scene graph panel
   - Fixed an issue that delayed the start by a few seconds

## 0.0.16 (2021-12-27)

General:

   - Fixed magicavoxel vox file saving
   - Added support for old magicavoxel (pre RIFF) format
   - Fixed bugs in binvox support
   - Fixed save dir for vxm files when saving vxr
   - Save vxm version 5 (with included pivot)
   - Support bigger volumes for magicavoxel files

VoxConvert:

   - Fixed `--force` handling for target files
   - Allow to operate on multiple input files
   - Added `--translate` command line option
   - Added `--pivot` command line option
   - nippon palette is not loaded if `--src-palette` is used and it's no hard error anymore if this fails

VoxEdit:

   - Add recently used files to the ui

## 0.0.15 (2021-12-18)

General:

   - Fixed missing vxm (version 4) saving support
   - Fixed missing palette value for vxm saving
   - Added support for loading only the palettes
   - Added support for goxel gox file format
   - Added support for sproxel csv file format
   - Added support for a lot more image formats
   - Improved lod creation for thin surface voxels
   - Fixed vxr9 load support
   - Added support for writing vxr files

VoxConvert:

   - Added option to keep the input file palette and don't perform quantization
   - Allow to export the palette to png
   - Allow to generate models from heightmap images
   - Allow to run lua scripts to modify volumes
   - Allow to export or convert only single layers (`--filter`)
   - Allow to mirror and rotate the volumes

Thumbnailer:

   - Try to use the built-in palette for models

VoxEdit:

   - Allow to import palettes from volume formats, too
   - Implemented camera panning
   - Added more layer merge functions

## 0.0.14 (2021-11-21)

General:

   - License for our own voxel models is now CC-BY-SA
   - Support loading just the thumbnails from voxel formats
   - Support bigger volume sizes for a few formats
   - Don't pollute the home directory with build dir settings
   - Fixed gamma handling in shaders
   - Added bookmark support to the ui dialog

Thumbnailer:

   - Added qbcl thumbnail support

VoxEdit:

   - Render the inactive layer in grayscale mode

## 0.0.13 (2021-10-29)

General:

   - Logfile support added
   - Fixed windows DLL handling for animation hot reloading

UI:

   - Fixed log notifications taking away the focus from the current widget

VoxEdit:

   - Fixed windows OpenGL error while rendering the viewport

## 0.0.12 (2021-10-26)

General:

   - Fixed a few windows compilation issues
   - Fixed issues in the automated build pipelines to produce windows binaries

## 0.0.11 (2021-10-25)

General:

   - Added url command
   - Reduced memory allocations per frame
   - Added key bindings dialog
   - Added notifications for warnings and errors in the ui
   - Fixed Sandbox Voxedit VXM v12 loading and added saving support
   - Fixed MagicaVoxel vox file rotation handling

VoxEdit:

   - Removed old ui and switched to dearimgui
   - Added lua script editor
   - Added noise api support to the lua scripts

## 0.0.10 (2021-09-19)

General:

   - Added `--version` and `-v` commandline option to show the current version of each application
   - Fixed texture coordinate indices for multi layer obj exports
   - Improved magicavoxel transform support for some models
   - Fixed magicavoxel x-axis handling
   - Support newer versions of vxm and vxr
   - Fixed bug in file dialog which prevents you to delete characters #77

VoxEdit:

   - Improved scene edit mode
   - Progress on the ui conversion to dearimgui

Tools:

   - Rewrote the ai debugger

## 0.0.9 (2020-10-03)

General:

   - Fixed obj texcoord export: Sampling the borders of the texel now
   - Added multi object support to obj export

## 0.0.8 (2020-09-30)

General:

   - Added obj and ply export support
   - Restructured the documentation
   - Improved font support for imgui ui

Backend:

   - Reworked ai debugging network protocol
   - Optimized behaviour tree filters

## 0.0.7 (2020-09-15)

General:

   - Fixed wrong-name-for-symlinks shown
   - Added support for writing qef files
   - Added lua script interface to generate voxels
   - Added stacktrace support for windows
   - Refactored module structure (split app and core)
   - Optimized character animations
   - Hot reload character animation C++ source changes in debug builds
   - Added quaternion lua support
   - Updated external dependencies
   - Refactored lua bindings
   - Support Chronovox-Studio files (csm)
   - Support Nick's Voxel Model files (nvm)
   - Support more versions of the vxm format

VoxEdit:

   - Converted some voxel generation functions to lua
   - Implemented new voxel generator scripts

## 0.0.6 (2020-08-02)

General:

   - Fixed gamma cvar usage
   - Enable vsync by default
   - Updated external dependencies
   - Activated OpenCL in a few tools
   - Added symlink support to virtual filesystem

VoxEdit:

   - Fixed loading palette lua script with material definitions
   - Fixed error in resetting mirror axis
   - Fixed noise generation
   - Reduced palette widget size
   - Fixed palette widget being invisible on some dpi scales

## 0.0.5 (2020-07-26)

Client:

   - Fixed movement

Server:

   - Fixed visibility check
   - Fixed segfault while removing npcs

VoxEdit:

   - Started to add scene mode edit support (move volumes)

VoxConvert:

   - Support different palette files (cvar `palette`)
   - Support writing outside the registered application paths
   - Allow to overwrite existing files

General:

   - Switched to qb as default volume format
   - Improved scene graph support for Magicavoxel vox files
   - Fixed invisible voxels for qb and qbt (Qubicle) volume format
   - Support automatic loading different volume formats for assets
   - Support Command&Conquer vxl files
   - Support Ace of Spades map files (vxl)
   - Support Qubicle exchange format (qef)
   - Perform mesh extraction in dedicated threads for simple volume rendering
   - Improved gizmo rendering and translation support
   - Fixed memory leaks on shutdown
   - Improved profiling support via tracy

## 0.0.4 (2020-06-07)

General:

   - Added support for writing binvox files
   - Added support for reading kvx (Build-Engine) and kv6 (SLAB6) voxel volumes
   - Performed some AFL hardening on voxel format code
   - Don't execute keybindings if the console is active
   - Added basic shader storage buffer support
   - Reduced voxel vertex size from 16 to 8 bytes
   - Apply checkerboard pattern to water surface
   - Improved tracy profiling support
   - A few highdpi fixes

Server:

   - Allow to specify the database port
   - Fixed loading database chunks

VoxEdit:

   - Added `scale` console command to produce LODs

VoxConvert:

   - Added ability to merge all layers into one

## 0.0.3 (2020-05-17)

Assets:

   - Added music tracks
   - Updated and added some new voxel models

VoxEdit:

   - Made some commands available to the ui
   - Tweak `thicken` command
   - Updated default tree generation ui values
   - Save layers to all supported formats
   - Fixed tree generation issue for some tree types
   - Changed default reference position to be at the center bottom
   - Reduced max supported volume size

General:

   - Print stacktraces on asserts
   - Improved tree generation (mainly used in voxedit)
   - Fixed a few asserts in debug mode for the microsoft stl
   - Added debian package support
   - Fixed a few undefined behaviour issues and integer overflows that could lead to problems
   - Reorganized some modules to speed up compilation and linking times
   - Improved audio support
   - Fixed timing issues
   - Fixed invalid GL states after deleting objects

VoxConvert:

   - Added a new tool to convert different voxel volumes between supported formats
     Currently supported are cub (CubeWorld), vox (MagicaVoxel), vmx (VoxEdit Sandbox), binvox
     and qb/qbt (Qubicle)

Client:

   - Added footstep and ambience sounds

## 0.0.2 (2020-05-06)

VoxEdit:

   - Static linked VC++ Runtime
   - Extract voxels by color into own layers
   - Updated tree and noise windows
   - Implemented `thicken` console command
   - Escape abort modifier action
   - Added L-System panel

General:

   - Fixed binvox header parsing
   - Improved compilation speed
   - Fixed compile errors with locally installed glm 0.9.9
   - Fixed setup-documentation errors
   - Fixed shader pipeline rebuilds if included shader files were modified
   - Improved palm tree generator
   - Optimized mesh extraction for the world (streaming volumes)
   - Added new voxel models
   - (Re-)added Tracy profiler support and removed own imgui-based implementation
   - Fixed writing of key bindings
   - Improved compile speed and further removed the STL from a lot of places
   - Updated all dependencies to their latest version

Server/Client:

   - Added DBChunkPersister
   - Built-in HTTP server to download the chunks
   - Replaced ui for the client

Voxel rendering:

   - Implemented reflection for water surfaces
   - Apply checkerboard pattern to voxel surfaces
   - Up-scaling effect for new voxel chunks while they pop in
   - Optimized rendering by not using one giant vbo


## 0.0.1 "Initial Release" (2020-02-08)

VoxEdit:

   - initial release

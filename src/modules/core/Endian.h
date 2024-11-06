/**
 * @file
 */

#include <SDL3/SDL_endian.h>

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define CORE_LITTLE_ENDIAN 1
#else
#define CORE_LITTLE_ENDIAN 0
#endif

#define core_swap32le SDL_Swap32LE
#define core_swap32be SDL_Swap32BE
#define core_swap16le SDL_Swap16LE
#define core_swap16be SDL_Swap16BE
#define core_swap64le SDL_Swap64LE
#define core_swap64be SDL_Swap64BE

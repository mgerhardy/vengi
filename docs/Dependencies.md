# Dependencies

* cmake
* ninja-build or make
* compiler (see [compilation](Compilation.md) docs)

## Libraries

* development headers/libs for
  * glm
  * lua >= 5.4
  * sdl3 >= 3.1.3
  * gtest (and gmock)
  * freetype2 (optional)
  * libjpeg (optional but recommended)

Some of these dependencies might not be available as packages in your toolchain - most
of them are also bundled with the application. But local installed headers always have
the higher priority. Usually you don't have to install anything of these.

## Debian

```bash
apt-get install binutils-dev libunwind-dev libglm-dev lua5.4 liblua5.4-dev libfreetype-dev libsdl2-dev wayland-protocols pkg-config libjpeg-dev
```

## Arch

```bash
pacman -Sy git make cmake ninja sdl3 clang
```

## Brew

```bash
brew install sdl3 jpeg-turbo freetype cmake ninja pkg-config
```

## Windows

```bash
vcpkg install pkg-config sdl3 libjpeg-turbo lua glm glslang gtest freetype
```

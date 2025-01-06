# Winner, A Windows Magnifier App based on [Boomer](https://github.com/tsoding/boomer)

Written in C (using mingw-gcc) using OpenGL and the windows api

# Usage

By opening the executable the whole screen is consumed by the app. The app will close if q or escape are pressed. Drag the left mouse button to move the zoomed image around, and use the wheel to zoom in and out. While the flashlight is enabled pressing shift and scrolling will change its size. The keyboard can be used as follows:

| Key Combo     | Description                                   |
| ------------- |---------------------------------------------- |
| `q` or `ESC`  | Closes the application                        |
| `arrow keys`  | Moves the image around                        |
| `ctrl+c`      | Creates a copy of the screen to the clipboard |
| `o`           | Zooms the image out                           |
| `i`           | Zooms the image in                            |
| `r`           | Resets the zoom and position                  |
| `p`           | Resets only the camera position               |
| `f`           | Toggles the flashlight                        |
| `s`           | Toggles between nearest and linear filtering  |

The following are only enabled if winner is compiled for debug
| Key Combo     | Description                                   |
| ------------- |---------------------------------------------- |
| `t`           | Opens a popup window of the current zoom      |

# How to Compile

Winner is compiled with the build system called [nob](https://github.com/tsoding/nob.h). The build system must first be compiled using any C compiler, however winner requires gcc so that should be used
`gcc nob.c -o nob.exe`

Next run the build system with the `release` argument
`.\nob.exe release`

And that is it!

The build system has more arguments and options, too see them try `.\nob.exe help`

#!/bin/sh
ffmpeg -y -i output/%d.png -vf palettegen output/palette.png
ffmpeg -y -framerate $1 -i output/%d.png -i output/palette.png -lavfi paletteuse output.gif
#!/bin/sh
ffmpeg -i output/%d.png -vf palettegen output/palette.png
ffmpeg -i output/%d.png -i output/palette.png -lavfi paletteuse output.gif
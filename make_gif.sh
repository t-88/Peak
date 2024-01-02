#!/bin/sh
ffmpeg -framerate 50 -i output/%d.png -pix_fmt yuv420p output.gif
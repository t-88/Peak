# Peak
Peak is a screen capture application for **linux**, and only **linux**. 
its intended to replace peek, for my use, trying to fix the low fps problem i get when recording my gifs.

- why not cross platform:
i think gtk is cross platform, so Peak is kinda there but i only tested it for my mint linux machine.         
also i dont care, and i like the small codebase all the code is in the main.c and make_gif.sh files, not like peek source code where there are ton of files to see and go through

## Output Comparison
### Peek
<img src="./peek_output.gif" width=400 alt="peek output"/>        
 
### Peak
<img src="./peak_output.gif" width=400 alt="Peak output" />    


# Requiments
- **gtk+-3.0 , gdk-3.0**: window creation , rendering and screenshot capture
- **ffmpeg**: gif creation  

# Quick Start
```bash
    $ ./run.sh
    $ ./make_gif.sh
```
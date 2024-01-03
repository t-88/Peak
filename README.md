# peak
peak is a simple screen capture application for **linux**.
also use [Peek](https://github.com/phw/peek)

### Output
<img src="./peak_output.gif" width=400 alt="peak output" />    


# TODO
- make events propagate like [Peek](https://github.com/phw/peek)
- less gif size

# Requiments
- **gtk+-3.0 , gdk-3.0**: window creation , rendering and screenshot capture
- **ffmpeg**: gif creation  
- also u need a compositor, i used xcompmgr. i dont know how **Peek** does it without one, but it works for me.

# Quick Start
```bash
    $ ./run.sh
```
- **space** :  to start recording
- **escape**:  to quit

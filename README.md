# pydemux

ffmpeg wrapper to decode frames from video sequence

# Build

```
python setup.py install
```

# Usage

```python
from PyDemux import Video

v = Video.open('video.mov')
if v is not None:
    for i in rage(0, 10):
        im = v.get_frame()
        im.show()
```


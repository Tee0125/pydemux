# pydemux

ffmpeg wrapper to decode frames from video sequence

# Install 

From source:

```
python setup.py install
```

From anaconda cloud:

```
conda install -c tee pydemux
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


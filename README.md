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

for i in range(0, 10):
    im = v.get_frame()

    if im is None:
        break

    im.show()
```


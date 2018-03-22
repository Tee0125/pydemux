from PyDemux import _demux
from PIL import Image

class Demux(object):
    def __init__(self, filename):
        self.ctx = _demux.open(filename)

    def get_frame(self):
        frame, w, h = _demux.get_frame(self.ctx)

        if frame is not None:
            return Image.frombytes("RGB", (w, h), frame)
        else:
            return None

    def __del__(self):
        if self.ctx is not None:
            _demux.close(self.ctx)


def open(filename):
    """
    Opens video file
    """
    return Demux(filename)

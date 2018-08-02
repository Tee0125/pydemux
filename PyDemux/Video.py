from PyDemux import _demux
from PIL import Image

class Demux(object):
    def __init__(self, filename):
        self.ctx = _demux.open(filename)

    def get_frame(self):
        result = _demux.get_frame(self.ctx)

        if result is None:
            return None

        frame, w, h, = result

        return Image.frombytes("RGB", (w, h), frame)


    def seek(self, ms, stream_type=0, seek_type=0):
        return _demux.seek(self.ctx, ms, stream_type, seek_type);


    def __del__(self):
        if self.ctx is not None:
            _demux.close(self.ctx)


def open(filename):
    """
    Opens video file
    """
    return Demux(filename)

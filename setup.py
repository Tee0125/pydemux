#!/usr/bin/env python

from distutils.core import setup, Extension

import os

srcs = ['_demux.c', '_yuv2rgb.c', '_py.c']
defs = []
libs = ['avcodec', 'avformat', 'avutil', 'avfilter', 'vpx', 'swscale', 'swresample']

ext = Extension('PyDemux._demux', 
            srcs,
            libraries=libs)

setup(name='PyDemux',
      version='0.0',
      description='PyDemux extension',
      author='Tee Jung',
      author_email='naey05@gmail.com',
      url='https://github.com/Tee0125/pydemux',
      ext_modules=[ext],
      packages=['PyDemux'],
     )

#!/bin/sh

gst-launch-1.0 tcpclientsrc host=192.168.1.11 port=5000 ! jpegdec ! videoconvert ! autovideosink

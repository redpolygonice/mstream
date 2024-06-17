#!/bin/sh

ffplay http://192.168.1.37:8080/playlist.m3u8

gst-launch-1.0 souphttpsrc location=http://192.168.1.37:8080/playlist.m3u8 ! hlsdemux ! decodebin ! videoconvert ! videoscale ! autovideosink

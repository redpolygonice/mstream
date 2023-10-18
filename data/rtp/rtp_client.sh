#!/bin/sh

gst-launch-1.0 -v udpsrc uri=udp://192.168.1.33:5000 ! "application/x-rtp,media=video,clock-rate=90000,encoding-name=H264,payload=96" ! rtph264depay ! h264parse ! decodebin ! videoconvert ! autovideosink sync=false

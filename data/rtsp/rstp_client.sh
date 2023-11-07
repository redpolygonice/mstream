#!/bin/sh

gst-launch-1.0 rtspsrc location=rtsp://127.0.0.1:8554/camera ! queue ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! videoscale ! video/x-raw,width=640,height=480 ! autovideosink

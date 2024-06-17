#!/bin/sh

gst-launch-1.0 libcamerasrc ! video/x-raw,width=640,height=480,format=NV12 ! queue ! x264enc tune=zerolatency ! rtph264pay ! udpsink host=127.0.0.1 port=5000

gst-launch-1.0 libcamerasrc ! video/x-raw,width=640,height=480,format=NV12 ! queue ! jpegenc ! rtpjpegpay ! udpsink host=127.0.0.1 port=5000

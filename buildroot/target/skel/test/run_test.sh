#!/bin/sh

echo "Touch Screen Test"
ts_calibrate


echo "Test MIC"
sleep 10
/test/codec-test-mic-in


echo "Test LINE-IN/OUT"
/test/codec-test-line-in


echo "Test HDMI"
/test/display/fb_test.dat -o 4 9
sleep 30
/test/display/fb_test.dat -o 1 0


echo "Input Test"
/test/input-test












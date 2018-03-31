#!/bin/sh

infow2 5 5 MIC "Records 5sec, and plays"
infow2 6 5 AUD "Plays in 5 seconds"
codec-test-mic-in 2>&1 1>/dev/null

RESULT=$?
if [ $RESULT -eq 0 ]; then
	infow2 5 2 MIC "OK"
	infow2 6 2 AUD "OK"
else
	infow2 5 1 MIC "OK"
	infow2 6 1 AUD "OK"
fi

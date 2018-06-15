#!/bin/bash

SCRIPT_PATH="./exokit.sh"

# Execute ./exokit.sh -h and log the console output to testRunLog.txt,
# stdout and stderr included
"$SCRIPT_PATH" -h >testRunLog.txt 2>&1 &
echo "Exokit loaded, Exit Code: " $?

sleep 5s
# Get process ID of exokit and kill it. (does not work )
PID=$(pgrep -u root exokit.cmd)
$(pkill -SIGKILL $PID)
echo "Process killed, exit Code: " $?

echo "Finished test run of ./exokit.sh -h"


# keep console open for user to read or else goes away to quick
read
#!/bin/bash

"./exokit.sh" -t -h

exitCode=$?

if [ $exitCode -ne 0 ]; then
    echo "Error, Exokit failed to start! Exit code: " $exitCode
else
    echo "Success, Exokit started! Exit code: " $exitCode
fi
exit $exitCode

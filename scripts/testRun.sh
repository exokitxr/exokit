#!/bin/bash

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

"$DIR/exokit.sh" --quit

exitCode=$?

if [ $exitCode -ne 0 ]; then
    echo "Error, Exokit failed to start! Exit code: " $exitCode
else
    echo "Success, Exokit started! Exit code: " $exitCode
fi
exit $exitCode

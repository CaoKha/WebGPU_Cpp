#!/bin/bash
# There are 2 arguments you can pass into build.sh 
# --backend and --build-dir
# by default, --backend=wgpu and --build-dir=./build
# Ex command: ./build.sh --backend=dawn --build-dir=./build

BACKEND="wgpu"
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
BUILD_DIR="$SCRIPT_DIR/build"

while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    --backend=*)
    BACKEND="${key#*=}"
    shift # get backend value
    ;;
    --build-dir=*)
    BUILD_DIR="${key#*=}"
    shift # get build-dir value
    ;;
    *)    # unknown option
    echo "Unknown option: $1"
    exit 1
    ;;
esac
done

if [ "$BACKEND" = "dawn" ]; then
  cmake -B "$BUILD_DIR" -DWEBGPU_BACKEND=DAWN "$SCRIPT_DIR"
elif [ "$BACKEND" = "wgpu" ]; then
  cmake -B "$BUILD_DIR" -DWEBGPU_BACKEND=WGPU "$SCRIPT_DIR"
else 
  echo "Unknown backend: $BACKEND"
  exit 1
fi

echo "Building..."
cmake --build "$BUILD_DIR"
echo "Done."

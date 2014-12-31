#!/bin/bash

WORKING_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $WORKING_DIR

OUTPUT_EXEC_NAME="createStream"
OUTPUT_EXEC_DIR="."
C_SRC_DIR="src/c/"
C_OBJECT_DIR="bin/"
COFFEE_SRC_DIR="src/coffee/"
if [ $1 = 'bootstrap' ]; then
  npm install
elif [ $1 = 'build' ]; then
  ## C
  # set flags
  C_FLAGS="-Wall -Wextra -Werror -O3"
  # create object files
  find $C_SRC_DIR -name "*.c" \
       -exec ./modify_c_path.sh "$C_FLAGS" "$C_OBJECT_DIR" '{}' \;
  if [ ! -f ".error" ]; then
    # link
    echo "linking $OUTPUT_EXEC_DIR/$OUTPUT_EXEC_NAME..."
    C_LINK_FLAGS="-pthread"
    clang $C_OBJECT_DIR/*.o $C_LINK_FLAGS -o $OUTPUT_EXEC_DIR/$OUTPUT_EXEC_NAME
  fi
  if [ -f '.error' ]; then
    rm .error
  fi

  ## Coffeescript
  find $COFFEE_SRC_DIR -name "*.coffee" \
       -exec ./modify_coffee_path.sh $OUTPUT_EXEC_DIR '{}' \;

  mv .newbuilddatedb .builddatedb
elif [ $1 = 'test' ]; then
  ./test.sh $OUTPUT_EXEC_DIR
elif [ $1 = 'clean' ]; then
  if [ -f "$OUTPUT_EXEC_DIR/$OUTPUT_EXEC_NAME" ]; then
    rm "$OUTPUT_EXEC_DIR/$OUTPUT_EXEC_NAME"
  fi
  if [ -f "$OUTPUT_EXEC_DIR"/*.js ]; then
    rm "$OUTPUT_EXEC_DIR"/*.js
  fi
  if [ -f "$C_OBJECT_DIR"/*.o ]; then
    rm "$C_OBJECT_DIR"/*.o
  fi
else
  echo "unrecognized build option"
fi

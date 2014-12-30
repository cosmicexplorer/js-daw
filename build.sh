#!/bin/bash

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
  # link
  echo -n "linking $OUTPUT_EXEC_DIR/$OUTPUT_EXEC_NAME..."
  clang $C_OBJECT_DIR/*.o -o $OUTPUT_EXEC_DIR/$OUTPUT_EXEC_NAME
  echo "done."

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

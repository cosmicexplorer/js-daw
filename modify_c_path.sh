#!/bin/bash

input_filename=$(echo $3 | sed -e 's/^.*\///g')
output_filename=$(echo $input_filename | sed -e 's/\.c$/\.o/g')

do_compile="$(./needs_compile.sh $2/$output_filename $3)"

if [ $do_compile = true ]; then
  echo $3:$cur_date >> .newbuilddatedb
  echo "compiling $3..."
  clang -c $3 $1 -o "$2/$output_filename"
fi

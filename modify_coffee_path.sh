#!/bin/bash

input_filename=$(echo $2 | sed -e 's/^.*\///g')
output_filename=$(echo $input_filename | sed -e 's/\.coffee$/\.js/g')

do_compile="$(./needs_compile.sh $1/$output_filename $2)"

if [ $do_compile = "true" ]; then
  echo -n "compiling $2..."
  coffee -o $1 -c $2
  echo "done."
fi

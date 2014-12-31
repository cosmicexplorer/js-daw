#!/bin/bash

input_dir=$(dirname $3)
input_filename=$(echo $3 | sed -e 's/^.*\///g')
output_filename=$(echo $input_filename | sed -e 's/\.c$/\.o/g')

do_compile="$(./needs_compile.sh $2/$output_filename $3)"

if [ $do_compile = true ]; then
  echo $3:$cur_date >> .newbuilddatedb
  echo "compiling $3..."
  prev_dir=$(pwd)
  cd $(dirname $3)
  clang -c $input_filename $1 -o "$output_filename"
  if [ $? -ne 0 ]; then
    touch $prev_dir/.error
  fi
  if [ -f $output_filename ]; then
    mv $output_filename "$prev_dir/$2"
  fi
  cd $prev_dir
fi

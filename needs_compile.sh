#!/bin/bash

# "$1": output file path
# $2: input file path

do_compile="false"

cur_date=$(stat -c %Y "$2")
if [ ! -f "$1" ]; then
  do_compile="true"
else
  if [ -f ".builddatedb" ]; then
    prev_date=$(grep -Po "(?<=$2:).*$" ".builddatedb" | head -n1) # first match
    if [ "$prev_date" = "" ]; then
      do_compile="true"
    elif [ "$prev_date" -lt "$cur_date" ]; then
      do_compile="true"
    else
      do_compile="false"          # not required but kepy for clarity
      echo "$2:$prev_date" >> .newbuilddatedb
    fi
  else
    do_compile="true"
  fi
fi

if [ "$do_compile" = "true" ]; then
  echo "$2:$cur_date" >> .newbuilddatedb
else
  echo "$2:$prev_date" >> .newbuilddatedb
fi

echo "$do_compile"

#!/usr/bin/env bash
include_dirs=(
  # "./static/"
)

STD=gnu11
# STD=c11
# STD=c99


checksOn=(
  -fmax-errors=3
  -Werror
  -Wall
  -Wextra
  -pedantic
  -Wpedantic
  -pedantic-errors
  -Wformat
  -Wmissing-include-dirs

  -fstrict-aliasing
  -Wstrict-aliasing

  -fstrict-overflow
  -Wstrict-overflow

  -Wlogical-op

  -fstack-protector
  -Wstack-protector
)

checksOff=(
  # -fno-strict-aliasing
  # -Wno-nested-externs
  # -fno-stack-protector
)

lib=(
  -lm
  -lpthread 
)


file=$1


args=""
shift
while [ -n "$1" ]
do
  args="$args $1" 
  shift
done


include_dirs_opt=""
for dir in ${include_dirs[@]}
do
  include_dirs_opt="$include_dirs_opt -I $dir"
done

cc_args=(
  -O2
  -std=$STD
  ${include_dirs_opt[@]}
  ${checksOn[@]}
  ${checksOff[@]}
  $args
  ${lib[@]}
  $file
)

# echo ${cc_args[@]}

cc ${cc_args[@]}


if [ "$?" = "0" ]; then
    ./a.out
fi


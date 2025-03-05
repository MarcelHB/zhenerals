#!/usr/bin/bash

files=$(find . -name '*.vert' -o -name '*.frag' -o -name '*.comp')

for f in $files; do
  if [[ $f -nt $f.spv ]] || ! [[ -f $f.spv ]]; then
    glslc "$f" -o"$f.spv"
  fi
done

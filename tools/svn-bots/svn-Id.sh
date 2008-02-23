#!/bin/sh
find . -type f -a '(' -path '*/branches/*' -path '*/branches*' -prune -o -path '*/.*' -prune -o -print ')' | while read file; do
    if grep -q '\$Id:' "$file" && ! svn propget svn:keywords "$file" | grep -q '^Id$'; then svn propset svn:keywords Id "$file"; fi
  done


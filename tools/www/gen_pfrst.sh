#!/bin/bash

cat html/header.html
rst2html.py "$1" | sed -e '1,7d'
cat html/footer.html


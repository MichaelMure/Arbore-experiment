#!/bin/bash

# pfnet
rm -f out.dot
tools/stats/dep-graph/dep-graph.py $(find common/ -name \*.h) $(find common/ -name \*.cpp) $(find peerfuse-net/ -name \*.h) $(find peerfuse-net/ -name \*.cpp)
neato -Tpng -o pfnet-deps.png < out.dot

# pflan
rm -f out.dot
tools/stats/dep-graph/dep-graph.py $(find common/ -name \*.h) $(find common/ -name \*.cpp) $(find peerfuse-lan/ -name \*.h) $(find peerfuse-lan/ -name \*.cpp)
neato -Tpng -o pflan-deps.png < out.dot

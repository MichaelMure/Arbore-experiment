#!/bin/bash
# Copyright(C) 2008 Laurent Defert, Romain Bignon
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# $Id$
#


# pfnet
rm -f out.dot
tools/stats/dep-graph/dep-graph.py $(find common/ -name \*.h) $(find common/ -name \*.cpp) $(find peerfuse-net/ -name \*.h) $(find peerfuse-net/ -name \*.cpp)
neato -Tpng -o pfnet-deps.png < out.dot

# pflan
rm -f out.dot
tools/stats/dep-graph/dep-graph.py $(find common/ -name \*.h) $(find common/ -name \*.cpp) $(find peerfuse-lan/ -name \*.h) $(find peerfuse-lan/ -name \*.cpp)
neato -Tpng -o pflan-deps.png < out.dot

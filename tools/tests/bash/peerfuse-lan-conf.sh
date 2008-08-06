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


function build_conf
{
	if [ "$LISTEN_PORT" == "" ]
	then
		LISTEN_PORT=9876
	fi

	if [ "$CONNECT_PORT" == "" ]
	then
		CONNECT_PORT=6789
	fi

	if [ "$XMLRPC_PORT" == "" ]
	then
		XMLRPC_PORT=1772
	fi

	if [ "$CACHE_ROOT" == "" ]
	then
		exit 1
	fi
	if [ "$CERT" == "" ]
	then
		CERT=key${NB_P2PFS}.crt
	fi

	if [ "$KEY" == "" ]
	then
		KEY=key${NB_P2PFS}.key
	fi

	echo "
###########################
#   ___ ___ ___  __       #
#  | _ \_  ) _ \/ _|___   #
#  |  _// /|  _/  _(_-<   #
#  |_| /___|_| |_| /__/   #
#                         #
###########################

# This section describes listening
listen {

	# Listen on this ip.
	# 0.0.0.0 means listen everywhere
	bind = 0.0.0.0

	# Port to listen
	port = $LISTEN_PORT

}

# Connection to an other peer
connection {

	host = localhost

	port = $CONNECT_PORT
}

ssl {

	ca = $TESTS/conf/certs/ca.crt

	key = $TESTS/conf/certs/$KEY

	cert = $TESTS/conf/certs/$CERT

	disable_crl = true
}

# Harddisk configuration
hdd {

	root = $CACHE_ROOT

	workdir = ${!CONFDIR}
}

# Xmlrpc configuration
xmlrpc {

	bind = 127.0.0.1

	port = $XMLRPC_PORT

}

# Log function
logging {

	# What do you want to log
	#       DEBUG       :Debug informations (discouraged)
	#       PARSE       :Parse informations (discouraged)
	#       DESYNCH     :Desynchronizations
	#       WARNING     :Warnings
	#       BIGWARNING  :Big Warning (encouraged)
	#       ERR         :Errors
	#       CONNEC      :Connections/disconnections
	#       ALL         :Show all infos
	# You can put several logging level on the same line.

	level = ALL

	to_syslog = false
}
	" > "$1"
}

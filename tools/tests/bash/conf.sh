#!/bin/bash

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
	
	if [ "$CACHE_ROOT" == "" ]
	then
		exit 1
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

# Harddisk configuration
hdd {

	root = $CACHE_ROOT

	workdir = ${!CONFDIR}
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


}
	" > "$1"
}

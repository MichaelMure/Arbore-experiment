#!/bin/bash

dot -Tpng cert_request.dot > cert_request.png
dot -Tpng peer_connection.dot > peer_connection.png
dot -Tpng netmerge.dot > netmerge.png


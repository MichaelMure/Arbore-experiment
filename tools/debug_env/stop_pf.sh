#!/bin/bash
for pf_mount in pf*
do
	fusermount -u $pf_mount/mount
done

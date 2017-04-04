#!/bin/sh

#set -x

nice git pull  --commit --no-edit | grep 'Already up-to-date' || {
	rm -f ninslash_srv
	nice bam server_release
	[ -e ninslash_srv ] && mv -f ninslash_srv ninslash_srv_run
}

CONFIG=/ctf-
while echo "$CONFIG" | grep '/ctf-' ; do
CONFIG=`find "example configs" -maxdepth 1 -mindepth 1 -type f -print0 \
		| sort --zero-terminated --random-sort \
		| sed 's/\d000.*//g'`
echo CONFIG $CONFIG
done

echo CONFIG $CONFIG

./ninslash_srv_run -f "$CONFIG" 'sv_port 21501' 'sv_name "'"`hostname | sed 's/oa-//'` - vote server"'"'


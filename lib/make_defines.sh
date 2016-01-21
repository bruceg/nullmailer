#!/bin/sh
(
	echo "extern const char QUEUE_DIR[]=\"$1\";"
	echo "extern const char CONFIG_DIR[]=\"$2\";"
	echo "extern const char PROTOCOLS_DIR[]=\"$3\";"
	echo "extern const char BIN_DIR[]=\"$4\";"
	echo "extern const char SBIN_DIR[]=\"$5\";"
) > defines.cc

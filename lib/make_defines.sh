#!/bin/sh
echo "const char* QUEUE_DIR=\"$1/\";"			>defines.cc
echo "const char* QUEUE_TMP_DIR=\"$1/tmp/\";"		>>defines.cc
echo "const char* QUEUE_MSG_DIR=\"$1/queue/\";"		>>defines.cc
echo "const char* QUEUE_TRIGGER=\"$1/trigger\";"	>>defines.cc
echo "const char* CONFIG_DIR=\"$2/\";"			>>defines.cc
echo "const char* PROTOCOL_DIR=\"$3/\";"		>>defines.cc
echo "const char* BIN_DIR=\"$4/\";"			>>defines.cc
echo "const char* SBIN_DIR=\"$5/\";"			>>defines.cc

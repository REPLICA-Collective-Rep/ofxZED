# Attempt to load a config.make file.
# If none is found, project defaults in config.project.make will be used.

LIB=-L/usr/local/cuda-10.0/lib64/libnppc.so.10.0 -L/usr/local/zed/lib/libsl_core.so -L/usr/local/zed/lib/libsl_input.so -L/usr/local/zed/lib/libsl_svo.so -L/usr/local/zed/lib/libsl_zed.so
INC=-I/usr/local/cuda-10.0/include -I/usr/local/zed/include

ifneq ($(wildcard config.make),)
	include config.make
endif

# make sure the the OF_ROOT location is defined
ifndef OF_ROOT
	OF_ROOT=$(realpath ../../..)
endif

# call the project makefile!
include $(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk

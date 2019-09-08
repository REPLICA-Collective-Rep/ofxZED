# All variables and this file are optional, if they are not present the PG and the
# makefiles will try to parse the correct values from the file system.
#
# Variables that specify exclusions can use % as a wildcard to specify that anything in
# that position will match. A partial path can also be specified to, for example, exclude
# a whole folder from the parsed paths from the file system
#
# Variables can be specified using = or +=
# = will clear the contents of that variable both specified from the file or the ones parsed
# from the file system
# += will add the values to the previous ones in the file or the ones parsed from the file 
# system
# 
# The PG can be used to detect errors in this file, just create a new project with this addon 
# and the PG will write to the console the kind of error and in which line it is

meta:
	ADDON_NAME = ofxZED
	ADDON_DESCRIPTION = Ubuntu 18.04 / Windows 10 implementation of ZED Camera SDK 2.8 with CUDA 10.0
	ADDON_AUTHOR = @autr
	ADDON_TAGS = "addon" "video" "camera" "computervision" "ml"
	ADDON_URL = http://github.com/autr/ofxZED

common:
	# dependencies with other addons, a list of them separated by spaces 
	# or use += in several lines
	# ADDON_DEPENDENCIES =
	
	# include search paths, this will be usually parsed from the file system
	# but if the addon or addon libraries need special search paths they can be
	# specified here separated by spaces or one per line using +=
	# ADDON_INCLUDES =
	
	# any special flag that should be passed to the compiler when using this
	# addon
	# ADDON_CFLAGS =
	
	# any special flag that should be passed to the compiler for c++ files when 
	# using this addon
	# ADDON_CPPFLAGS =
	
	# any special flag that should be passed to the linker when using this
	# addon, also used for system libraries with -lname
	# ADDON_LDFLAGS =
	
	# source files, these will be usually parsed from the file system looking
	# in the src folders in libs and the root of the addon. if your addon needs
	# to include files in different places or a different set of files per platform
	# they can be specified here
	# ADDON_SOURCES =
	
	# source files that will be included as C files explicitly
	# ADDON_C_SOURCES = 
	
	# source files that will be included as header files explicitly
	# ADDON_HEADER_SOURCES = 
	
	# source files that will be included as c++ files explicitly
	# ADDON_CPP_SOURCES = 
	
	# source files that will be included as objective c files explicitly
	# ADDON_OBJC_SOURCES = 
	
	# derines that will be passed to the compiler when including this addon
	# ADDON_DEFINES
	
	# some addons need resources to be copied to the bin/data folder of the project
	# specify here any files that need to be copied, you can use wildcards like * and ?
	# ADDON_DATA = 
	
	# when parsing the file system looking for libraries exclude this for all or
	# a specific platform
	# ADDON_LIBS_EXCLUDE =
	
	# binary libraries, these will be usually parsed from the file system but some 
	# libraries need to passed to the linker in a specific order/
	# 
	# For example in the ofxOpenCV addon we do something like this:
	#
	# ADDON_LIBS =
	# ADDON_LIBS += libs/opencv/lib/linuxarmv6l/libopencv_legacy.a
	# ADDON_LIBS += libs/opencv/lib/linuxarmv6l/libopencv_calib3d.a
	# ...
	
	
linux64:
	# linux only, any library that should be included in the project using
	# pkg-config
	# ADDON_PKG_CONFIG_LIBRARIES =
	ADDON_INCLUDES += /usr/local/cuda-10.0/include
	ADDON_INCLUDES += /usr/local/zed/include


	ADDON_LIBS += /usr/local/cuda-10.0/lib64/libnppc.so.10.0
	ADDON_LIBS += /usr/local/zed/lib/libsl_core.so
	ADDON_LIBS += /usr/local/zed/lib/libsl_input.so
	ADDON_LIBS += /usr/local/zed/lib/libsl_svo.so
	ADDON_LIBS += /usr/local/zed/lib/libsl_zed.so
vs:
	# After compiling copy the following dynamic libraries to the executable directory
	# only windows visual studio
	# ADDON_DLLS_TO_COPY = 
	ADDON_INCLUDES += "C:/Program Files (x86)/ZED SDK/include"
# 	ADDON_CFLAGS += -I"C:/Program Files (x86)/ZED SDK/include"
	ADDON_INCLUDES += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/include"
# 	ADDON_CFLAGS += -I"C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/include"
	ADDON_LIBS += "C:/Program Files (x86)/ZED SDK/lib/sl_core64.lib"
	ADDON_LIBS += "C:/Program Files (x86)/ZED SDK/lib/sl_input64.lib"
	ADDON_LIBS += "C:/Program Files (x86)/ZED SDK/lib/sl_zed64.lib"
# 	ADDON_LDFLAGS += -L"C:/Program Files (x86)/ZED SDK/lib"
# 	ADDON_LDFLAGS += -L"C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/Win32"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/OpenCL.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/cublas.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/cuda.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/cudadevrt.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/cudart.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/cudart_static.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/cufft.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/cufftw.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/curand.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/cusolver.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/cusparse.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/nppc.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/nppial.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/nppicc.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/nppicom.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/nppidei.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/nppif.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/nppig.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/nppim.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/nppist.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/nppisu.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/nppitc.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/npps.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/nvblas.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/nvgraph.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/nvml.lib"
	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/nvrtc.lib"

# 	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/Win32/cuda.lib"
# 	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/Win32/cudadevrt.lib"
# 	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/Win32/cudart.lib"
# 	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/Win32/cudart_static.lib"
# 	ADDON_LIBS += "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/Win32/OpenCL.lib"
linuxarmv6l:
linuxarmv7l:
android/armeabi:	
android/armeabi-v7a:	
osx:
	# osx/iOS only, any framework that should be included in the project
	# ADDON_FRAMEWORKS =
ios:

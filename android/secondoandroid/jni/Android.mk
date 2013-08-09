LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := app

LOCAL_SRC_FILES := secondo_jni.cpp ListOutput.cpp JNITool.cpp

LOCAL_LDLIBS := -L../lib/ -L$(HOME)/jpeg-6b -L$(HOME)/git/secondo2/secondo2/libs  -L$(HOME)/bison/lib -L$(HOME)/flex/lib -L$(HOME)/secondo/android/lib/ -lsecondo -lfl  -ly  -L$(HOME)/android-ndk/sources/cxx-stl/gnu-libstdc++/4.7/libs/armeabi -lgnustl_static -ljpeg   -ldl -L$(HOME)/BDB/lib -L$(HOME)/gsl/lib -lgslcblas -lgsl -l$(HOME)/BDB/lib/libdb_cxx.a -llog

LOCAL_CFLAGS := -ggdb -I$(HOME)/secondo/Tools/ListsAndTables -I. -I$(HOME)/secondo/include -Wall -fexceptions -frtti  -D_REENTRANT -DNL_PERSISTENT -DUSE_PROGRESS -DUSE_SERIALIZATION -I$(HOME)/BDB/include -I../include -I$(HOME)/BDB/include -I$(HOME)/android-ndk/sources/cxx-stl/gnu-libstdc++/4.8/include -I$(HOME)/android-ndk-r8d/sources/cxx-stl/gnu-libstdc++/4.7/libs/armeabi/include


include $(BUILD_SHARED_LIBRARY)


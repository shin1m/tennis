LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SOURCE_PATH := ../../../..

LOCAL_CPPFLAGS := -std=c++1y -frtti -fexceptions -O3

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../SDL2/include \
	$(LOCAL_PATH)/../SDL2_image \
	$(LOCAL_PATH)/../SDL2_ttf \
	$(LOCAL_PATH)/../SDL2_mixer \
	$(LOCAL_PATH)/../xml2/include \
	$(LOCAL_PATH)/$(SOURCE_PATH)

LOCAL_SRC_FILES := ../SDL2/src/main/android/SDL_android_main.c \
	$(SOURCE_PATH)/portable.cc \
	$(SOURCE_PATH)/gl.cc \
	$(SOURCE_PATH)/collada.cc \
	$(SOURCE_PATH)/ball.cc \
	$(SOURCE_PATH)/player.cc \
	$(SOURCE_PATH)/stage.cc \
	$(SOURCE_PATH)/computer.cc \
	$(SOURCE_PATH)/main.cc

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_image SDL2_ttf SDL2_mixer xml2

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)

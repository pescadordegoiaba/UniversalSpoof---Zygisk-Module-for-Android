LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := spoofimei
LOCAL_SRC_FILES := imei_spoofer.cpp

# spoof_ids.cpp entra em conflito com o example.cpp por causa da mesma função
#LOCAL_SRC_FILES := example.cpp, spoof_ids.cpp

# Define C++ moderno
LOCAL_CPPFLAGS := -std=c++17
LOCAL_LDLIBS := -llog -landroid
LOCAL_LDFLAGS := -stdlib=libc++  # garante link com libc++

include $(BUILD_SHARED_LIBRARY)

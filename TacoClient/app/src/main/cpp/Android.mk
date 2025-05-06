LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := mc_widgets
LOCAL_C_INCLUDES := $(LOCAL_PATH)/ImGui $(LOCAL_PATH)/ImGui/backends
LOCAL_SRC_FILES := \
    mc-init.cpp \
    main.cpp \
    ImGui/imgui.cpp \
    ImGui/imgui_draw.cpp \
    ImGui/imgui_widgets.cpp \
    ImGui/imgui_tables.cpp \
    ImGui/backends/imgui_impl_android.cpp \
    ImGui/backends/imgui_impl_opengl3.cpp
LOCAL_LDLIBS := -lGLESv3 -lEGL -llog
include $(BUILD_SHARED_LIBRARY)

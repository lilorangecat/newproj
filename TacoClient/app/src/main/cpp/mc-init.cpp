#include <jni.h>
#include <string>
#include <android/native_activity.h>
#include <android/log.h>
#include <android/input.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "ImGui/imgui.h"
#include "ImGui/backends/imgui_impl_android.h"
#include "ImGui/backends/imgui_impl_opengl3.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "mc-init", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "mc-init", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "mc-init", __VA_ARGS__))

// EGL state
static EGLDisplay g_eglDisplay = EGL_NO_DISPLAY;
static EGLSurface g_eglSurface = EGL_NO_SURFACE;
static EGLContext g_eglContext = EGL_NO_CONTEXT;
static int g_surfaceWidth = 0;
static int g_surfaceHeight = 0;

static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    // Pass input events to ImGui
    ImGui_ImplAndroid_HandleInputEvent(event);
    // Consume the event so it's not passed further (e.g., to Minecraft if it were running)
    return 1;
}

static void handle_cmd(struct android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (app->window != NULL) {
                LOGI("APP_CMD_INIT_WINDOW");
                // Initialize EGL
                g_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
                eglInitialize(g_eglDisplay, 0, 0);

                const EGLint attribs[] = {
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
                    EGL_BLUE_SIZE, 8,
                    EGL_GREEN_SIZE, 8,
                    EGL_RED_SIZE, 8,
                    EGL_ALPHA_SIZE, 8,
                    EGL_DEPTH_SIZE, 16,
                    EGL_NONE
                };
                EGLConfig config;
                EGLint num_configs;
                eglChooseConfig(g_eglDisplay, attribs, &config, 1, &num_configs);

                const EGLint context_attribs[] = {
                    EGL_CONTEXT_CLIENT_VERSION, 3,
                    EGL_NONE
                };
                g_eglContext = eglCreateContext(g_eglDisplay, config, EGL_NO_CONTEXT, context_attribs);

                g_eglSurface = eglCreateWindowSurface(g_eglDisplay, config, app->window, NULL);
                eglMakeCurrent(g_eglDisplay, g_eglSurface, g_eglSurface, g_eglContext);

                eglQuerySurface(g_eglDisplay, g_eglSurface, EGL_WIDTH, &g_surfaceWidth);
                eglQuerySurface(g_eglDisplay, g_eglSurface, EGL_HEIGHT, &g_surfaceHeight);

                // Initialize ImGui
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO();
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
                //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls // FIXME: Android doesn't support gamepad yet

                // Setup Dear ImGui style
                ImGui::StyleColorsDark();
                //ImGui::StyleColorsClassic();

                // Setup Platform/Renderer backends
                ImGui_ImplAndroid_Init(app->window);
                ImGui_ImplOpenGL3_Init("#version 300 es");

                // Load Fonts
                // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
                // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font later.
                // - If you want to use text input, uncomment the Lookup table.
                //io.Fonts->AddFontDefault();
                //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
                //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
                //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
                //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
                //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
                //IM_ASSERT(font != NULL);

                // For example, load a font from assets
                // AAssetManager* mgr = app->activity->assetManager;
                // AAsset* asset = AAssetManager_open(mgr, "myfont.ttf", AASSET_MODE_BUFFER);
                // if (asset) {
                //     const void* data = AAsset_getBuffer(asset);
                //     size_t len = AAsset_getLength(asset);
                //     io.Fonts->AddFontFromMemoryTTF((void*)data, (int)len, 16.0f);
                //     AAsset_close(asset);
                // } else {
                //     LOGW("Could not load font from assets.");
                // }

                ImGui_ImplOpenGL3_CreateFontsTexture();
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            LOGI("APP_CMD_TERM_WINDOW");
            ImGui_ImplOpenGL3_DestroyDeviceObjects();
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplAndroid_Shutdown();
            ImGui::DestroyContext();

            if (g_eglDisplay != EGL_NO_DISPLAY) {
                eglMakeCurrent(g_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                if (g_eglContext != EGL_NO_CONTEXT) {
                    eglDestroyContext(g_eglDisplay, g_eglContext);
                }
                if (g_eglSurface != EGL_NO_SURFACE) {
                    eglDestroySurface(g_eglDisplay, g_eglSurface);
                }
                eglTerminate(g_eglDisplay);
            }
            g_eglDisplay = EGL_NO_DISPLAY;
            g_eglContext = EGL_NO_CONTEXT;
            g_eglSurface = EGL_NO_SURFACE;
            break;
        case APP_CMD_GAINED_FOCUS:
            LOGI("APP_CMD_GAINED_FOCUS");
            break;
        case APP_CMD_LOST_FOCUS:
            LOGI("APP_CMD_LOST_FOCUS");
            break;
        case APP_CMD_CONFIG_CHANGED:
            LOGI("APP_CMD_CONFIG_CHANGED");
            // Window size might have changed
            if (g_eglDisplay != EGL_NO_DISPLAY) {
                eglQuerySurface(g_eglDisplay, g_eglSurface, EGL_WIDTH, &g_surfaceWidth);
                eglQuerySurface(g_eglDisplay, g_eglSurface, EGL_HEIGHT, &g_surfaceHeight);
            }
            break;
        case APP_CMD_SAVE_STATE:
            LOGI("APP_CMD_SAVE_STATE");
            break;
        case APP_CMD_PAUSE:
            LOGI("APP_CMD_PAUSE");
            break;
        case APP_CMD_RESUME:
            LOGI("APP_CMD_RESUME");
            break;
        case APP_CMD_START:
            LOGI("APP_CMD_START");
            break;
        case APP_CMD_STOP:
            LOGI("APP_CMD_STOP");
            break;
        case APP_CMD_DESTROY:
            LOGI("APP_CMD_DESTROY");
            break;
    }
}

extern "C" void android_main(struct android_app* app) {
    // Set the command and input event callbacks
    app->onAppCmd = handle_cmd;
    app->onInputEvent = handle_input;

    // Main loop
    int ident;
    int events;
    struct android_poll_source* source;

    // We are not running Minecraft, so we don't need to load its library.
    // The loop will run until the app is destroyed.
    while (!app->destroyRequested) {
        // Process events
        while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {
            if (source != NULL) {
                source->process(app, source);
            }

            // Check if we are exiting
            if (app->destroyRequested) {
                return;
            }
        }

        // If window is not ready, skip rendering
        if (g_eglDisplay == EGL_NO_DISPLAY) {
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        ImGui::ShowDemoWindow();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &ImGui::GetIO().ShowDemoWindow);      // Edit bool storing whether to show the demo window.

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            //ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, g_surfaceWidth, g_surfaceHeight);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f); // Clear with a background color
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        eglSwapBuffers(g_eglDisplay, g_eglSurface);
    }
}

// ANativeActivity_onCreate is called by the system when the activity is created.
// We don't need to do anything specific here as android_main will handle the setup.
extern "C" void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
    // Set the ANativeActivity's callbacks. These will be called by the system.
    // The android_main function will be called after this returns.
    activity->callbacks->onInputEvent = handle_input;
    activity->callbacks->onAppCmd = handle_cmd;
}

// JNI_OnLoad is called when the native library is loaded.
// We don't need to load libminecraftpe.so anymore.
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    // No specific JNI setup needed for this basic ImGui example.
    // If you need to call Java methods from native code, you would do JNI setup here.
    return JNI_VERSION_1_6;
}

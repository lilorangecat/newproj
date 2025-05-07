package com.tacoclient.launcher
import android.app.Activity
import android.content.Context
import android.graphics.PixelFormat
import android.os.Build
import android.os.Handler
import android.util.Log // Added for better error handling
import android.view.Gravity
import android.view.MotionEvent
import android.view.View
import android.view.WindowManager
import com.tacoclient.launcher.GLES3JNIView // Ensure this is importable

// Define a singleton object to hold the overlay management logic and state
object ImGuiManager {

    // Properties to hold the state, equivalent to static variables in Java
    // Make them nullable as they are initialized later
    var windowManager: WindowManager? = null
    var layoutParams: WindowManager.LayoutParams? = null // Renamed from vParams for clarity
    var touchView: View? = null // Renamed from vTouch for clarity

    // Original unused variables kept for completeness
    var windowManagerOther: WindowManager? = null // Original xfqManager
    var realWidth: Int = 0 // Original 真实宽
    var realHeight: Int = 0 // Original 真实高


    /**
     * Initializes and sets up the ImGui overlay views using WindowManager.
     * This replaces the logic previously in MainActivity.Start().
     *
     * @param activity The Activity context. Must be an Activity to get the WindowManager.
     */
    fun startImGuiOverlay(activity: Activity) {
        // Get the WindowManager from the Activity
        val localWindowManager = activity.windowManager

        // Create layout parameters for the touch view (invisible, for touch) and the display view (for rendering)
        val localTouchLayoutParams = getAttributes(false)
        val localDisplayLayoutParams = getAttributes(true) // This params is for GLES3JNIView

        // Create the views
        val displayView = GLES3JNIView(activity) // The view that renders ImGui
        val localTouchView = View(activity) // An invisible view to capture touches

        // Add the views to the WindowManager
        // Note: Adding to activity.getWindowManager() places views within the activity's window.
        // For a true system-level overlay, you'd use context.getSystemService(WindowManager::class.java)
        // and different LayoutParams types (e.g., TYPE_APPLICATION_OVERLAY).
        // We replicate the original logic here.
        localWindowManager.addView(localTouchView, localTouchLayoutParams)
        localWindowManager.addView(displayView, localDisplayLayoutParams)

        // Assign the successfully created/added resources to the object's properties
        windowManager = localWindowManager
        layoutParams = localTouchLayoutParams
        touchView = localTouchView


        // Set up the touch listener on the touch view
        localTouchView.setOnTouchListener { v, event ->
            val action = event.action
            when (action) {
                MotionEvent.ACTION_MOVE, MotionEvent.ACTION_DOWN, MotionEvent.ACTION_UP -> {
                    // Assuming GLES3JNIView.MotionEventClick is a static/companion method
                    // Pass touch events to the native side (presumably handled by GLES3JNIView)
                    GLES3JNIView.MotionEventClick(action != MotionEvent.ACTION_UP, event.rawX, event.rawY)
                }
                else -> {
                    // Ignore other touch actions
                }
            }
            // Return false so other views below this invisible one might also receive touch events,
            // depending on flags and view hierarchy. Original code returned false.
            false
        }

        // Set up a Handler loop to periodically update the touch view's position/size
        // based on native code information (assuming GLES3JNIView.getWindowRect() is native)
        val handler = Handler()
        val updateLayoutRunnable = object : Runnable {
            override fun run() {
                try {
                    // Get window rect from native code (e.g., ImGui)
                    val rectString = GLES3JNIView.getWindowRect()
                    val rect = rectString.split("|")

                    // Ensure we have enough parts and the variables are initialized
                    if (rect.size == 4 && layoutParams != null && windowManager != null && touchView != null) {
                        // Parse the coordinates and dimensions
                        val x = rect[0].toInt()
                        val y = rect[1].toInt()
                        val width = rect[2].toInt()
                        val height = rect[3].toInt()

                        // Update the layout parameters
                        layoutParams!!.x = x
                        layoutParams!!.y = y
                        layoutParams!!.width = width
                        layoutParams!!.height = height

                        // Update the view layout on the screen
                        windowManager!!.updateViewLayout(touchView!!, layoutParams!!)
                    } else {
                       // Log a warning if the format is unexpected or state is null
                       Log.w("ImGuiManager", "Invalid rect string format or manager/params/view is null: $rectString")
                    }

                } catch (e: NumberFormatException) {
                     Log.e("ImGuiManager", "Error parsing window rect integers: ${e.message}")
                } catch (e: Exception) {
                    // Log any other unexpected exceptions
                    Log.e("ImGuiManager", "Error in update layout loop: ${e.message}", e)
                } finally {
                    // Schedule the runnable to run again after a delay
                    handler.postDelayed(this, 20) // Run every 20 milliseconds
                }
            }
        }

        // Start the handler loop
        handler.postDelayed(updateLayoutRunnable, 20)
    }

    /**
     * Helper function to create WindowManager.LayoutParams with specific flags.
     * This was originally a static method in MainActivity.
     *
     * @param isWindow If true, sets parameters suitable for the rendering window (MATCH_PARENT, not touchable).
     *                 If false, sets parameters suitable for the touch window (0 size, not focusable, touchable).
     * @return Configured WindowManager.LayoutParams.
     */
    private fun getAttributes(isWindow: Boolean): WindowManager.LayoutParams {
        // The original Java code creates params with certain arguments,
        // but then immediately overwrites many of them. We replicate that sequence.

        val params = WindowManager.LayoutParams(
            WindowManager.LayoutParams.WRAP_CONTENT, // Will be overwritten
            WindowManager.LayoutParams.WRAP_CONTENT, // Will be overwritten
            0, // Will be overwritten
            100, // Will be overwritten
            // Original code used TYPE_APPLICATION. This is often used for views within an Activity's window.
            // For a system overlay (outside any specific activity), TYPE_APPLICATION_OVERLAY (API 26+)
            // or older types like TYPE_SYSTEM_ALERT would be used, requiring SYSTEM_ALERT_WINDOW permission.
            // We stick to TYPE_APPLICATION as per the original code's use with activity.windowManager.
            WindowManager.LayoutParams.TYPE_APPLICATION,
            // Flags set in constructor but immediately overwritten below
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE or
                    WindowManager.LayoutParams.FLAG_LAYOUT_IN_OVERSCAN or
                    WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN or
                    WindowManager.LayoutParams.FLAG_SPLIT_TOUCH or
                    WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL or
                    WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE or
                    WindowManager.LayoutParams.FLAG_FULLSCREEN or // Note: FLAG_FULLSCREEN often interacts weirdly with overlay types and layouts.
                    WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS or
                    WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION,
            PixelFormat.TRANSPARENT // Will be overwritten
        )

        // --- Overwrite parameters as per original Java code ---

        // Overwrite flags:
        // Original code sets these specific flags, ignoring most set in the constructor
        params.flags = WindowManager.LayoutParams.FLAG_FULLSCREEN or
                WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS or
                WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION or
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE // Keep not focusable

        // Conditionally add more flags based on whether it's the display window
        if (isWindow) {
            // The display window should not be touchable and should not consume touch modals
            params.flags = params.flags or
                    WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL or
                    WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE
        } else {
             // The touch window needs to be touchable.
             // The original code adds FLAG_NOT_TOUCHABLE in the constructor then removes it implicitly by overwriting flags.
             // Let's explicitly ensure it's touchable for the !isWindow case if needed,
             // although the overwrite effectively removes FLAG_NOT_TOUCHABLE unless `isWindow` is true.
             // Based on the original logic, the `if (isWindow)` block adds NOT_TOUCHABLE, meaning
             // the !isWindow case (the touch view) *is* touchable. This is correct.
        }


        // Set format (overwrites constructor value)
        params.format = PixelFormat.RGBA_8888 // Setting to RGBA_8888 for potential alpha channel / transparency

        // Set layoutInDisplayCutoutMode for newer Android versions
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            params.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES
        }

        // Set gravity - adjusts the coordinate system origin to top-left
        params.gravity = Gravity.LEFT or Gravity.TOP

        // Set initial x and y coordinates (overwrites constructor values)
        params.x = 0
        params.y = 0

        // Set width and height (overwrites constructor values)
        // The display window takes up the whole screen (MATCH_PARENT)
        // The touch window starts with size 0 and its size/position is updated dynamically
        val size = if (isWindow) WindowManager.LayoutParams.MATCH_PARENT else 0
        params.width = size
        params.height = size

        return params
    }

    // Add a cleanup function (optional but recommended)
    fun stopImGuiOverlay() {
        if (windowManager != null && touchView != null) {
            try {
                windowManager!!.removeView(touchView!!)
                // Assuming the GLES3JNIView was added second and is also present, remove it too
                // Note: The original code didn't show how the GLES3JNIView was referenced for removal.
                // If you need to remove it, you'd need to store its reference as well.
                // Let's assume for now only the touchView is explicitly managed for removal based on original snippet structure.
                // If GLES3JNIView needs removing, you'll need `var displayView: View? = null` and remove it here.

            } catch (e: IllegalArgumentException) {
                // View not found or already removed
                Log.w("ImGuiManager", "View already removed or not attached: ${e.message}")
            } catch (e: Exception) {
                Log.e("ImGuiManager", "Error removing overlay views: ${e.message}", e)
            } finally {
                // Clear references
                windowManager = null
                layoutParams = null
                touchView = null
                // displayView = null // If you added a property for it
            }
        }
        // Also stop the handler? Typically not needed if the activity finishes, as the handler's looper
        // might be tied to the activity's thread. But for a clean stop, you might want to keep a reference
        // to the runnable and handler and call handler.removeCallbacks(runnable).
        // Leaving this out for now to match the original's lack of explicit cleanup.
    }
}

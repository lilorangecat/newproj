package com.tacoclient.launcher

import android.content.Intent
import android.os.Bundle
import androidx.activity.ComponentActivity

class GameActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Launch Minecraft
        val intent = Intent(this, classLoader.loadClass("com.mojang.minecraftpe.Launcher"))
        intent.putStringArrayListExtra("APKS", getIntent().getStringArrayListExtra("APKS"))
        intent.flags = Intent.FLAG_ACTIVITY_NO_ANIMATION
        startActivity(intent)
    }
}

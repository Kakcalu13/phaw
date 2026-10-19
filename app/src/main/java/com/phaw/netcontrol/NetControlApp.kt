package com.phaw.netcontrol

import android.app.Application
import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.Context
import com.phaw.netcontrol.bedtime.BedtimeScheduler
import com.phaw.netcontrol.data.AppPrefs
import com.phaw.netcontrol.router.ZyxelRouter
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.Dispatchers

class NetControlApp : Application() {
    val prefs by lazy { AppPrefs(this) }
    val router by lazy { ZyxelRouter { prefs.routerConfig } }

    /** Outlives screens: used for logout and bedtime changes that must finish after the UI closes. */
    val scope = CoroutineScope(SupervisorJob() + Dispatchers.Default)

    override fun onCreate() {
        super.onCreate()
        getSystemService(NotificationManager::class.java).createNotificationChannel(
            NotificationChannel(CHANNEL_ALERTS, "Bedtime alerts", NotificationManager.IMPORTANCE_DEFAULT).apply {
                description = "Tells you when bedtime couldn't be applied"
            }
        )
        BedtimeScheduler.sync(this)
    }

    companion object {
        const val CHANNEL_ALERTS = "alerts"
    }
}

val Context.app get() = applicationContext as NetControlApp

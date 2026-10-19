package com.phaw.netcontrol.bedtime

import android.Manifest
import android.app.AlarmManager
import android.app.Notification
import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat
import androidx.core.content.ContextCompat
import androidx.work.BackoffPolicy
import androidx.work.Constraints
import androidx.work.CoroutineWorker
import androidx.work.ExistingPeriodicWorkPolicy
import androidx.work.ExistingWorkPolicy
import androidx.work.NetworkType
import androidx.work.OneTimeWorkRequestBuilder
import androidx.work.PeriodicWorkRequestBuilder
import androidx.work.WorkManager
import androidx.work.WorkerParameters
import androidx.work.workDataOf
import com.phaw.netcontrol.NetControlApp
import com.phaw.netcontrol.R
import com.phaw.netcontrol.app
import com.phaw.netcontrol.router.RouterAuthException
import com.phaw.netcontrol.ui.MainActivity
import kotlinx.coroutines.launch
import kotlinx.coroutines.withTimeout
import java.io.IOException
import java.time.ZonedDateTime
import java.util.concurrent.TimeUnit

object BedtimeScheduler {
    private const val PERIODIC = "bedtime-periodic"
    private const val RETRY = "bedtime-retry"

    /** Re-arms the next alarm and the 15-minute safety net from current settings. */
    fun sync(context: Context) {
        val prefs = context.app.prefs
        val wm = WorkManager.getInstance(context)
        val alarms = context.getSystemService(AlarmManager::class.java)
        val pending = alarmIntent(context)

        if (!prefs.bedtimeEnabled || prefs.bedtimeDevices.isEmpty()) {
            alarms.cancel(pending)
            // Keep the worker while the schedule still has devices to release.
            if (prefs.scheduleBlocked.isEmpty()) wm.cancelUniqueWork(PERIODIC) else schedulePeriodic(wm)
            return
        }

        val at = prefs.bedtimeWindow.nextBoundary(ZonedDateTime.now()).toInstant().toEpochMilli()
        val exact = Build.VERSION.SDK_INT < Build.VERSION_CODES.S || alarms.canScheduleExactAlarms()
        if (exact) alarms.setExactAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, at, pending)
        else alarms.setAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, at, pending)
        schedulePeriodic(wm)
    }

    /** Runs enforcement soon in the background, retrying while the router is unreachable. */
    fun enqueueRetry(context: Context, boundary: Boolean) {
        val request = OneTimeWorkRequestBuilder<EnforceWorker>()
            .setInputData(workDataOf(EnforceWorker.KEY_BOUNDARY to boundary))
            .setConstraints(Constraints.Builder().setRequiredNetworkType(NetworkType.CONNECTED).build())
            .setBackoffCriteria(BackoffPolicy.LINEAR, 1, TimeUnit.MINUTES)
            .build()
        WorkManager.getInstance(context).enqueueUniqueWork(RETRY, ExistingWorkPolicy.REPLACE, request)
    }

    private fun schedulePeriodic(wm: WorkManager) {
        val request = PeriodicWorkRequestBuilder<EnforceWorker>(15, TimeUnit.MINUTES)
            .setConstraints(Constraints.Builder().setRequiredNetworkType(NetworkType.CONNECTED).build())
            .build()
        wm.enqueueUniquePeriodicWork(PERIODIC, ExistingPeriodicWorkPolicy.KEEP, request)
    }

    private fun alarmIntent(context: Context): PendingIntent = PendingIntent.getBroadcast(
        context, 0, Intent(context, BedtimeAlarmReceiver::class.java),
        PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE,
    )

    fun notifyFailure(context: Context, message: String) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU &&
            ContextCompat.checkSelfPermission(context, Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED
        ) return
        val open = PendingIntent.getActivity(
            context, 0, Intent(context, MainActivity::class.java), PendingIntent.FLAG_IMMUTABLE,
        )
        val notification: Notification = NotificationCompat.Builder(context, NetControlApp.CHANNEL_ALERTS)
            .setSmallIcon(R.drawable.ic_stat_moon)
            .setContentTitle("Bedtime wasn't applied")
            .setContentText(message)
            .setStyle(NotificationCompat.BigTextStyle().bigText(message))
            .setContentIntent(open)
            .setAutoCancel(true)
            .build()
        NotificationManagerCompat.from(context).notify(1, notification)
    }
}

/** Fires at bedtime start and end. */
class BedtimeAlarmReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context, intent: Intent) {
        val result = goAsync()
        context.app.scope.launch {
            try {
                withTimeout(45_000) { BedtimeEnforcer.enforce(context) }
            } catch (e: Exception) {
                BedtimeScheduler.enqueueRetry(context, boundary = true)
            } finally {
                BedtimeScheduler.sync(context)
                result.finish()
            }
        }
    }
}

class BootReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context, intent: Intent) {
        // Application.onCreate already re-armed the alarm; catch up in case bedtime started while off.
        BedtimeScheduler.enqueueRetry(context, boundary = false)
    }
}

class EnforceWorker(context: Context, params: WorkerParameters) : CoroutineWorker(context, params) {
    override suspend fun doWork(): Result = try {
        BedtimeEnforcer.enforce(applicationContext)
        Result.success()
    } catch (e: RouterAuthException) {
        BedtimeScheduler.notifyFailure(applicationContext, e.message ?: "Router login failed.")
        Result.failure()
    } catch (e: IOException) {
        if (runAttemptCount < MAX_ATTEMPTS) {
            Result.retry()
        } else {
            if (inputData.getBoolean(KEY_BOUNDARY, false)) {
                BedtimeScheduler.notifyFailure(
                    applicationContext,
                    "Couldn't reach the router. Is this phone on the home Wi-Fi? (${e.message})",
                )
            }
            Result.failure()
        }
    }

    companion object {
        const val KEY_BOUNDARY = "boundary"
        private const val MAX_ATTEMPTS = 5
    }
}

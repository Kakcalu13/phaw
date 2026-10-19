package com.phaw.netcontrol.bedtime

import android.content.Context
import com.phaw.netcontrol.app
import java.time.ZonedDateTime

/**
 * Makes the router match the bedtime schedule. Idempotent, so alarms, the periodic
 * worker and the UI can all call it.
 *
 * Only devices the schedule itself blocked are released in the morning; a device the
 * parent blocked by hand stays blocked.
 */
object BedtimeEnforcer {

    suspend fun enforce(context: Context, now: ZonedDateTime = ZonedDateTime.now()) {
        val prefs = context.app.prefs
        val router = context.app.router
        val nowMillis = now.toInstant().toEpochMilli()

        val active = prefs.bedtimeEnabled && now.toLocalTime() in prefs.bedtimeWindow
        val wanted = if (active) prefs.bedtimeDevices.filter { prefs.exemptUntil(it) <= nowMillis }.toSet() else emptySet()
        val toRelease = prefs.scheduleBlocked - wanted
        if (wanted.isEmpty() && toRelease.isEmpty()) return

        try {
            val activeRules = router.rules().filter { it.active }.map { it.mac }.toSet()
            for (mac in wanted - activeRules) {
                router.setBlocked(mac, true)
                prefs.scheduleBlocked += mac
            }
            for (mac in toRelease) {
                router.setBlocked(mac, false)
                prefs.scheduleBlocked -= mac
            }
        } finally {
            router.logout()
        }
    }
}

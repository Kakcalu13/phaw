package com.phaw.netcontrol.bedtime

import java.time.LocalTime
import java.time.ZonedDateTime

/** Daily window in minutes after midnight; may cross midnight (22:00 -> 07:00). */
data class BedtimeWindow(val startMinute: Int, val endMinute: Int) {

    operator fun contains(time: LocalTime): Boolean {
        val m = time.hour * 60 + time.minute
        return when {
            startMinute < endMinute -> m in startMinute until endMinute
            startMinute > endMinute -> m >= startMinute || m < endMinute
            else -> false
        }
    }

    fun nextStart(now: ZonedDateTime): ZonedDateTime = nextAt(now, startMinute)

    fun nextEnd(now: ZonedDateTime): ZonedDateTime = nextAt(now, endMinute)

    /** The next moment the window opens or closes. */
    fun nextBoundary(now: ZonedDateTime): ZonedDateTime = minOf(nextStart(now), nextEnd(now))

    private fun nextAt(now: ZonedDateTime, minute: Int): ZonedDateTime {
        val today = now.toLocalDate().atTime(minute / 60, minute % 60).atZone(now.zone)
        return if (today.isAfter(now)) today else today.plusDays(1)
    }
}

fun formatMinute(minute: Int): String = "%02d:%02d".format(minute / 60, minute % 60)

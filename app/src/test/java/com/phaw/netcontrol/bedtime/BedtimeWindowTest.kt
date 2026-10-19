package com.phaw.netcontrol.bedtime

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.time.LocalTime
import java.time.ZoneId
import java.time.ZonedDateTime

class BedtimeWindowTest {
    private val night = BedtimeWindow(22 * 60, 7 * 60)
    private val zone = ZoneId.of("Asia/Kolkata")

    @Test
    fun overnightWindow() {
        assertFalse(LocalTime.of(21, 59) in night)
        assertTrue(LocalTime.of(22, 0) in night)
        assertTrue(LocalTime.of(2, 30) in night)
        assertTrue(LocalTime.of(6, 59) in night)
        assertFalse(LocalTime.of(7, 0) in night)
    }

    @Test
    fun sameDayWindow() {
        val afternoon = BedtimeWindow(13 * 60, 15 * 60)
        assertTrue(LocalTime.of(14, 0) in afternoon)
        assertFalse(LocalTime.of(15, 0) in afternoon)
        assertFalse(LocalTime.of(12, 59) in afternoon)
    }

    @Test
    fun emptyWindowNeverActive() {
        assertFalse(LocalTime.of(22, 0) in BedtimeWindow(22 * 60, 22 * 60))
    }

    @Test
    fun nextBoundary() {
        val evening = ZonedDateTime.of(2026, 10, 16, 20, 0, 0, 0, zone)
        assertEquals(ZonedDateTime.of(2026, 10, 16, 22, 0, 0, 0, zone), night.nextBoundary(evening))

        val lateNight = ZonedDateTime.of(2026, 10, 16, 23, 0, 0, 0, zone)
        assertEquals(ZonedDateTime.of(2026, 10, 17, 7, 0, 0, 0, zone), night.nextBoundary(lateNight))

        // Exactly at the start: the next boundary is the end, not the same instant.
        val atStart = ZonedDateTime.of(2026, 10, 16, 22, 0, 0, 0, zone)
        assertEquals(ZonedDateTime.of(2026, 10, 17, 7, 0, 0, 0, zone), night.nextBoundary(atStart))
    }
}

package com.phaw.netcontrol.ui

import org.junit.Assert.assertEquals
import org.junit.Test

class DeviceKindTest {
    @Test
    fun guessesFromRouterNames() {
        assertEquals(DeviceKind.PHONE, kindOf("iPhone"))
        assertEquals(DeviceKind.PHONE, kindOf("Sasangi-s-Galaxy-M31"))
        assertEquals(DeviceKind.PHONE, kindOf("Sa-s-S24"))
        assertEquals(DeviceKind.TABLET, kindOf("Kid's iPad"))
        assertEquals(DeviceKind.LAPTOP, kindOf("bawng-XPS-15-9550"))
        assertEquals(DeviceKind.UNKNOWN, kindOf("Unknown device Unknown_64:16:66:44:b0:7e"))
    }
}

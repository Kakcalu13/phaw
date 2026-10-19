package com.phaw.netcontrol.data

import android.content.Context
import android.content.SharedPreferences
import androidx.core.content.edit
import androidx.security.crypto.EncryptedSharedPreferences
import androidx.security.crypto.MasterKey
import com.phaw.netcontrol.bedtime.BedtimeWindow
import com.phaw.netcontrol.router.RouterConfig

class AppPrefs(context: Context) {
    private val plain: SharedPreferences = context.getSharedPreferences("netcontrol", Context.MODE_PRIVATE)
    private val secure: SharedPreferences = openSecure(context)

    var host: String
        get() = plain.getString("host", "192.168.1.1")!!
        set(v) = plain.edit { putString("host", v.trim()) }

    var username: String
        get() = plain.getString("username", "admin")!!
        set(v) = plain.edit { putString("username", v.trim()) }

    var password: String
        get() = secure.getString("password", "")!!
        set(v) = secure.edit { putString("password", v) }

    val routerConfig get() = RouterConfig(host, username, password)

    /** Last name the router reported, so the title shows it before the first refresh. */
    var routerTitle: String?
        get() = plain.getString("router_title", null)
        set(v) = plain.edit { putString("router_title", v) }

    var sortOrder: String
        get() = plain.getString("sort_order", "NAME")!!
        set(v) = plain.edit { putString("sort_order", v) }

    fun nickname(mac: String): String? = plain.getString("nick_$mac", null)

    fun setNickname(mac: String, name: String) = plain.edit(commit = true) {
        if (name.isBlank()) remove("nick_$mac") else putString("nick_$mac", name.trim())
    }

    /** Router name and IP from the last time the device was connected; paused devices drop off the router's list. */
    fun lastSeen(mac: String): Pair<String, String>? =
        plain.getString("seen_$mac", null)?.split('\n', limit = 2)?.takeIf { it.size == 2 }?.let { it[0] to it[1] }

    fun setLastSeen(mac: String, name: String, ip: String) {
        if (lastSeen(mac) != (name to ip)) plain.edit { putString("seen_$mac", "$name\n$ip") }
    }

    var bedtimeEnabled: Boolean
        get() = plain.getBoolean("bedtime_enabled", true)
        set(v) = plain.edit { putBoolean("bedtime_enabled", v) }

    var bedtimeWindow: BedtimeWindow
        get() = BedtimeWindow(plain.getInt("bedtime_start", 22 * 60), plain.getInt("bedtime_end", 7 * 60))
        set(v) = plain.edit { putInt("bedtime_start", v.startMinute); putInt("bedtime_end", v.endMinute) }

    /** Devices that go offline during bedtime. */
    var bedtimeDevices: Set<String>
        get() = plain.getStringSet("bedtime_devices", emptySet())!!.toSet()
        set(v) = plain.edit { putStringSet("bedtime_devices", v.toSet()) }

    /** Devices the bedtime schedule blocked, so it knows what to release in the morning. */
    var scheduleBlocked: Set<String>
        get() = plain.getStringSet("schedule_blocked", emptySet())!!.toSet()
        set(v) = plain.edit { putStringSet("schedule_blocked", v.toSet()) }

    /** Epoch millis until which a device is let off tonight's bedtime (parent unblocked it by hand). */
    fun exemptUntil(mac: String): Long = plain.getLong("exempt_$mac", 0L)

    fun setExemptUntil(mac: String, epochMillis: Long?) = plain.edit {
        if (epochMillis == null) remove("exempt_$mac") else putLong("exempt_$mac", epochMillis)
    }

    private companion object {
        fun openSecure(context: Context): SharedPreferences {
            fun create() = EncryptedSharedPreferences.create(
                context,
                "netcontrol_secure",
                MasterKey.Builder(context).setKeyScheme(MasterKey.KeyScheme.AES256_GCM).build(),
                EncryptedSharedPreferences.PrefKeyEncryptionScheme.AES256_SIV,
                EncryptedSharedPreferences.PrefValueEncryptionScheme.AES256_GCM,
            )
            return try {
                create()
            } catch (e: Exception) {
                // Keystore key lost (e.g. after a restore): start over; the user re-enters the password.
                context.deleteSharedPreferences("netcontrol_secure")
                create()
            }
        }
    }
}

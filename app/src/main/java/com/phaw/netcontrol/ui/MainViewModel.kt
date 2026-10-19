package com.phaw.netcontrol.ui

import android.app.Application
import android.net.ConnectivityManager
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.phaw.netcontrol.app
import com.phaw.netcontrol.bedtime.BedtimeEnforcer
import com.phaw.netcontrol.bedtime.BedtimeScheduler
import com.phaw.netcontrol.bedtime.BedtimeWindow
import com.phaw.netcontrol.router.RouterAuthException
import com.phaw.netcontrol.router.Snapshot
import com.phaw.netcontrol.router.isPrivateMac
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import java.net.Inet4Address
import java.time.LocalTime
import java.time.ZonedDateTime

enum class DeviceKind { PHONE, TABLET, LAPTOP, UNKNOWN }

enum class SortOrder(val label: String) { NAME("Name"), IP("IP address"), PAUSED("Paused first") }

data class DeviceRow(
    val mac: String,
    val kind: DeviceKind,
    val named: Boolean,
    val title: String,
    val routerName: String,
    val ip: String,
    val online: Boolean,
    val blocked: Boolean,
    val privateMac: Boolean,
    val thisPhone: Boolean,
    val onBedtime: Boolean,
)

data class BedtimeUi(val enabled: Boolean, val window: BedtimeWindow, val activeNow: Boolean)

data class UiState(
    val title: String,
    val sort: SortOrder,
    val rows: List<DeviceRow> = emptyList(),
    val loaded: Boolean = false,
    val error: String? = null,
    val authError: Boolean = false,
    val updatedAt: LocalTime? = null,
    val busy: Set<String> = emptySet(),
    val bedtime: BedtimeUi,
    val needsSetup: Boolean,
)

class MainViewModel(application: Application) : AndroidViewModel(application) {
    private val prefs = application.app.prefs
    private val router = application.app.router
    private var snapshot: Snapshot? = null
    private var polling: Job? = null

    private val _state = MutableStateFlow(
        UiState(
            title = prefs.routerTitle ?: DEFAULT_TITLE,
            sort = runCatching { SortOrder.valueOf(prefs.sortOrder) }.getOrDefault(SortOrder.NAME),
            bedtime = bedtimeUi(),
            needsSetup = prefs.password.isEmpty(),
        )
    )
    val state: StateFlow<UiState> = _state

    fun startPolling() {
        if (polling?.isActive == true || prefs.password.isEmpty()) return
        polling = viewModelScope.launch {
            while (isActive) {
                refresh()
                delay(POLL_MS)
            }
        }
    }

    fun stopPolling() {
        polling?.cancel()
        polling = null
        // Free the router's single admin session so a browser can still log in.
        getApplication<Application>().app.scope.launch { router.logout() }
    }

    fun refreshNow() {
        viewModelScope.launch { refresh() }
    }

    fun setSort(order: SortOrder) {
        prefs.sortOrder = order.name
        _state.update { it.copy(sort = order) }
        rebuild()
    }

    suspend fun refresh() {
        try {
            val snap = router.snapshot()
            snapshot = snap
            // Router's own name; else its Wi-Fi name; else a generic title.
            val title = snap.info?.model ?: snap.info?.ssid
            if (title != null && title != prefs.routerTitle) prefs.routerTitle = title
            _state.update { it.copy(title = title ?: prefs.routerTitle ?: DEFAULT_TITLE) }
            _state.update { it.copy(loaded = true, error = null, authError = false, updatedAt = LocalTime.now()) }
            rebuild()
        } catch (e: RouterAuthException) {
            polling?.cancel()
            _state.update { it.copy(loaded = true, error = e.message, authError = true) }
        } catch (e: Exception) {
            _state.update { it.copy(loaded = true, error = "Can't reach the router: ${e.message ?: e.javaClass.simpleName}") }
        }
    }

    fun setBlocked(mac: String, blocked: Boolean) = launchBusy(mac) {
        router.setBlocked(mac, blocked)
        // A manual change takes the device out of the schedule's hands.
        prefs.scheduleBlocked -= mac
        val bedtime = bedtimeUi()
        if (!blocked && bedtime.activeNow && mac in prefs.bedtimeDevices) {
            prefs.setExemptUntil(mac, prefs.bedtimeWindow.nextEnd(ZonedDateTime.now()).toInstant().toEpochMilli())
        } else if (blocked) {
            prefs.setExemptUntil(mac, null)
        }
    }

    fun setOnBedtime(mac: String, on: Boolean) = launchBusy(mac) {
        prefs.bedtimeDevices = if (on) prefs.bedtimeDevices + mac else prefs.bedtimeDevices - mac
        prefs.setExemptUntil(mac, null)
        applyBedtime()
    }

    fun saveBedtime(enabled: Boolean, window: BedtimeWindow) = launchBusy(BEDTIME) {
        prefs.bedtimeEnabled = enabled
        prefs.bedtimeWindow = window
        prefs.bedtimeDevices.forEach { prefs.setExemptUntil(it, null) }
        applyBedtime()
    }

    fun rename(mac: String, name: String) {
        prefs.setNickname(mac, name)
        rebuild()
    }

    /** Saves router settings after a successful test login. Returns an error message or null. */
    suspend fun saveSettings(host: String, username: String, password: String): String? {
        val old = Triple(prefs.host, prefs.username, prefs.password)
        prefs.host = host
        prefs.username = username
        prefs.password = password
        return try {
            router.testLogin()
            _state.update { it.copy(needsSetup = false, authError = false, error = null) }
            startPolling()
            null
        } catch (e: Exception) {
            prefs.host = old.first
            prefs.username = old.second
            prefs.password = old.third
            e.message ?: e.javaClass.simpleName
        }
    }

    private suspend fun applyBedtime() {
        BedtimeScheduler.sync(getApplication())
        BedtimeEnforcer.enforce(getApplication())
    }

    private fun launchBusy(key: String, block: suspend () -> Unit) {
        viewModelScope.launch {
            _state.update { it.copy(busy = it.busy + key) }
            try {
                block()
                _state.update { it.copy(error = null) }
            } catch (e: Exception) {
                _state.update { it.copy(error = e.message ?: e.javaClass.simpleName) }
            } finally {
                _state.update { it.copy(busy = it.busy - key, bedtime = bedtimeUi()) }
                refresh()
            }
        }
    }

    private fun rebuild() {
        val snap = snapshot ?: return
        val active = snap.rules.filter { it.active }.map { it.mac }.toSet()
        val bedtimeDevices = prefs.bedtimeDevices
        val phoneIps = phoneIps()
        val seen = snap.devices.map { it.mac }.toSet()

        val online = snap.devices.map { d ->
            prefs.setLastSeen(d.mac, d.name, d.ip)
            val nickname = prefs.nickname(d.mac)
            val title = nickname ?: friendlyName(d.name)
            DeviceRow(
                mac = d.mac,
                kind = kindOf("$title ${d.name}"),
                named = nickname != null || title != UNKNOWN,
                title = title,
                routerName = d.name,
                ip = d.ip,
                online = d.online,
                blocked = d.mac in active,
                privateMac = isPrivateMac(d.mac),
                thisPhone = d.ip in phoneIps,
                onBedtime = d.mac in bedtimeDevices,
            )
        }
        // Devices the router knows only through a filter rule or the bedtime list (currently off the network).
        val offline = (snap.rules.map { it.mac } + bedtimeDevices).distinct().filter { it !in seen }.map { mac ->
            val nickname = prefs.nickname(mac)
            val (seenName, seenIp) = prefs.lastSeen(mac) ?: ("" to "")
            val title = nickname ?: seenName.takeIf { it.isNotBlank() }?.let(::friendlyName) ?: OFFLINE
            DeviceRow(
                mac = mac,
                kind = kindOf("$title $seenName"),
                named = nickname != null || (title != UNKNOWN && title != OFFLINE),
                title = title,
                routerName = seenName,
                ip = seenIp,
                online = false,
                blocked = mac in active,
                privateMac = isPrivateMac(mac),
                thisPhone = false,
                onBedtime = mac in bedtimeDevices,
            )
        }
        val sorted = when (_state.value.sort) {
            SortOrder.NAME -> compareBy<DeviceRow>({ !it.thisPhone }, { !it.named }, { it.title.lowercase() })
            SortOrder.IP -> compareBy { ipKey(it.ip) }
            SortOrder.PAUSED -> compareBy({ !it.blocked }, { it.title.lowercase() })
        }
        _state.update { it.copy(rows = (online + offline).sortedWith(sorted), bedtime = bedtimeUi()) }
    }

    private fun bedtimeUi() = BedtimeUi(
        enabled = prefs.bedtimeEnabled,
        window = prefs.bedtimeWindow,
        activeNow = prefs.bedtimeEnabled && LocalTime.now() in prefs.bedtimeWindow,
    )

    private fun phoneIps(): Set<String> {
        val cm = getApplication<Application>().getSystemService(ConnectivityManager::class.java)
        val props = cm.getLinkProperties(cm.activeNetwork) ?: return emptySet()
        return props.linkAddresses.map { it.address }.filterIsInstance<Inet4Address>().mapNotNull { it.hostAddress }.toSet()
    }

    private fun friendlyName(routerName: String) =
        if (routerName.isBlank() || routerName.startsWith("Unknown_")) UNKNOWN else routerName

    private fun ipKey(ip: String): Long =
        ip.split('.').mapNotNull { it.toLongOrNull() }.takeIf { it.size == 4 }
            ?.fold(0L) { acc, part -> acc * 256 + part } ?: Long.MAX_VALUE

    companion object {
        const val DEFAULT_TITLE = "Home network"
        const val UNKNOWN = "Unknown device"
        const val OFFLINE = "Offline device"

        const val BEDTIME = "bedtime"
        private const val POLL_MS = 5_000L
    }
}

/** Best guess from the device's name, for the icon. */
fun kindOf(name: String): DeviceKind {
    val n = name.lowercase()
    return when {
        listOf("ipad", "tab", "tablet", "kindle", "fire").any { it in n } -> DeviceKind.TABLET
        listOf("iphone", "galaxy", "pixel", "phone", "android", "redmi", "oneplus", "oppo", "vivo", "realme", "razer")
            .any { it in n } || Regex("""\bs\d{2}\b|-s\d{2}\b|\bm\d{2}\b""").containsMatchIn(n) -> DeviceKind.PHONE
        listOf("xps", "laptop", "macbook", "thinkpad", "desktop", "pc", "imac", "mac", "surface", "chromebook", "ubuntu", "linux")
            .any { it in n } -> DeviceKind.LAPTOP
        else -> DeviceKind.UNKNOWN
    }
}

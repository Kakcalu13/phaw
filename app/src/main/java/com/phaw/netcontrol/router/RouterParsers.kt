package com.phaw.netcontrol.router

import java.security.MessageDigest

data class Device(
    val name: String,
    val ip: String,
    val mac: String,
    val online: Boolean,
)

data class FilterRule(
    val index: Int,
    val active: Boolean,
    val iface: String,
    val direction: String,
    val mac: String,
)

/** Identity shown on /cgi-bin/statusview.cgi. */
data class RouterInfo(val model: String?, val ssid: String?)

/** MAC filter mode from the router's "Rule Type selection" drop-down. */
enum class FilterMode(val value: Int) {
    DISABLED(0), BLACKLIST(1), WHITELIST(2);

    companion object {
        fun of(value: Int) = entries.firstOrNull { it.value == value }
    }
}

/**
 * Parsers for the HTML pages served by the Zyxel firmware Airtel ships
 * ("Web-Based Configurator"). Pure functions so they can be unit tested
 * against saved pages.
 */
object RouterParsers {
    private val SID = Regex("""var sid = '([0-9a-fA-F]+)'""")
    private val LOCKED_TIME = Regex("""var locked_time = "(\d+)"""")
    private val LOGIN_OK = Regex("""location\.href\s*=\s*"indexmain\.cgi"""")
    private val ROW_SPLIT = Regex("<tr", RegexOption.IGNORE_CASE)
    private val TD = Regex("<td[^>]*>(.*?)</td>", setOf(RegexOption.IGNORE_CASE, RegexOption.DOT_MATCHES_ALL))
    private val TAG = Regex("<[^>]*>")
    private val MAC = Regex("^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$")
    private val SELECTED_OPTION = Regex("""<option\s+value="(\d)"\s+selected""", RegexOption.IGNORE_CASE)

    private val MODEL = Regex("""id='ModelName_ConnectionStatus'></span>:\s*</td>\s*<td[^>]*>\s*([^<]+?)\s*<""")
    private val SSID = Regex("""-\s*SSID:\s*</td>\s*<td[^>]*>\s*([^<]+?)\s*<""")

    fun routerInfo(html: String) = RouterInfo(
        model = MODEL.find(html)?.groupValues?.get(1)?.takeIf { it.isNotBlank() },
        ssid = SSID.find(html)?.groupValues?.get(1)?.takeIf { it.isNotBlank() },
    )

    fun loginSid(html: String): String? = SID.find(html)?.groupValues?.get(1)

    fun isLoginLocked(html: String): Boolean = LOCKED_TIME.containsMatchIn(html)

    /** A successful login answers with a tiny page that redirects to indexmain.cgi. */
    fun isLoginSuccess(html: String): Boolean =
        LOGIN_OK.containsMatchIn(html) && !html.contains("name=\"passWarning\"")

    /** Every protected page answers with this redirect once the session is gone. */
    fun isLoginRedirect(html: String): Boolean =
        html.length < 2000 && html.contains("location.href=\"/cgi-bin/login_advance.cgi\"")

    /**
     * The page hashes `encodeURIComponent(password)` (hex digits lowercased) + ":" + sid with MD5.
     */
    fun passwordHash(password: String, sid: String): String {
        val digest = MessageDigest.getInstance("MD5").digest("${encodeUriComponent(password)}:$sid".toByteArray())
        return digest.joinToString("") { "%02x".format(it) }
    }

    /** JavaScript's encodeURIComponent, with the lowercase hex the router's page uses. */
    fun encodeUriComponent(s: String): String = buildString {
        for (b in s.toByteArray(Charsets.UTF_8)) {
            val c = b.toInt() and 0xff
            val ch = c.toChar()
            if (ch in 'A'..'Z' || ch in 'a'..'z' || ch in '0'..'9' || ch in "-_.!~*'()") append(ch)
            else append('%').append("%02x".format(c))
        }
    }

    /** /cgi-bin/connection_table_list.cgi: icon, name, IPv4, link-local v6, global v6, MAC, reserve. */
    fun devices(html: String): List<Device> = rows(html).mapNotNull { cells ->
        if (cells.size < 6) return@mapNotNull null
        val mac = text(cells[5])
        if (!MAC.matches(mac)) return@mapNotNull null
        Device(
            name = text(cells[1]),
            ip = text(cells[2]),
            mac = mac.uppercase(),
            online = !cells[0].contains("_off", ignoreCase = true),
        )
    }

    /** /cgi-bin/ipMacFilterList.cgi: index, active, interface, direction, MAC. */
    fun filterRules(html: String): List<FilterRule> = rows(html).mapNotNull { cells ->
        if (cells.size < 5) return@mapNotNull null
        val index = text(cells[0]).toIntOrNull() ?: return@mapNotNull null
        val mac = text(cells[4])
        if (!MAC.matches(mac)) return@mapNotNull null
        FilterRule(
            index = index,
            active = text(cells[1]).equals("Yes", ignoreCase = true),
            iface = text(cells[2]),
            direction = text(cells[3]),
            mac = mac.uppercase(),
        )
    }

    /** The selected "Rule Type selection" option on /cgi-bin/IP_MAC_Filter.cgi. */
    fun filterMode(html: String): FilterMode? {
        val start = html.indexOf("name=\"RuleTypeSEL\"")
        if (start < 0) return null
        val end = html.indexOf("</select>", start).takeIf { it > 0 } ?: html.length
        val value = SELECTED_OPTION.find(html.substring(start, end))?.groupValues?.get(1) ?: return null
        return FilterMode.of(value.toInt())
    }

    /** Text label in the rule list -> form value for InterfaceSEL. */
    fun interfaceValue(label: String): String = when (label) {
        "Internet" -> "ppp111"
        "VoIP" -> "nas0_660"
        "Eth_Internet" -> "nas0_notag"
        "Internet_IPoE", "Internet_Bridge" -> "nas0_100"
        "EoGRE_MGMT_IPoE" -> "nas0_1999"
        else -> "br0"
    }

    private fun rows(html: String): List<List<String>> =
        html.split(ROW_SPLIT).map { row -> TD.findAll(row).map { it.groupValues[1] }.toList() }

    private fun text(cell: String): String =
        TAG.replace(cell, "")
            .replace("&nbsp;", " ")
            .replace("&amp;", "&")
            .replace("&lt;", "<")
            .replace("&gt;", ">")
            .replace("&quot;", "\"")
            .replace("&#39;", "'")
            .trim()
}

/** Locally administered bit set: the device is using a private/randomized address. */
fun isPrivateMac(mac: String): Boolean =
    mac.take(2).toIntOrNull(16)?.let { it and 0x02 != 0 } ?: false

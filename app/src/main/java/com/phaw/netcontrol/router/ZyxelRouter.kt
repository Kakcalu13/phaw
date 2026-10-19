package com.phaw.netcontrol.router

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withContext
import okhttp3.Cookie
import okhttp3.CookieJar
import okhttp3.FormBody
import okhttp3.HttpUrl
import okhttp3.OkHttpClient
import okhttp3.Request
import java.io.IOException
import java.security.SecureRandom
import java.security.cert.X509Certificate
import java.util.concurrent.TimeUnit
import javax.net.ssl.SSLContext
import javax.net.ssl.X509TrustManager

data class RouterConfig(val host: String, val username: String, val password: String)

open class RouterException(message: String) : IOException(message)
class RouterAuthException(message: String) : RouterException(message)
private class SessionExpired : IOException("session expired")

data class Snapshot(val devices: List<Device>, val rules: List<FilterRule>, val info: RouterInfo?)

/**
 * Talks to the router's admin web UI the same way its own pages do.
 *
 * The router keeps one stateful admin session (the MAC filter editor selects a rule
 * in one request and saves it in the next), so every operation runs under a mutex.
 */
class ZyxelRouter(private val config: () -> RouterConfig) {
    private val mutex = Mutex()
    private val cookies = MemoryCookieJar()
    private var loggedIn = false
    private var info: RouterInfo? = null

    private val client: OkHttpClient = run {
        // The router serves a self-signed certificate, and this client never talks to anything else.
        val trustAll = object : X509TrustManager {
            override fun checkClientTrusted(chain: Array<X509Certificate>, authType: String) {}
            override fun checkServerTrusted(chain: Array<X509Certificate>, authType: String) {}
            override fun getAcceptedIssuers(): Array<X509Certificate> = emptyArray()
        }
        val ssl = SSLContext.getInstance("TLS").apply { init(null, arrayOf(trustAll), SecureRandom()) }
        OkHttpClient.Builder()
            .sslSocketFactory(ssl.socketFactory, trustAll)
            .hostnameVerifier { _, _ -> true }
            .cookieJar(cookies)
            .connectTimeout(5, TimeUnit.SECONDS)
            .readTimeout(15, TimeUnit.SECONDS)
            .followRedirects(false)
            .build()
    }

    private val base get() = "https://${config().host.trim().removePrefix("https://").removePrefix("http://").trimEnd('/')}"

    suspend fun testLogin() = op { login() }

    suspend fun snapshot(): Snapshot = op {
        authed {
            // The router's identity doesn't change; read it once.
            if (info == null) info = runCatching { RouterParsers.routerInfo(get(STATUS)) }.getOrNull()
            Snapshot(RouterParsers.devices(get(DEVICES)), RouterParsers.filterRules(get(RULES)), info)
        }
    }

    suspend fun rules(): List<FilterRule> = op { authed { RouterParsers.filterRules(get(RULES)) } }

    /** Blocks or unblocks [mac] with the router's MAC filter (Black List mode). */
    suspend fun setBlocked(mac: String, blocked: Boolean) = op {
        authed {
            val target = mac.uppercase()
            val mode = RouterParsers.filterMode(get(FILTER))
            if (mode == FilterMode.WHITELIST) {
                throw RouterException("The router's MAC filter is in White List mode. Switch it to Black List first.")
            }
            val rules = RouterParsers.filterRules(get(RULES))
            val existing = rules.firstOrNull { it.mac == target }
            when {
                existing == null && !blocked -> return@authed
                existing == null -> {
                    if (rules.size >= MAX_RULES) throw RouterException("The router's MAC filter is full ($MAX_RULES rules).")
                    saveRule(index = 0, mac = target, active = true, iface = "br0", direction = "All")
                }
                existing.active == blocked -> return@authed
                else -> {
                    post(FILTER, "RuleTypeChange" to "0", "HiddenRuleTypeIndex" to existing.index.toString())
                    saveRule(
                        index = existing.index,
                        mac = target,
                        active = blocked,
                        iface = RouterParsers.interfaceValue(existing.iface),
                        direction = existing.direction,
                    )
                }
            }
            val after = RouterParsers.filterRules(get(RULES)).firstOrNull { it.mac == target }
            if ((after?.active ?: false) != blocked) throw RouterException("The router didn't apply the change.")
        }
    }

    /** Frees the admin session so the router's web page can still be used from a browser. */
    suspend fun logout() = op {
        if (!loggedIn) return@op
        loggedIn = false
        runCatching { request(Request.Builder().url("$base/cgi-bin/logout_advance.cgi").header("Referer", referer).build()) }
        cookies.clear()
    }

    private fun saveRule(index: Int, mac: String, active: Boolean, iface: String, direction: String) {
        post(
            FILTER,
            "RuleTypeChange" to "1",
            "HiddenRuleTypeIndex" to index.toString(),
            "RuleTypeSEL" to FilterMode.BLACKLIST.value.toString(),
            "RuleActiveRDO_H" to if (active) "Yes" else "No",
            "RuleIndexSEL" to index.toString(),
            "InterfaceSEL" to iface,
            "DirectionSEL" to direction,
            "MacAddrTXT" to mac,
        )
    }

    private suspend fun <T> op(block: () -> T): T = mutex.withLock { withContext(Dispatchers.IO) { block() } }

    private fun <T> authed(block: () -> T): T {
        if (!loggedIn) login()
        return try {
            block()
        } catch (e: SessionExpired) {
            login()
            block()
        }
    }

    private fun login() {
        loggedIn = false
        cookies.clear()
        val cfg = config()
        if (cfg.password.isEmpty()) throw RouterAuthException("Enter the router password in Settings.")
        val loginUrl = "$base/cgi-bin/login_advance.cgi"
        val page = request(Request.Builder().url(loginUrl).build())
        val sid = RouterParsers.loginSid(page) ?: throw RouterException("This doesn't look like the Zyxel router login page.")
        val form = FormBody.Builder()
            .add("Loginuser", cfg.username)
            .add("LoginPasswordValue", RouterParsers.passwordHash(cfg.password, sid))
            .add("submitValue", "1")
            .add("LoginSidValue", sid)
            .add("Prestige_Login", "Login")
            .build()
        val result = request(Request.Builder().url(loginUrl).header("Referer", loginUrl).post(form).build())
        if (!RouterParsers.isLoginSuccess(result)) {
            if (RouterParsers.isLoginLocked(result)) {
                throw RouterAuthException("The router locked logins after failed attempts. Wait a few minutes.")
            }
            throw RouterAuthException("The router rejected the username or password.")
        }
        loggedIn = true
    }

    private fun get(path: String): String =
        checked(request(Request.Builder().url(base + path).header("Referer", referer).build()))

    private fun post(path: String, vararg fields: Pair<String, String>): String {
        val form = FormBody.Builder().apply { fields.forEach { (k, v) -> add(k, v) } }.build()
        return checked(request(Request.Builder().url(base + path).header("Referer", referer).post(form).build()))
    }

    private fun checked(body: String): String {
        if (RouterParsers.isLoginRedirect(body)) {
            loggedIn = false
            throw SessionExpired()
        }
        return body
    }

    private fun request(request: Request): String {
        val req = request.newBuilder().header("Cache-Control", "no-cache").header("Pragma", "no-cache").build()
        client.newCall(req).execute().use { resp ->
            if (resp.code == 403) throw RouterException("The router refused the request (403).")
            if (!resp.isSuccessful) throw RouterException("The router answered HTTP ${resp.code}.")
            return resp.body?.string().orEmpty()
        }
    }

    private val referer get() = "$base/cgi-bin/indexmain.cgi"

    private class MemoryCookieJar : CookieJar {
        private val store = mutableMapOf<String, Cookie>()

        @Synchronized
        override fun saveFromResponse(url: HttpUrl, cookies: List<Cookie>) {
            cookies.forEach { store[it.name] = it }
        }

        @Synchronized
        override fun loadForRequest(url: HttpUrl): List<Cookie> = store.values.filter { it.matches(url) }

        @Synchronized
        fun clear() = store.clear()
    }

    private companion object {
        const val DEVICES = "/cgi-bin/connection_table_list.cgi"
        const val RULES = "/cgi-bin/ipMacFilterList.cgi"
        const val FILTER = "/cgi-bin/IP_MAC_Filter.cgi"
        const val STATUS = "/cgi-bin/statusview.cgi"
        const val MAX_RULES = 32
    }
}

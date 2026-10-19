package com.phaw.netcontrol.router

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test

class RouterParsersTest {

    // Shape of /cgi-bin/connection_table_list.cgi, including the unclosed quote in the reserve checkbox.
    private val connectionTable = """
        <tr align="center">
          <td width="5%" class="top_font">#</td>
          <td  class="top_font"><span id="MLG_Device_Name">Device Name</span></td>
          <td  class="top_font"><span>IP Address</span></td>
          <td  class="top_font">Link-local IPv6 Address</td>
          <td  class="top_font">Global IPv6 Address</td>
          <td  class="top_font"><span>MAC Address</span></td>
          <td  class="top_font"><span>Reserve</span></td>
        </tr>
        <tr align="center">
        <td  class="table_font"><img src="/html/images/network/i_s_pc_on.gif" width="16" height="16" /></td>
        <td  class="table_font"> iPhone </td>	<td class="table_font">192.168.1.3</td>
        <td  class="table_font">	fe80::1	</td>	<td class="table_font">	 2401:db8::1	</td>	<td class="table_font">	4A:6C:BF:C1:82:0F	</td>
        <td class="table_font"><input type="checkbox"   onClick="doModifyRule(this,'add',-1,1,'192.168.1.3','4A:6C:BF:C1:82:0F','0','0')" style="cursor: pointer;>
        <font style="cursor:hand;cursor:pointer;"> </font> 	</td>	</tr>
        <tr align="center">
        <td  class="table_font"><img src="/html/images/network/i_s_pc_off.gif" width="16" height="16" /></td>
        <td  class="table_font"> Unknown_64:16:66:44:b0:7e </td>	<td class="table_font">192.168.1.2</td>
        <td  class="table_font">	N/A	</td>	<td class="table_font">	N/A	</td>	<td class="table_font">	64:16:66:44:b0:7e	</td>
        <td class="table_font"></td></tr>
    """.trimIndent()

    private val ruleList = """
        <table id="filter_table">
        <tr><td width="5%" align=center class="top_font">#</td>
        <td width="10%" align=center class="top_font">Active</td>
        <td width="15%" align=center class="top_font">Interface</td>
        <td width="15%" align=center class="top_font">Direction</td>
        <td width="55%" align=center class="top_font">MAC Address</td>
        <tr class="table_font1"><td align=center>1</td>
        <td align="center">No</td>
        <td align="center">LAN</td>
        <td align="center">All</td>
        <td align=center>08:BF:B8:AD:DB:20</td>
        <tr class="table_font1"><td align=center>2</td>
        <td align="center">Yes</td>
        <td align="center">Internet</td>
        <td align="center">Outgoing</td>
        <td align=center>02:00:00:00:00:01</td>
        </table>
    """.trimIndent()

    @Test
    fun parsesConnectedDevices() {
        val devices = RouterParsers.devices(connectionTable)
        assertEquals(
            listOf(
                Device("iPhone", "192.168.1.3", "4A:6C:BF:C1:82:0F", online = true),
                Device("Unknown_64:16:66:44:b0:7e", "192.168.1.2", "64:16:66:44:B0:7E", online = false),
            ),
            devices,
        )
    }

    @Test
    fun parsesFilterRules() {
        assertEquals(
            listOf(
                FilterRule(1, active = false, iface = "LAN", direction = "All", mac = "08:BF:B8:AD:DB:20"),
                FilterRule(2, active = true, iface = "Internet", direction = "Outgoing", mac = "02:00:00:00:00:01"),
            ),
            RouterParsers.filterRules(ruleList),
        )
    }

    @Test
    fun readsFilterMode() {
        val page = """
            <select id='Rule_Type_selection' name="RuleTypeSEL" size="1" >
              <option value="0"  id="Disable_Text">Disable</option>
              <option value="1" selected id="Deny" >Black List</option>
              <option value="2"  id="Allow">White List</option>
            </select>
            <select id='DirectionSELsel' name="DirectionSEL"><option value="All" selected>All</option></select>
        """.trimIndent()
        assertEquals(FilterMode.BLACKLIST, RouterParsers.filterMode(page))
        assertNull(RouterParsers.filterMode("<html></html>"))
    }

    @Test
    fun loginPageAndResponses() {
        assertEquals("2dbce319", RouterParsers.loginSid("var sid = '2dbce319';"))

        val ok = """<script language="javascript">
            window.parent.location.href="indexmain.cgi";
            </script>"""
        assertTrue(RouterParsers.isLoginSuccess(ok))

        // A failed login re-serves the login page, which itself mentions indexmain.cgi.
        val failed = """<form name="passWarning" method="post">
            window.setTimeout(function(){window.location.href='/cgi-bin/indexmain.cgi';}, 1000);
            var locked_time = "";"""
        assertFalse(RouterParsers.isLoginSuccess(failed))
        assertFalse(RouterParsers.isLoginLocked(failed))
        assertTrue(RouterParsers.isLoginLocked("""var locked_time = "1760000000";"""))
    }

    @Test
    fun detectsExpiredSession() {
        val redirect = """<head><script language="javascript">
            window.parent.location.href="/cgi-bin/login_advance.cgi";
            </script></head>"""
        assertTrue(RouterParsers.isLoginRedirect(redirect))
        assertFalse(RouterParsers.isLoginRedirect(connectionTable))
    }

    @Test
    fun hashesPasswordLikeTheRouterPage() {
        // encodeURIComponent("p@ss w/rd!") with lowercase hex, as the login page does.
        assertEquals("p%40ss%20w%2frd!", RouterParsers.encodeUriComponent("p@ss w/rd!"))
        // Same as the page: hex_md5(encodeURIComponent("abc@") + ":" + sid)
        assertEquals("e83cd2d5d840b780aea9d829be2cf0a6", RouterParsers.passwordHash("abc@", "1234abcd"))
    }

    @Test
    fun privateMacDetection() {
        assertTrue(isPrivateMac("4A:6C:BF:C1:82:0F"))
        assertTrue(isPrivateMac("BA:FB:93:35:82:0D"))
        assertFalse(isPrivateMac("08:BF:B8:AD:DB:20"))
        assertFalse(isPrivateMac("64:16:66:44:B0:7E"))
    }

    @Test
    fun readsRouterNameFromStatusPage() {
        val status = """
            </td></tr><tr ><td  class="table_font"><span id='ModelName_ConnectionStatus'></span>: </td><td  class="table_font w_blue">
            AOT5221ZY
            </td></tr>
            </td></tr ><tr ><td  class="table_font">&nbsp;&nbsp;-  SSID: </td><td  class="table_font w_blue">
            Airtel_vanl_4805
            </td></tr>
        """.trimIndent()
        assertEquals(RouterInfo("AOT5221ZY", "Airtel_vanl_4805"), RouterParsers.routerInfo(status))
        assertEquals(RouterInfo(null, null), RouterParsers.routerInfo("<html></html>"))
    }
}

package com.phaw.netcontrol.ui

import androidx.annotation.DrawableRes
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ExperimentalLayoutApi
import androidx.compose.foundation.layout.FlowRow
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.asPaddingValues
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.systemBars
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.automirrored.filled.KeyboardArrowRight
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.filled.Info
import androidx.compose.material.icons.filled.KeyboardArrowDown
import androidx.compose.material.icons.filled.MoreVert
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material.icons.outlined.Settings
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Checkbox
import androidx.compose.material3.CheckboxDefaults
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Surface
import androidx.compose.material3.Switch
import androidx.compose.material3.SwitchDefaults
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TimeInput
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.material3.rememberTimePickerState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalClipboardManager
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.AnnotatedString
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.phaw.netcontrol.R
import com.phaw.netcontrol.app
import com.phaw.netcontrol.bedtime.BedtimeWindow
import com.phaw.netcontrol.bedtime.formatMinute
import kotlinx.coroutines.launch
import java.time.format.DateTimeFormatter

@Composable
fun NetControlScreen(viewModel: MainViewModel) {
    val state by viewModel.state.collectAsStateWithLifecycle()
    var showSettings by remember { mutableStateOf(false) }

    if (state.needsSetup || state.authError || showSettings) {
        SettingsScreen(
            viewModel = viewModel,
            message = if (state.authError) state.error else null,
            canGoBack = !state.needsSetup && !state.authError,
            onDone = { showSettings = false },
        )
    } else {
        DevicesScreen(state, viewModel, onSettings = { showSettings = true })
    }
}

private enum class Help { PRIVATE_MAC, BEDTIME }

@Composable
private fun DevicesScreen(state: UiState, viewModel: MainViewModel, onSettings: () -> Unit) {
    var renaming by remember { mutableStateOf<DeviceRow?>(null) }
    var editingBedtime by remember { mutableStateOf(false) }
    var help by remember { mutableStateOf<Help?>(null) }
    val insets = WindowInsets.systemBars.asPaddingValues()

    // Surface supplies the default text color; without it, uncolored text renders black on the dark background.
    Surface(color = Palette.Background, contentColor = Palette.TextPrimary, modifier = Modifier.fillMaxSize()) {
    LazyColumn(
        modifier = Modifier.fillMaxSize(),
        contentPadding = PaddingValues(
            start = 16.dp, end = 16.dp,
            top = insets.calculateTopPadding() + 16.dp,
            bottom = insets.calculateBottomPadding() + 24.dp,
        ),
        verticalArrangement = Arrangement.spacedBy(14.dp),
    ) {
        item { Header(state, onRefresh = viewModel::refreshNow, onSettings = onSettings) }
        state.error?.let { error ->
            item {
                Surface(color = MaterialTheme.colorScheme.errorContainer, shape = RoundedCornerShape(16.dp)) {
                    Text(error, Modifier.padding(16.dp), color = MaterialTheme.colorScheme.onErrorContainer)
                }
            }
        }
        item {
            BedtimeCard(
                bedtime = state.bedtime,
                deviceCount = state.rows.count { it.onBedtime },
                busy = MainViewModel.BEDTIME in state.busy,
                onEdit = { editingBedtime = true },
            )
        }
        if (!state.loaded) {
            item {
                Box(Modifier.fillMaxWidth().padding(32.dp), contentAlignment = Alignment.Center) {
                    CircularProgressIndicator()
                }
            }
        }
        val (online, offline) = state.rows.partition { it.online }
        if (state.loaded) {
            item { SectionHeader("Connected now (${online.size})", state.sort, viewModel::setSort) }
        }
        items(online, key = { it.mac }) { row ->
            DeviceCard(
                row, busy = row.mac in state.busy, viewModel,
                onRename = { renaming = row }, onHelp = { help = it },
            )
        }
        if (offline.isNotEmpty()) {
            item { SectionHeader("Not connected (${offline.size})", sort = null, onSort = {}) }
        }
        items(offline, key = { it.mac }) { row ->
            DeviceCard(
                row, busy = row.mac in state.busy, viewModel,
                onRename = { renaming = row }, onHelp = { help = it },
            )
        }
    }
    }

    renaming?.let { row ->
        RenameDialog(row, onDismiss = { renaming = null }) { name ->
            viewModel.rename(row.mac, name)
            renaming = null
        }
    }
    if (editingBedtime) {
        BedtimeDialog(state.bedtime, onDismiss = { editingBedtime = false }) { enabled, window ->
            viewModel.saveBedtime(enabled, window)
            editingBedtime = false
        }
    }
    help?.let { HelpDialog(it, onDismiss = { help = null }) }
}

@Composable
private fun Header(state: UiState, onRefresh: () -> Unit, onSettings: () -> Unit) {
    Row(Modifier.fillMaxWidth().padding(start = 4.dp, bottom = 6.dp), verticalAlignment = Alignment.CenterVertically) {
        Column(Modifier.weight(1f)) {
            Text(
                state.title,
                fontSize = 32.sp,
                fontWeight = FontWeight.Bold,
                maxLines = 1,
                overflow = TextOverflow.Ellipsis,
            )
            Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.padding(top = 4.dp)) {
                val live = state.loaded && state.error == null
                Box(Modifier.size(10.dp).background(if (live) Palette.Live else Palette.Offline, CircleShape))
                Spacer(Modifier.width(8.dp))
                val time = state.updatedAt?.format(DateTimeFormatter.ofPattern("HH:mm:ss"))
                Text(
                    when {
                        !state.loaded -> "Connecting…"
                        live && time != null -> "Live · updated $time"
                        time != null -> "Offline · last updated $time"
                        else -> "Offline"
                    },
                    color = Palette.TextSecondary,
                    fontSize = 16.sp,
                )
            }
        }
        RoundIconButton(Icons.Default.Refresh, "Refresh", onRefresh)
        Spacer(Modifier.width(12.dp))
        RoundIconButton(Icons.Outlined.Settings, "Settings", onSettings)
    }
}

@Composable
private fun RoundIconButton(icon: ImageVector, description: String, onClick: () -> Unit) {
    Box(
        Modifier.size(52.dp).background(Palette.RoundButton, CircleShape).clickable(onClick = onClick),
        contentAlignment = Alignment.Center,
    ) {
        Icon(icon, contentDescription = description, tint = Palette.TextPrimary)
    }
}

@Composable
private fun BedtimeCard(bedtime: BedtimeUi, deviceCount: Int, busy: Boolean, onEdit: () -> Unit) {
    Card(
        Modifier.fillMaxWidth().clickable(onClick = onEdit),
        shape = RoundedCornerShape(22.dp),
        colors = CardDefaults.cardColors(containerColor = Palette.Card, contentColor = Palette.TextPrimary),
        border = BorderStroke(1.dp, Palette.CardBorder),
    ) {
        Row(Modifier.padding(16.dp), verticalAlignment = Alignment.CenterVertically) {
            Box(Modifier.size(56.dp).background(Palette.TilePurple, CircleShape), contentAlignment = Alignment.Center) {
                Icon(painterResource(R.drawable.ic_moon), contentDescription = null, tint = Palette.IconPurple,
                    modifier = Modifier.size(28.dp))
            }
            Spacer(Modifier.width(16.dp))
            Column(Modifier.weight(1f)) {
                Text("Bedtime", fontSize = 20.sp, fontWeight = FontWeight.SemiBold)
                val window = "${formatMinute(bedtime.window.startMinute)} – ${formatMinute(bedtime.window.endMinute)}"
                Text(
                    when {
                        !bedtime.enabled -> "Off · tap Edit to turn on"
                        bedtime.activeNow && deviceCount > 0 -> "$window · Active now · $deviceCount paused"
                        else -> "$window · Pause internet on selected devices"
                    },
                    color = Palette.TextSecondary,
                    fontSize = 14.sp,
                )
            }
            Spacer(Modifier.width(8.dp))
            if (busy) {
                CircularProgressIndicator(Modifier.size(28.dp))
            } else {
                Button(
                    onClick = onEdit,
                    colors = ButtonDefaults.buttonColors(containerColor = Palette.Purple, contentColor = Palette.OnPurple),
                ) { Text("Edit", fontWeight = FontWeight.SemiBold) }
            }
        }
    }
}

@Composable
private fun SectionHeader(text: String, sort: SortOrder?, onSort: (SortOrder) -> Unit) {
    var open by remember { mutableStateOf(false) }
    Row(
        Modifier.fillMaxWidth().padding(start = 4.dp, top = 10.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        Text(text, fontSize = 22.sp, fontWeight = FontWeight.SemiBold, modifier = Modifier.weight(1f))
        if (sort != null) {
            Box {
                TextButton(onClick = { open = true }) {
                    Text(if (sort == SortOrder.NAME) "Sort by" else "Sort: ${sort.label}", color = Palette.Purple)
                    Icon(Icons.Default.KeyboardArrowDown, contentDescription = null, tint = Palette.Purple)
                }
                DropdownMenu(expanded = open, onDismissRequest = { open = false }) {
                    SortOrder.entries.forEach { order ->
                        DropdownMenuItem(
                            text = { Text(order.label, fontWeight = if (order == sort) FontWeight.Bold else null) },
                            onClick = { onSort(order); open = false },
                        )
                    }
                }
            }
        }
    }
}

@OptIn(ExperimentalLayoutApi::class)
@Composable
private fun DeviceCard(
    row: DeviceRow,
    busy: Boolean,
    viewModel: MainViewModel,
    onRename: () -> Unit,
    onHelp: (Help) -> Unit,
) {
    var menu by remember { mutableStateOf(false) }
    val clipboard = LocalClipboardManager.current
    Card(
        Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(22.dp),
        colors = CardDefaults.cardColors(
            containerColor = if (row.blocked) Palette.CardPaused else Palette.Card,
            contentColor = Palette.TextPrimary,
        ),
        border = BorderStroke(1.dp, if (row.blocked) Palette.Paused.copy(alpha = 0.35f) else Palette.CardBorder),
    ) {
        Column(Modifier.padding(start = 16.dp, top = 16.dp, end = 4.dp, bottom = 8.dp)) {
            Row(verticalAlignment = Alignment.Top) {
                DeviceTile(row.kind, Modifier.alpha(if (row.online) 1f else 0.5f))
                Spacer(Modifier.width(16.dp))
                Column(Modifier.weight(1f)) {
                    FlowRow(
                        horizontalArrangement = Arrangement.spacedBy(8.dp),
                        verticalArrangement = Arrangement.spacedBy(4.dp),
                    ) {
                        Text(
                            row.title,
                            fontSize = 19.sp,
                            fontWeight = FontWeight.SemiBold,
                            modifier = Modifier.align(Alignment.CenterVertically),
                        )
                        when {
                            row.thisPhone -> Pill("This device")
                            !row.named -> Pill("Edit name", Icons.Default.Edit, onClick = onRename)
                        }
                    }
                    Spacer(Modifier.height(4.dp))
                    if (row.ip.isNotBlank()) Text(row.ip, color = Palette.TextSecondary, fontSize = 15.sp)
                    Text(row.mac, color = Palette.TextSecondary, fontSize = 15.sp)
                }
                Column(horizontalAlignment = Alignment.CenterHorizontally, modifier = Modifier.width(96.dp)) {
                    if (busy) {
                        CircularProgressIndicator(Modifier.padding(top = 6.dp).size(28.dp))
                    } else {
                        Switch(
                            checked = !row.blocked,
                            enabled = !row.thisPhone,
                            onCheckedChange = { allowed -> viewModel.setBlocked(row.mac, blocked = !allowed) },
                            colors = SwitchDefaults.colors(
                                checkedTrackColor = Palette.PurpleStrong,
                                checkedThumbColor = Color.White,
                                checkedBorderColor = Palette.PurpleStrong,
                                disabledCheckedTrackColor = Palette.PurpleStrong.copy(alpha = 0.45f),
                                disabledCheckedThumbColor = Color.White.copy(alpha = 0.8f),
                            ),
                        )
                    }
                    Text(
                        if (row.blocked) "Paused" else "Internet on",
                        color = if (row.blocked) Palette.Paused else Palette.TextSecondary,
                        fontSize = 14.sp,
                    )
                }
                Box {
                    IconButton(onClick = { menu = true }) {
                        Icon(Icons.Default.MoreVert, contentDescription = "More", tint = Palette.TextSecondary)
                    }
                    DropdownMenu(expanded = menu, onDismissRequest = { menu = false }) {
                        DropdownMenuItem(text = { Text("Rename") }, onClick = { menu = false; onRename() })
                        DropdownMenuItem(
                            text = { Text("Copy MAC address") },
                            onClick = { menu = false; clipboard.setText(AnnotatedString(row.mac)) },
                        )
                    }
                }
            }
            if (row.privateMac) {
                Spacer(Modifier.height(12.dp))
                Surface(
                    onClick = { onHelp(Help.PRIVATE_MAC) },
                    color = Palette.AmberBg,
                    shape = RoundedCornerShape(14.dp),
                    modifier = Modifier.fillMaxWidth().padding(end = 12.dp),
                ) {
                    Row(Modifier.padding(horizontal = 12.dp, vertical = 12.dp), verticalAlignment = Alignment.CenterVertically) {
                        Icon(Icons.Default.Info, contentDescription = null, tint = Palette.Amber)
                        Spacer(Modifier.width(10.dp))
                        Text(
                            "Private Wi-Fi address: it may change and slip past blocking.",
                            fontSize = 14.sp,
                            color = Palette.TextPrimary,
                            modifier = Modifier.weight(1f),
                        )
                        Icon(Icons.AutoMirrored.Filled.KeyboardArrowRight, contentDescription = null, tint = Palette.TextSecondary)
                    }
                }
            }
            if (!row.thisPhone) {
                Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.padding(top = 4.dp)) {
                    Checkbox(
                        checked = row.onBedtime,
                        enabled = !busy,
                        onCheckedChange = { viewModel.setOnBedtime(row.mac, it) },
                        colors = CheckboxDefaults.colors(checkedColor = Palette.PurpleStrong, uncheckedColor = Palette.TextSecondary),
                    )
                    Text("Pause internet at bedtime", fontSize = 16.sp)
                    IconButton(onClick = { onHelp(Help.BEDTIME) }) {
                        Icon(painterResource(R.drawable.ic_help), contentDescription = "What is this?",
                            tint = Palette.TextSecondary, modifier = Modifier.size(22.dp))
                    }
                }
            }
        }
    }
}

@Composable
private fun DeviceTile(kind: DeviceKind, modifier: Modifier = Modifier) {
    val (@DrawableRes icon, tile, tint) = when (kind) {
        DeviceKind.PHONE -> Triple(R.drawable.ic_phone, Palette.TilePurple, Palette.IconPurple)
        DeviceKind.TABLET -> Triple(R.drawable.ic_tablet, Palette.TilePurple, Palette.IconPurple)
        DeviceKind.LAPTOP -> Triple(R.drawable.ic_laptop, Palette.TileBlue, Palette.IconBlue)
        DeviceKind.UNKNOWN -> Triple(R.drawable.ic_devices, Palette.TileGrey, Palette.IconGrey)
    }
    Box(
        modifier.size(64.dp).background(tile, RoundedCornerShape(16.dp)),
        contentAlignment = Alignment.Center,
    ) {
        Icon(painterResource(icon), contentDescription = null, tint = tint, modifier = Modifier.size(32.dp))
    }
}

@Composable
private fun Pill(text: String, icon: ImageVector? = null, onClick: (() -> Unit)? = null) {
    val shape = RoundedCornerShape(50)
    Row(
        Modifier
            .border(1.dp, Palette.Purple.copy(alpha = 0.7f), shape)
            .then(if (onClick != null) Modifier.clickable(onClick = onClick) else Modifier)
            .padding(horizontal = 12.dp, vertical = 4.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        if (icon != null) {
            Icon(icon, contentDescription = null, tint = Palette.Purple, modifier = Modifier.size(16.dp))
            Spacer(Modifier.width(6.dp))
        }
        Text(text, color = Palette.Purple, fontSize = 14.sp)
    }
}

@Composable
private fun HelpDialog(help: Help, onDismiss: () -> Unit) {
    val (title, body) = when (help) {
        Help.PRIVATE_MAC -> "Private Wi-Fi address" to
            "This device uses a randomized MAC address. If it changes, the router sees a new device and " +
            "the block no longer applies.\n\n" +
            "iPhone / iPad: Settings → Wi-Fi → ⓘ next to the network → turn off Private Wi-Fi Address " +
            "(or set it to Fixed).\n\n" +
            "Android: Settings → Wi-Fi → the network → Privacy → Use device MAC.\n\n" +
            "On a child's device, protect the setting with a Screen Time / Family Link passcode."
        Help.BEDTIME -> "Pause internet at bedtime" to
            "During the bedtime hours this device's internet is paused at the router, and it comes back " +
            "on when bedtime ends.\n\n" +
            "If you switch it back on during bedtime, it stays on until the morning.\n\n" +
            "The schedule runs from this phone, so it needs to be on the home Wi-Fi at bedtime."
    }
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(title) },
        text = { Text(body) },
        confirmButton = { TextButton(onClick = onDismiss) { Text("OK") } },
    )
}

@Composable
private fun RenameDialog(row: DeviceRow, onDismiss: () -> Unit, onSave: (String) -> Unit) {
    var name by remember { mutableStateOf(if (row.named) row.title else "") }
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("Name this device") },
        text = {
            Column {
                Text(row.mac, color = Palette.TextSecondary, fontSize = 14.sp)
                Spacer(Modifier.size(8.dp))
                OutlinedTextField(name, { name = it }, label = { Text("e.g. Kid's iPad") }, singleLine = true)
            }
        },
        confirmButton = { TextButton(onClick = { onSave(name) }) { Text("Save") } },
        dismissButton = { TextButton(onClick = onDismiss) { Text("Cancel") } },
    )
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun BedtimeDialog(current: BedtimeUi, onDismiss: () -> Unit, onSave: (Boolean, BedtimeWindow) -> Unit) {
    var enabled by remember { mutableStateOf(current.enabled) }
    val start = rememberTimePickerState(current.window.startMinute / 60, current.window.startMinute % 60, is24Hour = true)
    val end = rememberTimePickerState(current.window.endMinute / 60, current.window.endMinute % 60, is24Hour = true)
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("Bedtime") },
        text = {
            Column(Modifier.verticalScroll(rememberScrollState())) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text("Pause internet on schedule", Modifier.weight(1f))
                    Switch(checked = enabled, onCheckedChange = { enabled = it })
                }
                Spacer(Modifier.size(12.dp))
                Text("Internet off from", style = MaterialTheme.typography.labelLarge)
                TimeInput(start)
                Text("until", style = MaterialTheme.typography.labelLarge)
                TimeInput(end)
                Text(
                    "Runs from this phone: it needs to be on the home Wi-Fi at these times.",
                    style = MaterialTheme.typography.bodySmall,
                    color = Palette.TextSecondary,
                )
            }
        },
        confirmButton = {
            TextButton(onClick = {
                onSave(enabled, BedtimeWindow(start.hour * 60 + start.minute, end.hour * 60 + end.minute))
            }) { Text("Save") }
        },
        dismissButton = { TextButton(onClick = onDismiss) { Text("Cancel") } },
    )
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun SettingsScreen(viewModel: MainViewModel, message: String?, canGoBack: Boolean, onDone: () -> Unit) {
    val prefs = LocalContext.current.app.prefs
    var host by remember { mutableStateOf(prefs.host) }
    var username by remember { mutableStateOf(prefs.username) }
    var password by remember { mutableStateOf(prefs.password) }
    var error by remember { mutableStateOf(message) }
    var testing by remember { mutableStateOf(false) }
    val scope = rememberCoroutineScope()

    Scaffold(
        containerColor = Palette.Background,
        topBar = {
            TopAppBar(
                title = { Text("Router login") },
                colors = TopAppBarDefaults.topAppBarColors(containerColor = Palette.Background),
                navigationIcon = {
                    if (canGoBack) IconButton(onClick = onDone) {
                        Icon(Icons.AutoMirrored.Filled.ArrowBack, contentDescription = "Back")
                    }
                },
            )
        },
    ) { padding ->
        Column(
            Modifier.fillMaxSize().padding(padding).padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp),
        ) {
            Text(
                "The admin login for your router (the same one you use at https://192.168.1.1).",
                color = Palette.TextSecondary,
            )
            OutlinedTextField(host, { host = it }, label = { Text("Router address") }, singleLine = true,
                keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Uri), modifier = Modifier.fillMaxWidth())
            OutlinedTextField(username, { username = it }, label = { Text("Username") }, singleLine = true,
                modifier = Modifier.fillMaxWidth())
            OutlinedTextField(password, { password = it }, label = { Text("Password") }, singleLine = true,
                visualTransformation = PasswordVisualTransformation(),
                keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Password), modifier = Modifier.fillMaxWidth())
            error?.let { Text(it, color = MaterialTheme.colorScheme.error) }
            Row(verticalAlignment = Alignment.CenterVertically) {
                Button(
                    enabled = !testing && password.isNotEmpty(),
                    colors = ButtonDefaults.buttonColors(containerColor = Palette.Purple, contentColor = Palette.OnPurple),
                    onClick = {
                        testing = true
                        scope.launch {
                            error = viewModel.saveSettings(host, username, password)
                            testing = false
                            if (error == null) onDone()
                        }
                    },
                ) { Text("Log in & save") }
                if (canGoBack) {
                    Spacer(Modifier.width(8.dp))
                    OutlinedButton(onClick = onDone) { Text("Cancel") }
                }
                if (testing) {
                    Spacer(Modifier.width(12.dp))
                    CircularProgressIndicator(Modifier.size(24.dp))
                }
            }
            Text(
                "The password is stored encrypted on this phone and only sent to the router.",
                style = MaterialTheme.typography.bodySmall,
                color = Palette.TextSecondary,
            )
        }
    }
}

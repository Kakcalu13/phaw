package com.phaw.netcontrol.ui

import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Typography
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color

/** Dark, purple-accented palette from the design mock. */
object Palette {
    val Background = Color(0xFF0D0D14)
    val Card = Color(0xFF17171F)
    val CardPaused = Color(0xFF131318)
    val CardBorder = Color(0xFF24242E)
    val RoundButton = Color(0xFF1E1E28)
    val Purple = Color(0xFFC6A8FF)
    val PurpleStrong = Color(0xFFA98BFF)
    val OnPurple = Color(0xFF1E1235)
    val TilePurple = Color(0xFF26223A)
    val TileBlue = Color(0xFF1F2836)
    val TileGrey = Color(0xFF24242B)
    val IconPurple = Color(0xFFB49BFF)
    val IconBlue = Color(0xFF8DB7FF)
    val IconGrey = Color(0xFF9A9AA6)
    val TextPrimary = Color(0xFFF1F0F5)
    val TextSecondary = Color(0xFFB4B3BE)
    val Live = Color(0xFF4CD07D)
    val Offline = Color(0xFFFF6B6B)
    val AmberBg = Color(0xFF2B2720)
    val Amber = Color(0xFFF2C14E)
    val Paused = Color(0xFFFF8A80)
}

private val colors = darkColorScheme(
    primary = Palette.Purple,
    onPrimary = Palette.OnPurple,
    secondary = Palette.PurpleStrong,
    background = Palette.Background,
    onBackground = Palette.TextPrimary,
    surface = Palette.Background,
    onSurface = Palette.TextPrimary,
    surfaceVariant = Palette.Card,
    onSurfaceVariant = Palette.TextSecondary,
    surfaceContainer = Palette.Card,
    surfaceContainerHigh = Palette.RoundButton,
    surfaceContainerHighest = Palette.RoundButton,
    outline = Palette.TextSecondary,
    error = Palette.Paused,
    errorContainer = Color(0xFF3A1D22),
    onErrorContainer = Color(0xFFFFD9DC),
)

@Composable
fun AppTheme(content: @Composable () -> Unit) {
    MaterialTheme(colorScheme = colors, typography = Typography(), content = content)
}

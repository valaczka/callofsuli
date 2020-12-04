pragma Singleton
import QtQuick 2.0
import QtQuick.Controls.Material 2.3

QtObject {
	property int materialPrimary: Material.Cyan
	property color colorPrimary: Material.color(materialPrimary, Material.Shade500)
	property color colorPrimaryLighter: Material.color(materialPrimary, Material.Shade200)
	property color colorPrimaryLight: Material.color(materialPrimary, Material.Shade50)
	property color colorPrimaryDarker: Material.color(materialPrimary, Material.Shade700)
	property color colorPrimaryDark: Material.color(materialPrimary, Material.Shade900)

	property int materialAccent: Material.Amber
	property color colorAccent: Material.color(materialAccent, Material.Shade500)
	property color colorAccentLighter: Material.color(materialAccent, Material.Shade200)
	property color colorAccentLight: Material.color(materialAccent, Material.Shade50)
	property color colorAccentDarker: Material.color(materialAccent, Material.Shade700)
	property color colorAccentDark: Material.color(materialAccent, Material.Shade900)

	property color colorError: Material.color(Material.DeepOrange, Material.ShadeA700)
	property color colorErrorLighter: Material.color(Material.DeepOrange, Material.ShadeA400)
	property color colorErrorLight: Material.color(Material.DeepOrange, Material.ShadeA100)
	property color colorErrorDarker: Material.color(Material.Red, Material.ShadeA700)
	property color colorErrorDark: Material.color(Material.Red, Material.Shade900)


	property color colorWarning: Material.color(Material.Orange, Material.ShadeA400)
	property color colorWarningLighter: Material.color(Material.Orange, Material.ShadeA200)
	property color colorWarningLight: Material.color(Material.Orange, Material.ShadeA100)
	property color colorWarningDarker: Material.color(Material.Orange, Material.Shade700)
	property color colorWarningDark: Material.color(Material.Orange, Material.Shade900)


	property color colorOK: Material.color(Material.LightGreen, Material.Shade500)
	property color colorOKLighter: Material.color(Material.LightGreen, Material.ShadeA700)
	property color colorOKLight: Material.color(Material.LightGreen, Material.ShadeA100)
	property color colorOKDarker: Material.color(Material.Green, Material.Shade700)
	property color colorOKDark: Material.color(Material.Green, Material.Shade900)

	property color colorBg: "#33ffffff"

	property int pixelSize: (Qt.platform.os === "android" ? 16 : 18)
	property int baseHeight: Math.max(pixelSize * 2.2, (Qt.platform.os === "android" ? 48 : 24))
	property int twoLineHeight: Math.max(pixelSize * 48/18, 32)
	property int panelPaddingLeft: pixelSize * 0.5
	property int panelPaddingRight: pixelSize * 0.5


	property var buttonThemeDefault: [colorPrimaryLight, colorPrimaryDarker, colorPrimaryLighter, colorPrimary]
	property var buttonThemeDelete: [colorPrimaryLight, colorErrorDarker, colorErrorDark, colorError]
	property var buttonThemeApply: [colorPrimaryLight, colorOKDarker, colorOKDark, colorOKLighter]

	property color colorPlayer: colorPrimaryLighter
	property color colorGlowEnemy: colorError
	property color colorGlowItem: colorAccentLighter
	property color colorEnemyMarker: colorWarningLighter
	property color colorEnemyMarkerAttack: colorError

	property string iconMenu: "image://font/Material Icons/\ue5d4"
	property string iconClose: "image://font/Material Icons/\ue14c"
	property string iconChecked: "image://font/Material Icons/\ue86c"
	property string iconUnchecked: "image://font/Material Icons/\ue836"
	property string iconTrash: "image://font/Material Icons/\ue872"
	property string iconRemove: iconDelete
	property string iconClear: "image://font/Material Icons/\ue14a"
	property string iconBack: "image://font/Material Icons/\ue5c4"
	property string iconUndo: "image://font/Material Icons/\ue166"
	property string iconSelectAll: "image://font/Material Icons/\ue877"
	property string iconDrawer: "image://font/Material Icons/\ue3c7"
	property string iconDelete: iconTrash
	property string iconCancel: "image://font/Material Icons/\ue5cd"
	property string iconAdd: "image://font/Material Icons/\ue145"
	property string iconSave: "image://font/Material Icons/\ue161"
	property string iconSend: "image://font/Material Icons/\ue163"
	property string iconEdit: "image://font/Material Icons/\ue254"
	property string iconOK: "image://font/Material Icons/\ue5ca"
	property string iconDown: "image://font/Material Icons/\ue5c5"
	property string iconDialogQuestion: "image://font/Material Icons/\ue887"
	property string iconDialogInfo: "image://font/Material Icons/\ue88f"
	property string iconDialogWarning: "image://font/Material Icons/\ue002"
	property string iconDialogError: "image://font/Material Icons/\ue000"

	property string iconKey: "image://font/Material Icons/\ue0da"
	property string iconClock1: "image://font/Material Icons/\ue190"
	property string iconClock2: "image://font/Material Icons/\ue192"
	property string iconClock3: "image://font/Material Icons/\ue8ae"
	property string iconClockAdd: "image://font/Material Icons/\ue193"
	property string iconStopWatch: "image://font/Material Icons/\ue425"
	property string iconStopWatchDisabled: "image://font/Material Icons/\ue426"
	property string iconAdjust: "image://font/Material Icons/\ue429"
	property string iconSetup: "image://font/Material Icons/\ue869"
	property string iconPreferences: "image://font/Material Icons/\ue8b8"
	property string iconSearch: "image://font/Material Icons/\ue8b6"

	property string iconLockAdd: "image://font/Material Icons/\ue63f"
	property string iconLockDisabled: "image://font/Material Icons/\ue641"
	property string iconLock: "image://font/Material Icons/\ue897"
	property string iconLockOpened: "image://font/Material Icons/\ue898"
	property string iconLockClosed: "image://font/Material Icons/\ue899"
	property string iconUser: "image://font/Material Icons/\ue7fd"
	property string iconUserWhite: "image://font/Material Icons/\ue7ff"
	property string iconUserAdd: "image://font/Material Icons/\ue7fe"
	property string iconUsers: "image://font/Material Icons/\ue7ef"
	property string iconUsersAdd: "image://font/Material Icons/\ue7f0"
	property string iconFavoriteOn: "image://font/Material Icons/\ue838"
	property string iconFavoriteOff: "image://font/Material Icons/\ue83a"
	property string iconVisible: "image://font/Material Icons/\ue8f4"
	property string iconInvisible: "image://font/Material Icons/\ue8f5"
}

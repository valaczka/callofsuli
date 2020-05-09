pragma Singleton
import QtQuick 2.0
import QtQuick.Controls.Material 2.3

QtObject {
	readonly property int materialPrimary: Material.Cyan
	readonly property color colorPrimary: Material.color(materialPrimary, Material.Shade500)
	readonly property color colorPrimaryLighter: Material.color(materialPrimary, Material.Shade200)
	readonly property color colorPrimaryLight: Material.color(materialPrimary, Material.Shade50)
	readonly property color colorPrimaryDarker: Material.color(materialPrimary, Material.Shade700)
	readonly property color colorPrimaryDark: Material.color(materialPrimary, Material.Shade900)

	readonly property int materialAccent: Material.Amber
	readonly property color colorAccent: Material.color(materialAccent, Material.Shade500)
	readonly property color colorAccentLighter: Material.color(materialAccent, Material.Shade200)
	readonly property color colorAccentLight: Material.color(materialAccent, Material.Shade50)
	readonly property color colorAccentDarker: Material.color(materialAccent, Material.Shade700)
	readonly property color colorAccentDark: Material.color(materialAccent, Material.Shade900)

	readonly property color colorError: Material.color(Material.DeepOrange, Material.ShadeA700)
	readonly property color colorErrorLighter: Material.color(Material.DeepOrange, Material.ShadeA400)
	readonly property color colorErrorLight: Material.color(Material.DeepOrange, Material.ShadeA100)
	readonly property color colorErrorDarker: Material.color(Material.Red, Material.ShadeA700)
	readonly property color colorErrorDark: Material.color(Material.Red, Material.Shade900)


	readonly property color colorWarning: Material.color(Material.Orange, Material.ShadeA400)
	readonly property color colorWarningLighter: Material.color(Material.Orange, Material.ShadeA200)
	readonly property color colorWarningLight: Material.color(Material.Orange, Material.ShadeA100)
	readonly property color colorWarningDarker: Material.color(Material.Orange, Material.Shade700)
	readonly property color colorWarningDark: Material.color(Material.Orange, Material.Shade900)


	readonly property color colorOK: Material.color(Material.Light, Material.Shade500)
	readonly property color colorOKLighter: Material.color(Material.LightGreen, Material.ShadeA700)
	readonly property color colorOKLigth: Material.color(Material.LightGreen, Material.ShadeA100)
	readonly property color colorOKDarker: Material.color(Material.Green, Material.Shade700)
	readonly property color colorOKDark: Material.color(Material.Green, Material.Shade900)

	readonly property color colorBg: "#33ffffff"

	readonly property int pixelSize: 18
	readonly property int baseHeight: Qt.platform.os === "android" ? 48 : 32
	readonly property int twoLineHeight: 48


	readonly property var buttonThemeDefault: [colorPrimaryLight, colorPrimaryDarker, colorPrimaryLighter, colorPrimary]
	readonly property var buttonThemeDelete: [colorPrimaryLight, colorErrorDarker, colorErrorDark, colorError]
	readonly property var buttonThemeApply: [colorPrimaryLight, colorOKDarker, colorOKDark, colorOKLighter]

	readonly property string iconMenu: "image://font/Material Icons/\ue5d4"
	readonly property string iconClose: "image://font/Material Icons/\ue14c"
	readonly property string iconChecked: "image://font/Material Icons/\ue86c"
	readonly property string iconUnchecked: "image://font/Material Icons/\ue836"
	readonly property string iconTrash: "image://font/Material Icons/\ue872"
	readonly property string iconRemove: iconTrash
	readonly property string iconClear: "image://font/Material Icons/\ue14a"
	readonly property string iconBack: "image://font/Material Icons/\ue5c4"
	readonly property string iconUndo: "image://font/Material Icons/\ue166"
	readonly property string iconSelectAll: "image://font/Material Icons/\ue162"
	readonly property string iconDrawer: "image://font/Material Icons/\ue3c7"
	readonly property string iconDelete: "image://font/Material Icons/\ue5cd"
	readonly property string iconCancel: iconDelete
	readonly property string iconAdd: "image://font/Material Icons/\ue145"
	readonly property string iconSave: "image://font/Material Icons/\ue161"
	readonly property string iconOK: "image://font/Material Icons/\ue5ca"
	readonly property string iconDown: "image://font/Material Icons/\ue5c5"
	readonly property string iconDialogQuestion: "image://font/Material Icons/\ue887"
	readonly property string iconDialogInfo: "image://font/Material Icons/\ue88f"
	readonly property string iconDialogWarning: "image://font/Material Icons/\ue002"
	readonly property string iconDialogError: "image://font/Material Icons/\ue000"
}

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
	readonly property color colorErrorLigth: Material.color(Material.DeepOrange, Material.ShadeA100)
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

	readonly property int pixelSize: 18
	readonly property int baseHeight: Qt.platform.os === "android" ? 48 : 32
}

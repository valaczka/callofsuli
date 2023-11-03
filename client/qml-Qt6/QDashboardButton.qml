import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

QButton {
	id: control

	readonly property QDashboardGrid dashboard: parent instanceof QDashboardGrid ? parent : null

	implicitWidth: dashboard ? dashboard.buttonSize : Qaterial.Style.dashboardButtonSize
	implicitHeight: dashboard ? dashboard.buttonSize : Qaterial.Style.dashboardButtonSize

	display: AbstractButton.TextUnderIcon

	//highlighted: true

	wrapMode: Text.WordWrap
	maximumLineCount: 2

	spacing: Qaterial.Style.pixelSize*0.5

	icon.width: width*0.4
	icon.height: height*0.4

	ToolTip.text: text
	ToolTip.visible: hovered || pressed
	ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval

	font.pixelSize: Qaterial.Style.textTheme.button.pixelSize
	font.family: Qaterial.Style.textTheme.button.family
	font.weight: Qaterial.Style.textTheme.button.weight
	font.capitalization: Qaterial.Style.textTheme.button.capitalization
	font.letterSpacing: 0
}

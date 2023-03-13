import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
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

	readonly property real _psRatio: Qaterial.Style.pixelSizeRatio
	on_PsRatioChanged: {
		var f = Qaterial.Style.textTheme.button
		f.letterSpacing = 0
		font = f
	}

	Component.onCompleted: {
		var f = Qaterial.Style.textTheme.button
		f.letterSpacing = 0
		font = f
	}
}

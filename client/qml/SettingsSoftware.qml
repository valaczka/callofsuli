import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Column {
	id: root

	spacing: 10 * Qaterial.Style.pixelSizeRatio

	Qaterial.SwitchButton {
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("Frissítések automatikus keresése induláskor")
		checked: Client.updater.autoUpdate
		onToggled: Client.updater.autoUpdate = checked
	}

	QButton {
		icon.source: Qaterial.Icons.update
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("Frissítések keresése")
		wrapMode: implicitWidth > parent.width ? Text.Wrap : Text.NoWrap
		onClicked: {
			Client.updater.checkAvailableUpdates(true)
			enabled = false
		}
	}
}

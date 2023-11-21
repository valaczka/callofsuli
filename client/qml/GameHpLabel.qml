import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.0
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


GameLabel {
	id: infoHP

	color: Qaterial.Colors.red800
	text: qsTr("%1 HP")
	iconLabel.icon.source: Qaterial.Icons.heartPulse
}

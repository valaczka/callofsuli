import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.0
import QtMultimedia 5.12
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


GameLabel {
	id: infoHP

	anchors.topMargin: Math.max(Client.safeMarginTop, 5)
	color: Qaterial.Colors.red800
	text: qsTr("%1 HP")
	iconLabel.icon.source: Qaterial.Icons.heartPulse
}

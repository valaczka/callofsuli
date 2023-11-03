import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt5Compat.GraphicalEffects
import QtMultimedia
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


GameLabel {
	id: infoHP

	color: Qaterial.Colors.red800
	text: qsTr("%1 HP")
	iconLabel.icon.source: Qaterial.Icons.heartPulse
}

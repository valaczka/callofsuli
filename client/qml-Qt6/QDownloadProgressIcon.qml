import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

Row {
	id: control

	property BaseMap map: null

	spacing: 10

	visible: map && !map.downloaded

	Qaterial.LabelHint1 {
		anchors.verticalCenter: parent.verticalCenter
		horizontalAlignment: Text.AlignRight
		visible: map && map.downloadProgress >= 0
		text: map ? qsTr("Letöltés...\n%1%").arg(Math.floor(map.downloadProgress*100)) : ""
		color: Qaterial.Style.accentColor
	}

	Qaterial.ColorIcon
	{
		anchors.verticalCenter: parent.verticalCenter
		visible: map && !map.downloaded
		color: map && map.downloadProgress >= 0 ? Qaterial.Style.accentColor : Qaterial.Style.iconColor()
		source: Qaterial.Icons.download
	}
}

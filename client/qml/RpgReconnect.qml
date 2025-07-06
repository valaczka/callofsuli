import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as J

Rectangle {
	id: root

	color: Client.Utils.colorSetAlpha(Qaterial.Colors.black, 0.85)

	Column {
		anchors.centerIn: parent
		spacing: 30

		Row {
			anchors.horizontalCenter: parent.horizontalCenter

			spacing: 20

			Qaterial.BusyIndicator {
				anchors.verticalCenter: parent.verticalCenter
				height: txt.height
				width: txt.height
			}

			Qaterial.LabelWithCaption {
				id: txt
				anchors.verticalCenter: parent.verticalCenter
				text: qsTr("Újracsatlakozás...")
				caption: Client.server ? Client.server.url : ""
			}
		}

		Qaterial.RaisedButton {
			anchors.horizontalCenter: parent.horizontalCenter
			backgroundColor: Qaterial.Colors.red600
			foregroundColor: Qaterial.Colors.white
			text: qsTr("Megszakítás")
			icon.source: Qaterial.Icons.close
			onClicked: Client.stackPop()
		}
	}

}

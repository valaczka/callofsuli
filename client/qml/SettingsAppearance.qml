import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

Column {
	id: root

	spacing: 10 * Qaterial.Style.pixelSizeRatio

	Row {
		spacing: 12 * Qaterial.Style.pixelSizeRatio
		Qaterial.IconLabel {
			icon.source: Qaterial.Icons.formatFont
			anchors.verticalCenter: parent.verticalCenter
			text: qsTr("Betűméret:")
		}

		Qaterial.LabelHeadline6 {
			anchors.verticalCenter: parent.verticalCenter
			text: Qaterial.Style.userPixelSize
			color: Qaterial.Style.accentColor
		}

		Row {
			anchors.verticalCenter: parent.verticalCenter
			Qaterial.ToolButton {
				action: Client.mainWindow.fontMinus
				display: AbstractButton.IconOnly
			}

			Qaterial.ToolButton {
				action: Client.mainWindow.fontNormal
				display: AbstractButton.IconOnly
			}

			Qaterial.ToolButton {
				action: Client.mainWindow.fontPlus
				display: AbstractButton.IconOnly
			}
		}
	}

	Qaterial.SwitchButton {
		anchors.horizontalCenter: parent.horizontalCenter
		action: Client.mainWindow.actionFullScreen
		visible: Qt.platform.os == "osx" || Qt.platform.os == "windows" || Qt.platform.os == "linux"
	}

	QButton {
		icon.source: Qaterial.Icons.notificationClearAll
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("Felugró üzenetek olvasatlanná tevése")
		wrapMode: implicitWidth > parent.width ? Text.Wrap : Text.NoWrap
		onClicked: {
			JS.questionDialog({
								  onAccepted: function()
								  {
									  Client.Utils.settingsClear("notification")
									  Client.snack("Felugró üzenetek olvasatlanok")
								  },
								  text: qsTr("Biztosan olvasatlanná teszel minden felugró üzenetet?"),
								  iconSource: Qaterial.Icons.notificationClearAll,
								  title: qsTr("Felugró üzenetek")
							  })
		}
	}
}

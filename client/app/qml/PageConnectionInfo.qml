import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import QZXing 3.2
import "."
import "Style"
import "JScript.js" as JS



QBasePage {
	id: control

	defaultTitle: qsTr("Csatlakozási információk")

	mainToolBarComponent: QToolButton { action: actionCopy; display: AbstractButton.IconOnly }


	QStackComponent {
		id: stackComponent
		anchors.fill: parent
		basePage: control

		initialItem: QSimpleContainer {
			id: panel

			title: cosClient.serverName
			icon: CosStyle.iconComputerData

			Column {
				spacing: 20
				anchors.centerIn: parent

				QLabel {
					property var _d: cosClient.connectionInfoMap()

					textFormat: Text.StyledText

					text: qsTr("Host: <b>%1</b><br>Port: <b>%2</b><br>SSL: <b>%3</b>")
					.arg(_d.host).arg(_d.port).arg(_d.ssl ? qsTr("IGEN") : qsTr("Nem"))

					color: CosStyle.colorPrimaryLighter
					font.pixelSize: CosStyle.pixelSize*1.4

					anchors.horizontalCenter: parent.horizontalCenter
				}

				QLabel {
					id: label
					anchors.horizontalCenter: parent.horizontalCenter
					text: cosClient.connectionInfo()
					font.pixelSize: CosStyle.pixelSize*0.9
					font.weight: Font.Medium
					width: panel.panelWidth*0.8
					wrapMode: Text.Wrap
					horizontalAlignment: Text.AlignHCenter
				}

				Image {
					anchors.horizontalCenter: parent.horizontalCenter

					source: label.text.length ? "image://qrcode/"+Qt.btoa(label.text) : ""

					width: Math.min(panel.panelWidth*0.8, panel.panelHeight*0.7)
					height: Math.min(panel.panelWidth*0.8, panel.panelHeight*0.7)

					fillMode: Image.PreserveAspectFit

					sourceSize.width: 500
					sourceSize.height: 500
				}
			}

		}


	}


	Action {
		id: actionCopy
		text: qsTr("Másolás")
		icon.source: CosStyle.iconDuplicate
		onTriggered: {
			cosClient.textToClipboard(cosClient.connectionInfo())
		}
	}

	function windowClose() {
		return false
	}

	function pageStackBack() {
		if (stackComponent.layoutBack())
			return true

		return false
	}
}



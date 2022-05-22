import QtQuick 2.15
import QtQuick.Controls 2.15
import QZXing 3.3
import "."
import "Style"

QTabContainer {
	id: control

	title: cosClient.serverName
	icon: CosStyle.iconComputerData

	property string serverFunc: "connect"
	property var serverQueries: ({})
	property string displayText: ""

	property var _infoMap: cosClient.connectionInfoMap()
	property string _url: cosClient.connectionInfo(serverFunc, serverQueries)

	QTabHeader {
		id: hdr
		tabContainer: control
		isPlaceholder: true
	}


	Column {
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: hdr.bottom
		anchors.bottom: parent.bottom

		spacing: 20

		QLabel {
			anchors.horizontalCenter: parent.horizontalCenter

			textFormat: Text.StyledText

			text: qsTr("Host: <b>%1</b><br>Port: <b>%2</b><br>SSL: <b>%3</b>")
			.arg(_infoMap.host).arg(_infoMap.port).arg(_infoMap.ssl ? qsTr("Igen") : qsTr("Nem"))
			+control.displayText

			color: CosStyle.colorPrimaryLighter
			font.pixelSize: CosStyle.pixelSize*1.3

		}

		Image {
			anchors.horizontalCenter: parent.horizontalCenter

			source: _url.length ? "image://qrcode/"+Qt.btoa(_url) : ""

			width: Math.min(control.panelWidth*0.8, control.panelHeight*0.7)
			height: Math.min(control.panelWidth*0.8, control.panelHeight*0.7)

			fillMode: Image.PreserveAspectFit

			sourceSize.width: 500
			sourceSize.height: 500
		}


		QLabel {
			anchors.horizontalCenter: parent.horizontalCenter
			text: _url
			font.pixelSize: CosStyle.pixelSize*0.9
			font.weight: Font.Medium
			width: control.panelWidth*0.8
			wrapMode: Text.Wrap
			horizontalAlignment: Text.AlignHCenter
		}

	}
}

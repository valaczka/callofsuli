import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style"
import "JScript.js" as JS
import "."

QDialogPanel {
	id: item

	maximumWidth: 750

	Item {
		id: topItem
		anchors.top: parent.top
		width: parent.width
		height: Math.min(parent.height*0.5, 250)

		Column {
			width: parent.width
			anchors.verticalCenter: parent.verticalCenter
			spacing: 10

			QCosImage {
				id: cosImage
				width: parent.width
				height: CosStyle.pixelSize*2.5
				glow: false
			}

			QLabel {
				id: labelVersion
				horizontalAlignment: Qt.AlignHCenter
				wrapMode: Text.Wrap
				width: parent.width

				padding: 20

				text: qsTr("Verzió: ")+Qt.application.version+"\n© 2012-2022 Valaczka János Pál"
			}
		}
	}

	Flickable {
		id: flick

		anchors.top: topItem.bottom
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 5
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.leftMargin: 5
		anchors.rightMargin: 5

		contentWidth: row2.width
		contentHeight: row2.y+row2.height

		clip: true

		boundsBehavior: Flickable.StopAtBounds
		flickableDirection: Flickable.VerticalFlick

		Column {
			id: row2

			width: flick.width

			y: Math.max((flick.height-row2.height)/2, 0)

			QLabel {
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Credits")
				font.weight: Font.Medium
				font.pixelSize: CosStyle.pixelSize*0.9
				topPadding: 15
				bottomPadding: 5
			}

			QLabel {
				width: parent.width
				wrapMode: Text.Wrap
				text: cosClient.fileContent(":/CREDITS.md")
				font.pixelSize: CosStyle.pixelSize*0.8
				textFormat: Text.MarkdownText
				bottomPadding: 15

				onLinkActivated: cosClient.openUrl(link)
			}

			QLabel {
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("License")
				font.weight: Font.Medium
				font.pixelSize: CosStyle.pixelSize*0.9
				topPadding: 15
				bottomPadding: 5
			}

			QLabel {
				width: parent.width
				anchors.horizontalCenter: parent.horizontalCenter
				wrapMode: Text.Wrap
				text: cosClient.fileContent(":/common/license.txt")
				font.pixelSize: CosStyle.pixelSize*0.8
				bottomPadding: 15
			}


		}
	}


	buttons: QButton {
		id: buttonOk
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("Bezárás")
		icon.source: CosStyle.iconClose

		onClicked: item.dlgClose()
	}


	function populated() {
		buttonOk.forceActiveFocus()
	}

}

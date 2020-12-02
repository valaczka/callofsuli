import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.12
import COS.Client 1.0
import "Style"
import "JScript.js" as JS
import "."

Item {
	id: item

	implicitHeight: 300
	implicitWidth: 400

	property alias title: mainRow.title
	property alias text: labelText.text
	property alias value: progressBar.value

	property CosDownloader downloader: null

	signal dlgClose()
	property var acceptedData: true

	Connections {
		target: downloader
		function onActiveDownloadChanged(activeDownload) {
			labelText.text = activeDownload
		}

		function onDownloadProgressChanged(progress) {
			setValue(progress)
		}

		function onDownloadFinished() {
			dlgClose()
		}

		function onDownloadFailed() {
			dlgClose()
		}
	}

	Item {
		id: dlgItem
		anchors.centerIn: parent

		width: Math.min(parent.width*0.9, 650)
		height: Math.min(parent.height*0.9, 300)



		BorderImage {
			id: bgRectMask
			source: "qrc:/internal/img/border.svg"
			visible: false

			anchors.fill: bgRectData

			border.left: 15; border.top: 10
			border.right: 15; border.bottom: 10

			horizontalTileMode: BorderImage.Repeat
			verticalTileMode: BorderImage.Repeat
		}

		Rectangle {
			id: bgRectData

			anchors.fill: rectBg
			visible: false

			color: JS.setColorAlpha(CosStyle.colorPrimaryDark, 0.2)
		}

		OpacityMask {
			id: rectBg
			source: bgRectData
			maskSource: bgRectMask

			anchors.top: parent.top
			anchors.left: parent.left
			anchors.right: parent.right
			anchors.bottom: buttonNo.top
			anchors.bottomMargin: 10

			QDialogHeader {
				id: mainRow
				icon: CosStyle.iconDialogQuestion
			}


			DropShadow {
				anchors.fill: labelText
				horizontalOffset: 2
				verticalOffset: 2
				radius: 2
				samples: 3
				source: labelText
				visible: true
			}

			QLabel {
				id: labelText
				color: CosStyle.colorPrimaryLighter

				anchors.top: mainRow.bottom
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.margins: 30
				anchors.topMargin: 20
				wrapMode: Text.WordWrap
				horizontalAlignment: Qt.AlignLeft
				verticalAlignment: Qt.AlignVCenter
			}


			Item {
				anchors.top: labelText.bottom
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.margins: 30
				anchors.topMargin: 20
				anchors.bottomMargin: mainRow.padding


				ProgressBar {
					id: progressBar

					anchors.left: parent.left
					anchors.right: parent.right
					anchors.verticalCenter: parent.verticalCenter

					indeterminate: true

					Material.accent: CosStyle.colorOK
				}
			}

		}

		QButton {
			id: buttonNo
			anchors.right: parent.right
			anchors.bottom: parent.bottom
			text: qsTr("Megszakítás")
			icon.source: CosStyle.iconCancel
			themeColors: CosStyle.buttonThemeDelete

			onClicked: {
				acceptedData = false
				if (downloader)
					downloader.abort()
				else
					dlgClose()
			}
		}

	}

	function populated() {
		if (downloader)
			downloader.start()
	}

	function setValue(v) {
		progressBar.indeterminate = false
		progressBar.value = v
	}

}

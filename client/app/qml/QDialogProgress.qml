import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import COS.Client 1.0
import "Style"
import "JScript.js" as JS
import "."

QDialogPanel {
	id: item

	property string type: "info"            // info, warning, error
	property alias text: labelText.text
	property alias value: progressBar.value

	property CosDownloader downloader: null

	acceptedData: true

	maximumHeight: 250
	maximumWidth: 650

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

	Column {
		anchors.verticalCenter: parent.verticalCenter
		width: parent.width

		QLabel {
			id: labelText
			color: CosStyle.colorPrimaryLighter

			width: parent.width
			visible: text.length
			wrapMode: Text.WordWrap
			verticalAlignment: Qt.AlignVCenter
			horizontalAlignment: Qt.AlignLeft
			font.weight: Font.Medium
		}

		ProgressBar {
			id: progressBar

			width: parent.width*0.8
			anchors.horizontalCenter: parent.horizontalCenter

			indeterminate: true

			Material.accent: CosStyle.colorOK
		}
	}


	buttons: QButton {
		id: buttonNo
		anchors.right: parent.right
		text: qsTr("Megszakítás")
		icon.source: CosStyle.iconCancel
		themeColors: CosStyle.buttonThemeRed

		onClicked: {
			acceptedData = null
			if (downloader)
				downloader.abort()
			else
				dlgClose()
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

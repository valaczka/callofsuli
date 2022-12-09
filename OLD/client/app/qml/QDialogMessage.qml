import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style"
import "JScript.js" as JS
import "."

QDialogPanel {
	id: item

	property string type: "info"            // info, warning, error, success
	property alias text: labelText.text
	property alias details: labelDetails.text
	property alias image: bigImage.icon

	maximumHeight: 300
	maximumWidth: 750


	icon: if (type === "warning")
			  "qrc:/internal/icon/message-alert-outline.svg"
		  else if (type === "error")
			  "qrc:/internal/icon/message-alert.svg"
		  else
			  "qrc:/internal/icon/message-outline.svg"

	titleColor: if (type === "warning")
					CosStyle.colorWarning
				else if (type === "error")
					CosStyle.colorError
				else if (type === "success")
					CosStyle.colorOK
				else
					CosStyle.colorPrimary


	Row {
		id: rw

		anchors.centerIn: parent

		QFontImage {
			id: bigImage
			visible: icon != ""
			color: labelText.color
			size: Math.min(item.panel.height*0.5, CosStyle.pixelSize*4.5)
			width: visible ? size+CosStyle.pixelSize*2 : 0
			icon: ""

			anchors.verticalCenter: parent.verticalCenter
		}

		/*QLabel {
			id: labelText
			color: CosStyle.colorPrimaryLighter

			width: Math.min(rw.parent.width-bigImage.width, implicitWidth)
			anchors.verticalCenter: parent.verticalCenter

			font.weight: Font.Medium

			wrapMode: Text.WordWrap
			horizontalAlignment: Qt.AlignHCenter
			verticalAlignment: Qt.AlignVCenter
		}*/




		Column {
			anchors.verticalCenter: parent.verticalCenter
			width: Math.min(rw.parent.width-bigImage.width, Math.max(labelText.implicitWidth, labelDetails.implicitWidth))

			QLabel {
				id: labelText
				color: if (type === "warning")
						   CosStyle.colorWarningLighter
					   else if (type === "error")
						   CosStyle.colorErrorLighter
					   else if (type === "success")
						   CosStyle.colorOKLighter
					   else
						   CosStyle.colorPrimaryLighter

				width: parent.width
				wrapMode: Text.WordWrap
				horizontalAlignment: Qt.AlignHCenter
				verticalAlignment: Qt.AlignVCenter
				font.weight: Font.Medium
			}

			QLabel {
				id: labelDetails
				color: labelText.color

				visible: text.length

				width: parent.width
				wrapMode: Text.WordWrap
				horizontalAlignment: Qt.AlignHCenter
				verticalAlignment: Qt.AlignVCenter
				font.weight: Font.Light
				//font.pixelSize: labelText.font.pixelSize*0.9
			}
		}
	}

	buttons: QButton {
		id: buttonOk
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("OK")
		icon.source: "qrc:/internal/icon/check-bold.svg"
		themeColors: type === "success" ? CosStyle.buttonThemeGreen : CosStyle.buttonThemeDefault

		onClicked: item.dlgClose()
	}


	function populated() {
		buttonOk.forceActiveFocus()
	}

}

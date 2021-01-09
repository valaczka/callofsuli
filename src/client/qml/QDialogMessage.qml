import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style"
import "JScript.js" as JS
import "."

QDialogPanel {
	id: item

	property string type: "info"            // info, warning, error
	property alias text: labelText.text
	property alias details: labelDetails.text

	maximumHeight: 300
	maximumWidth: 750


	icon: if (type === "warning")
			  CosStyle.iconDialogWarning
		  else if (type === "error")
			  CosStyle.iconDialogError
		  else
			  CosStyle.iconDialogInfo

	titleColor: if (type === "warning")
					CosStyle.colorWarning
				else if (type === "error")
					CosStyle.colorError
				else
					CosStyle.colorPrimary

	Column {
		anchors.verticalCenter: parent.verticalCenter
		width: parent.width

		QLabel {
			id: labelText
			color: if (type === "warning")
					   CosStyle.colorWarningLighter
				   else if (type === "error")
					   CosStyle.colorErrorLighter
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

	buttons: QButton {
		id: buttonOk
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("OK")
		icon.source: CosStyle.iconOK

		onClicked: item.dlgClose()
	}


	function populated() {
		buttonOk.forceActiveFocus()
	}

}

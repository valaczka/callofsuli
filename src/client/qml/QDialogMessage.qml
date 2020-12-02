import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS
import "."

Item {
	id: item

	implicitHeight: 300
	implicitWidth: 400

	property string type: "info"            // info, warning, error
	property alias title: mainRow.title
	property alias text: labelText.text
	property alias details: labelDetails.text

	signal dlgClose()
	property var acceptedData: true

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
			anchors.bottom: buttonOk.top
			anchors.bottomMargin: 10

			QDialogHeader {
				id: mainRow
				icon: if (type === "warning")
						  CosStyle.iconDialogWarning
					  else if (type === "error")
						  CosStyle.iconDialogError
					  else
						  CosStyle.iconDialogInfo

				color: if (type === "warning")
						   CosStyle.colorWarning
					   else if (type === "error")
						   CosStyle.colorError
					   else
						   CosStyle.colorPrimary

			}



			Item {
				id: lbls
				anchors.top: mainRow.bottom
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.bottom: parent.bottom
				anchors.margins: 30
				anchors.topMargin: 20

				Column {
					width: parent.width
					anchors.verticalCenter: parent.verticalCenter

					Item {
						width: parent.width
						height: Math.max(shadow.height, labelText.height)

						DropShadow {
							id: shadow
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
						}
					}

					QLabel {
						id: labelDetails
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
						font.weight: Font.Light
						//font.pixelSize: labelText.font.pixelSize*0.9
					}
				}
			}
		}

		QButton {
			id: buttonOk
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.bottom: parent.bottom
			text: qsTr("OK")
			icon.source: CosStyle.iconOK

			onClicked: item.dlgClose()
		}

	}

	function populated() {
		buttonOk.forceActiveFocus()
	}

}

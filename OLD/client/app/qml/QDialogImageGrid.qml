import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import SortFilterProxyModel 0.2
import "Style"
import "JScript.js" as JS
import "."

QDialogPanel {
	id: item

	property alias grid: grid
	property alias model: grid.model

	property alias cellWidth: grid.cellWidth
	property alias cellHeight: grid.cellHeight

	property color currentBorderColor: CosStyle.colorAccent
	property bool clearEnabled: true

	property string modelImagePattern: "%1"
	property int modelImageHeight: grid.cellHeight*0.9
	property int modelImageWidth: grid.cellWidth*0.9

	property string currentValue: ""

	icon: "qrc:/internal/icon/message-image.svg"

	maximumHeight: 0
	maximumWidth: 700

	acceptedData: null

	GridView {
		id: grid

		anchors.fill: parent
		model: item.model

		cellHeight: 75
		cellWidth: 75

		clip: true

		delegate: Rectangle {
			id: delegateRect
			width: grid.cellWidth
			height: grid.cellHeight
			color: "transparent"
			border.color: currentBorderColor
			border.width: GridView.isCurrentItem ? 1 : 0

			required property string modelData
			required property int index

			MouseArea {
				id: area
				anchors.fill: parent
				acceptedButtons: Qt.LeftButton


				Image {
					source: modelImagePattern.arg(delegateRect.modelData)
					width: modelImageWidth
					height: modelImageHeight
					fillMode: Image.PreserveAspectFit
					anchors.centerIn: parent
				}

				onClicked: grid.currentIndex = delegateRect.index

				onDoubleClicked: {
					grid.currentIndex = delegateRect.index
					acceptedData = delegateRect.modelData
					dlgClose()
				}
			}
		}
	}


	buttons: Row {
		id: buttonRow
		spacing: 10

		anchors.horizontalCenter: parent.horizontalCenter

		QButton {
			id: buttonClear
			anchors.verticalCenter: parent.verticalCenter
			text: qsTr("Törlés")
			icon.source: "qrc:/internal/icon/delete.svg"
			themeColors: CosStyle.buttonThemeOrange

			visible: clearEnabled
			enabled: clearEnabled

			onClicked: {
				acceptedData = ""
				dlgClose()
			}
		}

		QButton {
			id: buttonNo
			anchors.verticalCenter: parent.verticalCenter
			text: qsTr("Mégsem")
			icon.source: "qrc:/internal/icon/close-circle.svg"
			themeColors: CosStyle.buttonThemeRed

			onClicked: dlgClose()
		}

		QButton {
			id: buttonYes

			anchors.verticalCenter: parent.verticalCenter

			text: qsTr("OK")
			icon.source: "qrc:/internal/icon/check-bold.svg"
			themeColors: CosStyle.buttonThemeGreen

			onClicked: {
				if (grid.currentIndex != -1)
					acceptedData = grid.model[grid.currentIndex]

				dlgClose()
			}
		}
	}



	function populated() {
		if (currentValue.length && grid.model) {
			for (var i=0; i<grid.model.length; i++) {
				if (grid.model[i] === currentValue) {
					grid.currentIndex = i
					grid.positionViewAtIndex(i, GridView.Contain)
					break
				}
			}
		}

		grid.forceActiveFocus()
	}

}

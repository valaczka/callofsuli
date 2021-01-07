import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	layoutFillWidth: true

	property int selectedMissionIndex: -1
	property var selectedData: null

	title: selectedData ? qsTr("Küldetés") : ""
	icon: CosStyle.iconUser

	Connections {
		target: studentMaps

		function onMissionSelected(index) {
			selectedMissionIndex = index
			if (selectedMissionIndex == -1)
				selectedData = null
			else {
				if (swipeMode)
					parent.parentPage.swipeToPage(1)

				var x = studentMaps.modelMissionList.get(selectedMissionIndex)
				if (Object.keys(x).length)
					selectedData = x
				else
					selectedData = null
			}
		}

		function onMissionListChanged() {
			if (selectedMissionIndex == -1)
				selectedData = null
			else {
				var x = studentMaps.modelMissionList.get(selectedMissionIndex)
				if (Object.keys(x).length)
					selectedData = x
				else {
					selectedData = null
					selectedMissionIndex = -1
				}
			}
		}
	}

	QLabel {
		id: noLabel
		opacity: selectedData ? 0.0 : 1.0
		visible: opacity != 0

		anchors.centerIn: parent

		text: qsTr("Válassz küldetést")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}

	QLabel {
		id: titleLabel
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: parent.top

		opacity: text.length ? 1.0 : 0.0
		visible: opacity != 0

		text: selectedData ? selectedData.name : ""

		topPadding: 20
		leftPadding: 10
		rightPadding: 10
		bottomPadding: 20

		font.family: "HVD Peace"
		font.pixelSize: CosStyle.pixelSize*1.4
	}

	Column {
		anchors.top: titleLabel.bottom
		anchors.centerIn: parent

		opacity: selectedData ? 1.0 : 0.0
		visible: opacity != 0

		spacing: 5

		Repeater {
			id: rptr
			model: selectedData ? selectedData.levels : []

			Row {
				spacing: 10
				anchors.horizontalCenter: parent.horizontalCenter

				QButton {
					id: btn
					text: qsTr("Level %1").arg(modelData.level)
					anchors.verticalCenter: parent.verticalCenter
					icon.source: modelData.available ? CosStyle.iconPlay : CosStyle.iconLock
					enabled: modelData.available
					onClicked: {
						studentMaps.playGame({uuid: selectedData.uuid, level: modelData.level})
					}
				}

				QFontImage {
					anchors.verticalCenter: parent.verticalCenter

					height: btn.height*0.75
					width: height
					size: height*0.6

					icon: CosStyle.iconOK

					visible: modelData.solved

					color: CosStyle.colorOKLight
				}
			}
		}
	}
}




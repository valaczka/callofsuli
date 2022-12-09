import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QDialogPanel {
	id: dialogPanel


	icon: CosStyle.iconMedal

	required property StudentMaps studentMaps
	property string mapUuid: ""

	maximumWidth: 1200


	QAccordion {
		id: accordion
		anchors.fill: parent
		anchors.margins: 5

		Repeater {
			model: ListModel {
				ListElement {
					filterLevel: 1
					filterDeathmatch: false
				}
				ListElement {
					filterLevel: 1
					filterDeathmatch: true
				}
				ListElement {
					filterLevel: 2
					filterDeathmatch: false
				}
				ListElement {
					filterLevel: 2
					filterDeathmatch: true
				}
				ListElement {
					filterLevel: 3
					filterDeathmatch: false
				}
				ListElement {
					filterLevel: 3
					filterDeathmatch: true
				}

			}

			GridView {
				id: grid
				width: accordion.width
				height: contentHeight

				interactive: false

				model: SortFilterProxyModel {
					sourceModel: studentMaps.modelMedalList
					filters: AllOf {
						ValueFilter {
							roleName: "level"
							value: filterLevel
						}
						ValueFilter {
							roleName: "deathmatch"
							value: filterDeathmatch
						}
					}
				}

				cellWidth: 100
				cellHeight: 100

				delegate: Item {
					id: item

					width: grid.cellWidth
					height: grid.cellHeight

					required property bool deathmatch
					required property int level
					required property string image
					required property bool solved

					property real imageWidth: width-10
					property real imageHeight: height-10

					QImageInnerShadow {
						width: item.imageWidth
						height: item.imageHeight

						anchors.centerIn: parent

						image: "qrc:/internal/img/borderIcon.svg"
						contentItem: dialogPanel.metalBgTexture

						brightness: -0.5

						visible: !item.solved
					}

					QMedalImage {
						level: item.level
						isDeathmatch: item.deathmatch
						image: item.image

						width: item.imageWidth
						height: item.imageHeight
						anchors.centerIn: parent

						visible: item.solved
					}
				}
			}

		}
	}


	buttons: QButton {
		id: buttonOk
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("OK")
		icon.source: "qrc:/internal/icon/check-bold.svg"

		onClicked: dialogPanel.dlgClose()
	}


	function populated() {
		if (mapUuid.length)
			studentMaps.send("missionListGet", {map: mapUuid})
		else
			studentMaps.getMissionList()
		buttonOk.forceActiveFocus()
	}

	Component.onCompleted: studentMaps.modelMedalList.clear()
}



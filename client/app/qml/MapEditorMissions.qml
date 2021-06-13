import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QSwipeComponent {
	id: swComponent
	anchors.fill: parent

	implicitWidth: 1200

	headerContent: QRectangleBg {
		id: rectBg
		height: Math.max(labelMissionName.implicitHeight, imgMissionsMenu.implicitHeight, buttonMissionNew.implicitHeight, 32)
		width: parent.width

		Row {
			anchors.centerIn: parent
			spacing: 5

			Image {
				id: imgMissionMedal
				height: rectBg.height*0.9
				width: height
				fillMode: Image.PreserveAspectFit
				anchors.verticalCenter: parent.verticalCenter
			}

			QLabel {
				id: labelMissionName
				anchors.verticalCenter: parent.verticalCenter
				width: Math.min(implicitWidth, rectBg.width-imgMissionMedal.width-imgMissionsMenu.width-2*buttonMissionNew.width)
				elide: Text.ElideRight
				font.pixelSize: CosStyle.pixelSize*1.15
				font.weight: Font.Medium
				color: CosStyle.colorAccentLight
				topPadding: 5
				bottomPadding: 5

				property string _textFromData: ""
				text: mapEditor.currentMission.length ? _textFromData : qsTr("-- válassz küldetést --")
			}

			QFontImage {
				id: imgMissionsMenu
				//visible: titleItem.acceptedButtons != Qt.NoButton

				icon: CosStyle.iconDown
				color: CosStyle.colorPrimaryLight

				anchors.verticalCenter: parent.verticalCenter
			}
		}

		QToolButton {
			id: buttonMissionNew
			action: actionMissionNew
			anchors.right: parent.right
			anchors.verticalCenter: parent.verticalCenter
			display: AbstractButton.IconOnly
			color: CosStyle.colorOKLighter
		}

		acceptedButtons: Qt.LeftButton
		mouseArea.onClicked: {
			var d = JS.dialogCreateQml("List", {
										   roles: ["name", "medalImage"],
										   icon: CosStyle.iconUser,
										   title: qsTr("Válassz küldetést"),
										   selectorSet: false,
										   modelImageRole: "medalImage",
										   modelImagePattern: "qrc:/internal/medals/%1",
										   delegateHeight: CosStyle.twoLineHeight,
										   sourceModel: mapEditor.modelMissionList
									   })

			if (mapEditor.currentMission.length)
				d.item.selectCurrentItem("uuid", mapEditor.currentMission)

			d.accepted.connect(function(data) {
				if (data === -1)
					return

				var p = d.item.sourceModel.get(data)
				mapEditor.currentMission = p.uuid
			})
			d.open()
		}
	}

	content: [
		MapEditorMissionDetails {
			id: container1
			reparented: swComponent.swipeMode
			reparentedParent: placeholder1
			enabled: mapEditor.currentMission.length
		},

		MapEditorMissionLevel {
			id: container2
			reparented: swComponent.swipeMode
			reparentedParent: placeholder2
			level: 1
			enabled: mapEditor.currentMission.length
		},
		MapEditorMissionLevel {
			id: container3
			reparented: swComponent.swipeMode
			reparentedParent: placeholder3
			level: 2
			enabled: mapEditor.currentMission.length
		},
		MapEditorMissionLevel {
			id: container4
			reparented: swComponent.swipeMode
			reparentedParent: placeholder4
			level: 3
			enabled: mapEditor.currentMission.length
		}

	]

	swipeContent: [
		Item { id: placeholder1 },
		Item { id: placeholder2 },
		Item { id: placeholder3 },
		Item { id: placeholder4 }
	]

	tabBarContent: [
		QSwipeButton { swipeContainer: container1 },
		QSwipeButton { swipeContainer: container2 },
		QSwipeButton { swipeContainer: container3 },
		QSwipeButton { swipeContainer: container4 }
	]


	Action {
		id: actionMissionNew
		icon.source: CosStyle.iconAdd
		text: qsTr("Új küldetés")
	}


	Connections {
		target: mapEditor

		function onCurrentMissionDataChanged(data) {
			var mi = data.medalImage
			if (mi && mi.length)
				imgMissionMedal.source = "qrc:/internal/medals/"+mi
			else
				imgMissionMedal.source = ""

			labelMissionName._textFromData = data.name ? data.name : ""
		}
	}

	Component.onDestruction: mapEditor.currentMission = ""
}

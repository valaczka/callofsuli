import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QCollapsible {
	id: control

	required property bool selected

	property GameMapEditorMission self: null

	property bool editable: true

	backgroundColor: "transparent"
	titleColor: CosStyle.colorWarningLighter
	lineColor: "transparent"
	itemSelected: selected

	signal missionRemove()

	title: self && (selected || collapsed)? self.name : ""

	rightComponent: Row {
		spacing: 0
		/*QToolButton {
			anchors.verticalCenter: parent.verticalCenter
			action: actionMissionEdit
			color: CosStyle.colorAccent
			display: AbstractButton.IconOnly
			visible: !control.collapsed && !control.editable
		}*/
		QToolButton {
			anchors.verticalCenter: parent.verticalCenter
			action: actionMissionRemove
			color: CosStyle.colorErrorLighter
			display: AbstractButton.IconOnly
		}
	}

	QMenu {
		id: contextMenu

		MenuItem { action: actionMissionRemove }
	}

	onRightClicked: contextMenu.open()

	Item {
		id: controlContent
		width: parent.width
		height: contentLayout.height+levelRow.height

		QGridLayout {
			id: contentLayout
			watchModification: false
			anchors.top: parent.top

			QGridImageTextField {
				id: imageTextName
				fieldName: qsTr("Küldetés neve")

				textfield.font.family: "Rajdhani"
				textfield.font.pixelSize: CosStyle.pixelSize*1.5
				textfield.textColor: CosStyle.colorWarningLighter
				textfield.readOnly: !control.editable
				textfield.onTextModified: mapEditor.missionModify({name: textfield.text})

				text: self ? self.name : ""

				image: self && self.medalImage != "" ? cosClient.medalIconPath(self.medalImage) : ""

				mousearea.enabled: control.editable
				mousearea.onClicked:  {
					var d = JS.dialogCreateQml("ImageGrid", {
												   model: cosClient.medalIcons(),
												   icon: CosStyle.iconMedal,
												   title: qsTr("Küldetés medálképe"),
												   modelImagePattern: "qrc:/internal/medals/%1",
												   clearEnabled: false,
												   currentValue: self.medalImage
											   })

					d.accepted.connect(function(data) {
						mapEditor.missionModify({medalImage: data})
					})

					d.open()
				}
			}

			QGridLabel {
				field: areaDetails
			}

			QGridTextArea {
				id: areaDetails
				fieldName: qsTr("Leírás")
				placeholderText: qsTr("Rövid ismertető leírás a küldetésről")
				minimumHeight: CosStyle.baseHeight*2
				readOnly: !control.editable
				color: CosStyle.colorWarning

				text: self ? self.description : ""

				onTextModified: mapEditor.missionModify({description: text})
			}
		}


		Row {
			id: levelRow
			anchors.top: contentLayout.bottom
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: 5

			topPadding: 10
			bottomPadding: 20

			Repeater {
				id: groupRepeater

				model: self ? self.levels : null

				QCard {
					id: levelItem
					height: CosStyle.pixelSize*4.5
					width: height

					backgroundColor: CosStyle.colorAccent

					/*onClicked: {
						if (cosClient.userRoles & Client.RoleTeacher)
							JS.createPage("TeacherGroup", {
											  groupId: modelData.id
										  })
						else
							JS.createPage("StudentGroup", {
											  title: modelData.name+(modelData.readableClassList !== "" ? " | "+modelData.readableClassList : ""),
											  groupId: modelData.id,
											  profile: profile
										  })
					}*/

					required property int index

					property GameMapEditorMissionLevel levelSelf: self.levels.object(index)

					onLevelSelfChanged: if (!levelSelf) {
											delete levelItem
										}

					onClicked: {
						mapEditor.openMissionLevel({
													   missionLevel: levelSelf
												   })
					}

					Column {
						anchors.centerIn: parent
						spacing: 0

						QLabel {
							anchors.horizontalCenter: parent.horizontalCenter
							color: "white"
							font.weight: Font.DemiBold
							font.pixelSize: CosStyle.pixelSize*2
							text: levelItem.levelSelf.level
						}

						QLabel {
							anchors.horizontalCenter: parent.horizontalCenter
							color: "white"
							font.weight: Font.Medium
							font.pixelSize: CosStyle.pixelSize*0.9
							font.capitalization: Font.AllUppercase
							text: "Level"
						}
					}
				}
			}

			QCard {
				id: levelAddItem
				height: CosStyle.pixelSize*4.5
				width: height

				visible: self && self.levels.count < 3

				backgroundColor: CosStyle.colorOKDark

				QLabel {
					anchors.centerIn: parent
					color: CosStyle.colorOKLighter
					font.weight: Font.Bold
					font.pixelSize: CosStyle.pixelSize*3
					text: "+"
				}


			}
		}



		Rectangle {
			anchors.bottom: parent.bottom
			anchors.left: parent.left
			width: parent.width
			height: 0.5
			color: Qt.darker(CosStyle.colorAccentDark)
		}
	}




	Action {
		id: actionMissionEdit

		icon.source: CosStyle.iconEdit
		text: qsTr("Szerkesztés")

		onTriggered: control.editable = true
	}

	Action {
		id: actionMissionRemove

		icon.source: CosStyle.iconDelete
		text: qsTr("Törlés")

		onTriggered: control.missionRemove()
	}
}

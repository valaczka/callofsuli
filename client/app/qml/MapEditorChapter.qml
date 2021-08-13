import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QCollapsible {
	id: control

	required property string name
	required property int chapter
	required property int missionCount
	required property int objectiveCount
	required property int level

	//title: "%1 (#%2)".arg(name).arg(chapter)
	title: name
	titleColor: CosStyle.colorOKLight
	backgroundColor: CosStyle.colorOKDark
	contentBackgroundColor: JS.setColorAlpha(CosStyle.colorOKDarkest, 0.8)

	rightComponent: Row {
		spacing: 2
		QBadge {
			text: missionCount
			color: CosStyle.colorWarningDark
			anchors.verticalCenter: parent.verticalCenter
			visible: missionCount > (level>0 ? 1 : 0)
		}
		QBadge {
			text: objectiveCount
			color: CosStyle.colorPrimaryDark
			anchors.verticalCenter: parent.verticalCenter
			visible: objectiveCount > 0
		}
		QToolButton {
			anchors.verticalCenter: parent.verticalCenter
			icon.source: CosStyle.iconMenu
			color: control.titleColor

			QMenu {
				id: chapterMenu

				MenuItem { action: actionChapterMissions }
				MenuItem { action: actionChapterRename }
				MenuSeparator { }
				MenuItem { action: actionChapterRemove }

			}

			onClicked: chapterMenu.open()

			Connections {
				target: control
				function onRightClicked() {
					chapterMenu.open()
				}
			}
		}
	}

	Column {

		Repeater {
			model: SortFilterProxyModel {
				sourceModel: mapEditor.modelObjectiveList

				filters: ValueFilter {
					roleName: "chapter"
					value: control.chapter
				}

				sorters: RoleSorter {
					roleName: "sortid"
				}
			}

			Item {
				id: item
				width: control.width
				height: CosStyle.twoLineHeight*1.7

				required property string uuid
				required property string objectiveModule
				required property string objectiveData
				required property int storage
				required property int storageCount
				required property string storageData
				required property string storageModule

				readonly property var _info: cosClient.objectiveInfo(objectiveModule, objectiveData, storageModule, storageData)

				readonly property color mainColor: storage ? CosStyle.colorAccent : CosStyle.colorWarningLight

				QRectangleBg {
					id: rect
					anchors.fill: parent
					acceptedButtons: Qt.LeftButton | Qt.RightButton

					QLabel {
						id: labelName
						text: _info.name
						color: mainColor
						font.weight: Font.DemiBold
						font.pixelSize: CosStyle.pixelSize*0.6
						font.capitalization: Font.AllUppercase
						anchors.left: parent.left
						anchors.top: parent.top
						anchors.leftMargin: 3
						anchors.topMargin: 1
					}

					Row {
						anchors.verticalCenter: parent.verticalCenter

						spacing: 0

						QFontImage {
							id: imgModule
							width: Math.max(rect.height*0.5, size*1.1)
							size: CosStyle.pixelSize*1.5
							anchors.verticalCenter: parent.verticalCenter
							icon: _info.icon
							color: mainColor
						}


						Column {
							anchors.verticalCenter: parent.verticalCenter
							anchors.verticalCenterOffset: (labelName.height/2)*(subtitle.lineCount-1)/3

							QLabel {
								id: title
								anchors.left: parent.left
								width: rect.width-imgModule.width-btnMenu.width
									   -(badge.visible ? badge.width : 0)
								text: _info.title
								color: mainColor
								font.pixelSize: CosStyle.pixelSize*1.1
								font.weight: Font.Normal
								maximumLineCount: 1
								lineHeight: 0.9
								elide: Text.ElideRight
							}
							QLabel {
								id: subtitle
								anchors.left: parent.left
								width: title.width
								text: _info.details
								color: mainColor
								font.pixelSize: CosStyle.pixelSize*0.75
								font.weight: Font.Light
								maximumLineCount: 3
								lineHeight: 0.8
								wrapMode: Text.Wrap
								elide: Text.ElideRight
							}
						}

						QBadge {
							id: badge
							text: storageCount
							color: CosStyle.colorPrimaryDark
							anchors.verticalCenter: parent.verticalCenter
							visible: storageCount > 1
						}


						QToolButton {
							id: btnMenu
							anchors.verticalCenter: parent.verticalCenter
							icon.source: CosStyle.iconMenu

							QMenu {
								id: objectiveMenu
								MenuItem {
									//icon.source: CosStyle.iconRename
									text: qsTr("Másolás")

									onClicked: control.objectiveMoveCopy(item.uuid, true)
								}
								MenuItem {
									//icon.source: CosStyle.iconBooks
									text: qsTr("Áthelyezés")

									onClicked: control.objectiveMoveCopy(item.uuid, false)
								}
								MenuItem {
									icon.source: CosStyle.iconDuplicate
									text: qsTr("Kettőzés")

									onClicked: mapEditor.objectiveCopy({uuid: item.uuid, chapter: control.chapter})
								}

								MenuSeparator { }

								MenuItem {
									icon.source: CosStyle.iconDelete
									text: qsTr("Törlés")

									onClicked: mapEditor.objectiveRemove({uuid: item.uuid})
								}
							}

							onClicked: objectiveMenu.open()
						}
					}


					mouseArea.onClicked: {
						if (mouse.button === Qt.LeftButton) {
							//mapEditor.loadQuestion()

						} else if (mouse.button === Qt.RightButton) {
							objectiveMenu.open()
						}

					}
				}

				Rectangle {
					anchors.bottom: parent.bottom
					anchors.left: parent.left
					width: parent.width
					height: 1
					color: CosStyle.colorOKDark
				}
			}
		}

		QToolButtonFooter {
			anchors.horizontalCenter: parent.horizontalCenter
			icon.source: CosStyle.iconAdd
			text: qsTr("Új feladat")
			//onClicked: mapEditor.missionLevelGetChapterList(level)
		}
	}


	Action {
		id: actionChapterRename

		icon.source: CosStyle.iconRename
		text: qsTr("Átnevezés")

		onTriggered: {
			var d = JS.dialogCreateQml("TextField", { title: qsTr("Szakasz neve"), value: name })

			d.accepted.connect(function(data) {
				if (data.length)
					mapEditor.chapterModify({chapter: chapter, name: data})
			})
			d.open()
		}

	}



	Action {
		id: actionChapterMissions

		icon.source: CosStyle.iconBooks
		text: qsTr("Küldetések")

		onTriggered: mapEditor.chapterGetMissionList({chapter: chapter, name: name})
	}



	Action {
		id: actionChapterRemove

		icon.source: level > 0 ? CosStyle.iconRemove : CosStyle.iconDelete
		text: level > 0 ? qsTr("Eltávolítás") : qsTr("Törlés")

		onTriggered: if (level > 0)
						 mapEditor.missionLevelChapterRemove({chapter: chapter, level: level})
					 else
						 mapEditor.chapterRemove({chapter: chapter})
	}





	function objectiveMoveCopy(uuid, isCopy) {
		var d = JS.dialogCreateQml("List", {
									   icon: CosStyle.iconLockAdd,
									   title: isCopy ? qsTr("Feladat másolása") : qsTr("Feladat áthelyezése"),
									   roles: ["name", "chapter"],
									   modelTitleRole: "name",
									   selectorSet: false,
									   sourceModel: mapEditor.modelChapterList
								   })


		d.accepted.connect(function(data) {
			if (data === -1)
				return

			var p = d.item.sourceModel.get(data)

			if (isCopy)
				mapEditor.objectiveCopy({uuid: uuid, chapter: p.chapter})
			else
				mapEditor.objectiveModify({uuid: uuid, chapter: p.chapter})
		})
		d.open()
	}
}

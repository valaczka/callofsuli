import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QCollapsible {
	id: control

	required property int level
	required property bool selected

	property GameMapEditorChapter self: null

	signal chapterRemove()

	title: self ? self.name : ""
	itemSelected: selected

	rightComponent: Row {
		spacing: 2
		QBadge {
			text: self ? self.missionCount : ""
			color: CosStyle.colorAccentDark
			anchors.verticalCenter: parent.verticalCenter
			visible: self && self.missionCount > (level>0 ? 1 : 0)
		}
		QBadge {
			text: self ? self.objectiveCount : ""
			color: CosStyle.colorPrimaryDarker
			anchors.verticalCenter: parent.verticalCenter
			visible: self && self.objectiveCount > 0
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


	QObjectListDelegateView {
		id: objectiveList
		width: parent.width

		selectorSet: self && self.objectives.selectedCount

		model: self ? self.objectives : null

		delegate: Item {
			id: item
			width: objectiveList.width
			height: CosStyle.twoLineHeight*1.7

			required property bool selected
			required property int index

			property GameMapEditorObjective objectiveSelf: objectiveList.modelObject(index)

			readonly property color mainColor: objectiveSelf && objectiveSelf.storageId > 0 ?
												   CosStyle.colorOKLighter :
												   CosStyle.colorAccent

			property bool selectorSet: objectiveList.selectorSet


			onObjectiveSelfChanged: if (!objectiveSelf) {
										delete item
									}

			QRectangleBg {
				id: rect
				anchors.fill: parent
				acceptedButtons: Qt.LeftButton | Qt.RightButton

				QLabel {
					id: labelName
					text: objectiveSelf ? objectiveSelf.info[0] : ""
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
						icon: objectiveSelf ? objectiveSelf.info[1] : ""
						color: mainColor
						visible: !item.selectorSet
					}

					QFlipable {
						id: flipable
						width: imgModule.width
						height: fontSize*1.1
						fontSize: CosStyle.pixelSize*1.5

						anchors.verticalCenter: parent.verticalCenter

						visible: item.selectorSet

						mouseArea.enabled: false

						frontIcon: CosStyle.iconUnchecked
						backIcon: CosStyle.iconChecked
						color: CosStyle.colorAccent
						flipped: item.selected
					}


					Column {
						anchors.verticalCenter: parent.verticalCenter
						anchors.verticalCenterOffset: (labelName.height/2)*(subtitle.lineCount-1)/3

						QLabel {
							id: title
							anchors.left: parent.left
							width: rect.width-imgModule.width-btnMenu.width
								   -(badge.visible ? badge.width : 0)
							text: objectiveSelf ? objectiveSelf.info[2] : ""
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
							text: objectiveSelf ? objectiveSelf.info[3] : ""
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
						text: objectiveSelf ? objectiveSelf.storageCount : ""
						color: CosStyle.colorPrimaryDarker
						anchors.verticalCenter: parent.verticalCenter
						visible: objectiveSelf && objectiveSelf.storageCount > 1
					}


					QToolButton {
						id: btnMenu
						anchors.verticalCenter: parent.verticalCenter
						icon.source: CosStyle.iconMenu

						ListModel {
							id: _filteredChaptersModel
						}

						QMenu {
							id: objectiveMenu

							QMenu {
								id: submenuCopy
								title: qsTr("Másolás")

								MenuItem {
									text: "Új szakasz"
									onClicked: objectiveMoveCopy(-1, true, item.objectiveSelf)
								}

								MenuSeparator { }

								Instantiator {
									model: _filteredChaptersModel

									MenuItem {
										text: model.name
										onClicked: objectiveMoveCopy(model.id, true, item.objectiveSelf)
									}

									onObjectAdded: submenuCopy.insertItem(index+2, object)
									onObjectRemoved: submenuCopy.removeItem(object)
								}
							}

							QMenu {
								id: submenuMove
								title: qsTr("Áthelyezés")

								MenuItem {
									text: "Új szakasz"
									onClicked: objectiveMoveCopy(-1, false, item.objectiveSelf)
								}

								MenuSeparator { }

								Instantiator {
									model: _filteredChaptersModel

									MenuItem {
										text: model.name
										onClicked: objectiveMoveCopy(model.id, false, item.objectiveSelf)
									}

									onObjectAdded: submenuMove.insertItem(index+2, object)
									onObjectRemoved: submenuMove.removeItem(object)
								}
							}

							MenuItem {
								icon.source: CosStyle.iconDuplicate
								text: qsTr("Kettőzés")
								enabled: self.objectives.selectedCount === 0

								onClicked: mapEditor.openObjective({
																	   objective: item.objectiveSelf,
																	   chapter: control.self,
																	   duplicate: true
																   })
							}

							MenuSeparator { }

							MenuItem {
								icon.source: "qrc:/internal/icon/delete.svg"
								text: qsTr("Törlés")

								onClicked: {
									if (self.objectives.selectedCount > 0) {
										mapEditor.objectiveRemoveList(self, self.objectives.getSelected())
									} else {
										mapEditor.objectiveRemove(self, objectiveSelf)
									}
								}
							}
						}

						onClicked: {
							_filteredChaptersModel.clear()
							for (var i=0; i<mapEditor.editor.chapters.count; i++) {
								var d=mapEditor.editor.chapters.object(i)
								if (d.id !== self.id)
									_filteredChaptersModel.append({id: d.id, name: d.name})
							}

							objectiveMenu.open()
						}

					}
				}


				mouseArea.onClicked: {
					if (mouse.button === Qt.LeftButton) {
						if (item.selectorSet)
							objectiveList.onDelegateClicked(index, mouse.modifiers & Qt.ShiftModifier)
						else {
							mapEditor.openObjective({
														objective: item.objectiveSelf,
														chapter: control.self
													})
						}

					} else if (mouse.button === Qt.RightButton) {
						btnMenu.clicked()
					}
				}



				mouseArea.onPressAndHold: {
					if (mouse.button === Qt.LeftButton) {
						objectiveList.onDelegateLongClicked(index)
					}
				}
			}
		}


		footer: QToolButtonFooter {
			width: objectiveList.width
			icon.source: CosStyle.iconAdd
			text: qsTr("Új feladat")
			color: CosStyle.colorAccentLighter
			onClicked: mapEditor.openObjective({
												   chapter: control.self
											   })
		}
	}




	Action {
		id: actionChapterRename

		icon.source: CosStyle.iconRename
		text: qsTr("Átnevezés")

		onTriggered: {
			var d = JS.dialogCreateQml("TextField", { title: qsTr("Szakasz neve"), value: self.name })

			d.accepted.connect(function(data) {
				if (data.length)
					mapEditor.chapterModify(self, {name: data})
			})
			d.open()
		}

	}



	Action {
		id: actionChapterMissions

		icon.source: CosStyle.iconBooks
		text: qsTr("Küldetések")

		onTriggered: {
			mapEditor.updateMissionLevelModelChapter(self)

			if (mapEditor.missionLevelModel.count < 1) {
				cosClient.sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", qsTr("Küldetések"), qsTr("Még nincsen egyetlen küldetés sem!"))
				return
			}


			var d = JS.dialogCreateQml("MissionList", {
										   icon: CosStyle.iconLockAdd,
										   title: qsTr("%1 - Küldetések").arg(self.name),
										   selectorSet: true,
										   sourceModel: mapEditor.missionLevelModel
									   })

			d.accepted.connect(function(dlgdata) {
				if (!dlgdata)
					return

				mapEditor.chapterModifyMissionLevels(self, mapEditor.missionLevelModel.getSelected())
			})
			d.open()
		}
	}



	Action {
		id: actionChapterRemove

		icon.source: level > 0 ? CosStyle.iconRemove : "qrc:/internal/icon/delete.svg"
		text: level > 0 ? qsTr("Eltávolítás") : qsTr("Törlés")

		onTriggered: control.chapterRemove()
	}





	function objectiveMoveCopy(chapterId, isCopy, objective) {
		if (chapterId === -1) {
			var d = JS.dialogCreateQml("TextField", {
										   title: isCopy ? qsTr("Másolás új szakaszba") : qsTr("Áthelyezés új szakaszba"),
										   text: qsTr("Az új szakasz neve")
									   })

			d.accepted.connect(function(data) {
				if (data.length) {
					if (self.objectives.selectedCount > 0) {
						mapEditor.objectiveMoveCopyList(self, isCopy, self.objectives.getSelected(), -1, data)
					} else {
						mapEditor.objectiveMoveCopy(self, isCopy, objective, -1, data)
					}

					self.objectives.unselectAll()
				}
			})
			d.open()
		} else {
			if (self.objectives.selectedCount > 0) {
				mapEditor.objectiveMoveCopyList(self, isCopy, self.objectives.getSelected(), chapterId)
			} else {
				mapEditor.objectiveMoveCopy(self, isCopy, objective, chapterId)
			}

			self.objectives.unselectAll()
		}
	}
}

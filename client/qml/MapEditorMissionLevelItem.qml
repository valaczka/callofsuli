import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: root

	stackPopFunction: function() {
		/*if (_campaignList.view.selectEnabled) {
			_campaignList.view.unselectAll()
			return false
		}

		if (swipeView.currentIndex > 0) {
			swipeView.setCurrentIndex(0)
			return false
		}*/

		return true
	}

	title: mission ? mission.name : ""
	subtitle: editor ? editor.displayName : ""

	property MapEditorMissionLevel missionLevel: null
	readonly property MapEditorMission mission: missionLevel ? missionLevel.mission : null
	readonly property MapEditor editor: mission && mission.map ? mission.map.mapEditor : null

	property QListView _inventoryView: null//_inventoryView


	appBar.backButtonVisible: true
	appBar.rightComponent: MapEditorToolbarComponent {
		editor: root.editor
	}



	QScrollable {
		anchors.fill: parent

		QFormColumn {
			id: _form

			Qaterial.IconLabel {
				visible: mission

				width: parent.width

				icon.source: mission ? mission.fullMedalImage : ""
				icon.color: "transparent"
				icon.width: 2.2 * Qaterial.Style.pixelSize
				icon.height: 2.2 * Qaterial.Style.pixelSize
				enabled: mission

				font: Qaterial.Style.textTheme.headline4
				text: mission ? mission.name : ""

				elide: Text.ElideRight

			}

			Qaterial.LabelSubtitle1 {
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Level %1").arg(missionLevel ? missionLevel.level : 0)
				bottomPadding: 10
			}

			Row {
				anchors.horizontalCenter: parent.horizontalCenter
				spacing: 10

				Repeater {
					model: ListModel {
						ListElement {
							mode: GameMap.Action
							title: qsTr("Akciójáték")
						}
						ListElement {
							mode: GameMap.Lite
							title: qsTr("Feladatmegoldás")
						}
						ListElement {
							mode: GameMap.Test
							title: qsTr("Teszt")
						}
					}

					delegate: Qaterial.OutlineButton
					{
						text: title
						visible: mission && (mission.modes & mode)
						icon.source: Qaterial.Icons.play
						highlighted: false
						foregroundColor: Qaterial.Style.iconColor()
						onClicked: editor.missionLevelPlay(missionLevel, mode)
					}

				}

			}

			QButton {
				anchors.horizontalCenter: parent.horizontalCenter
				visible: editor && missionLevel && missionLevel.canDelete()
				text: qsTr("Törlés")
				icon.source: Qaterial.Icons.trashCan
				bgColor: Qaterial.Colors.red400
				textColor: Qaterial.Colors.white

				topPadding: 12
				bottomPadding: 12
				leftPadding: 6
				rightPadding: 6

				onClicked: editor.missionLevelRemove(missionLevel)
			}


			/*
			Q_PROPERTY(QString terrain READ terrain WRITE setTerrain NOTIFY terrainChanged)
			Q_PROPERTY(QString image READ image WRITE setImage NOTIFY imageChanged)
*/

			Row {
				anchors.left: parent.left

				Qaterial.LabelBody2 {
					text: qsTr("Időtartam")
					anchors.verticalCenter: parent.verticalCenter
				}

				SpinBox {
					anchors.verticalCenter: parent.verticalCenter
					from: 30
					to: 600
					stepSize: 30

					font: Qaterial.Style.textTheme.body1

					value: missionLevel ? missionLevel.duration : from

					textFromValue: function(value, locale) { return Client.Utils.formatMSecs(value*1000) }

					onValueModified: editor.missionLevelModify(missionLevel, function() {
						missionLevel.duration = value
					})
				}
			}

			Row {
				anchors.left: parent.left

				visible: mission && (mission.modes & (GameMap.Action|GameMap.Lite))

				Qaterial.LabelBody2 {
					text: qsTr("Kezdő HP")
					anchors.verticalCenter: parent.verticalCenter
				}

				SpinBox {
					anchors.verticalCenter: parent.verticalCenter
					from: 1
					to: 10
					stepSize: 1

					font: Qaterial.Style.textTheme.body1

					value: missionLevel ? missionLevel.startHP : from

					onValueModified: editor.missionLevelModify(missionLevel, function() {
						missionLevel.startHP = value
					})
				}
			}




			Row {
				anchors.left: parent.left

				visible: mission && (mission.modes & GameMap.Action)

				Qaterial.LabelBody2 {
					text: qsTr("Kérdések aránya")
					anchors.verticalCenter: parent.verticalCenter
				}

				SpinBox {
					anchors.verticalCenter: parent.verticalCenter
					from: 10
					to: 100
					stepSize: 5
					font: Qaterial.Style.textTheme.body1

					value: missionLevel ? missionLevel.questions*100 : from

					textFromValue: function(value, locale) { return value+"%" }

					onValueModified: editor.missionLevelModify(missionLevel, function() {
						missionLevel.questions = value/100.0
					})
				}
			}



			Row {
				anchors.left: parent.left

				visible: mission && (mission.modes & GameMap.Test)

				Qaterial.LabelBody2 {
					text: qsTr("Sikeres teljesítés")
					anchors.verticalCenter: parent.verticalCenter
				}

				SpinBox {
					anchors.verticalCenter: parent.verticalCenter
					from: 10
					to: 100
					stepSize: 5

					font: Qaterial.Style.textTheme.body1

					value: missionLevel ? missionLevel.passed*100 : from

					textFromValue: function(value, locale) { return value+"%" }

					onValueModified: editor.missionLevelModify(missionLevel, function() {
						missionLevel.passed = value/100.0
					})
				}
			}


			/// TERRAIN

			QFormSwitchButton
			{
				visible: mission && (mission.modes & GameMap.Action)
				text: qsTr("Sudden death mód engedélyezve")
				checked: missionLevel && missionLevel.canDeathmatch
				onToggled: editor.missionLevelModify(missionLevel, function() {
					missionLevel.canDeathmatch = checked
				})
			}




			Item {
				width: parent.width
				height: 20
			}


			Qaterial.Expandable {
				id: _expChapters
				width: parent.width

				expanded: true

				header: QExpandableHeader {
					text: qsTr("Feladatcsoportok")
					icon: Qaterial.Icons.molecule
					expandable: _expChapters
				}

				delegate: QIndentedItem {
					width: _form.width
					Column {
						width: parent.width

						Repeater {
							model: missionLevel ? missionLevel.chapterList : null

							delegate: MapEditorChapterItem {
								width: parent.width
								chapter: modelData

								onRemoveActionRequest: {
									if (editor)
										editor.missionLevelChapterRemove(missionLevel, [chapter])
								}
							}
						}

						QColoredItemDelegate {
							width: parent.width
							icon.source: Qaterial.Icons.plus
							text: qsTr("Új feladatcsoport létrehozása")
							color: Qaterial.Colors.green400

							onClicked: {
								Qaterial.DialogManager.showTextFieldDialog({
																			   textTitle: qsTr("Feladatcsoport neve"),
																			   title: qsTr("Új feladatcsoport létrehozása"),
																			   standardButtons: Dialog.Cancel | Dialog.Ok,
																			   onAccepted: function(_text, _noerror) {
																				   if (_noerror && _text.length) {
																					   editor.chapterAdd(_text, missionLevel)
																				   }
																			   }
																		   })
							}

						}

						QColoredItemDelegate {
							width: parent.width
							icon.source: Qaterial.Icons.accessPointPlus
							text: qsTr("Létező feladatcsoport hozzáadása")
							color: Qaterial.Colors.purple400

							onClicked: {
								_sortedChapterModel.reload()

								Qaterial.DialogManager.openCheckListView(
											{
												onAccepted: function(indexList)
												{
													if (indexList.length === 0)
														return

													var l = []

													for (let i=0; i<indexList.length; ++i) {
														l.push(_sortedChapterModel.get(indexList[i]).chapter)
													}

													editor.missionLevelChapterAdd(missionLevel, l)
												},
												title: qsTr("Feladatcsoport hozzáadása"),
												standardButtons: Dialog.Cancel | Dialog.Ok,
												model: _sortedChapterModel
											})
							}
						}
					}
				}
			}




			Item {
				width: parent.width
				height: 20
				visible: _expInventory.visible
			}

			Qaterial.Expandable {
				id: _expInventory
				width: parent.width

				visible: mission && (mission.modes & GameMap.Action)

				header: QExpandableHeader {
					text: qsTr("Felszerelés")
					icon: Qaterial.Icons.molecule
					expandable: _expInventory

					rightSourceComponent: Qaterial.RoundButton {
						icon.source: Qaterial.Icons.dotsVertical
						icon.color: Qaterial.Style.iconColor()
						onClicked: root._inventoryView ? contextMenu.popup() : contextMenuSimple.popup()

						Qaterial.Menu {
							id: contextMenu
							QMenuItem { action: root._inventoryView ? root._inventoryView.actionSelectAll : null}
							QMenuItem { action: root._inventoryView ? root._inventoryView.actionSelectNone : null }
							Qaterial.MenuSeparator {}
							QMenuItem { action: actionInventoryAdd }
							QMenuItem {
								text: qsTr("Törlés")
								icon.source: Qaterial.Icons.trashCan
								enabled: root._inventoryView
								onClicked: {
									if (!editor)
										return

									let l = root._inventoryView.getSelected()

									if (l.length)
										editor.missionLevelInventoryRemove(missionLevel, l)

									root._inventoryView.unselectAll()
								}
							}
						}

						Qaterial.Menu {
							id: contextMenuSimple
							QMenuItem { action: actionInventoryAdd }
						}
					}
				}

				delegate: QIndentedItem {
					width: _form.width
					QListView {
						id: _inventoryView

						width: parent.width

						height: contentHeight
						boundsBehavior: Flickable.StopAtBounds

						autoSelectChange: true

						model: SortFilterProxyModel {
							sourceModel: missionLevel ? missionLevel.inventoryList : null

							sorters: RoleSorter {
								roleName: "inventoryid"
							}
						}

						delegate: MapEditorInventoryItem {
							inventory: model.qtObject
							width: ListView.view.width
							onMenuRequest: _inventoryView.menuOpenFromDelegate(button)
						}

						footer: Qaterial.ItemDelegate {
							width: ListView.view.width
							textColor: Qaterial.Colors.blue700
							iconColor: textColor
							action: actionInventoryAdd
						}

						Qaterial.Menu {
							id: _inventoryContextMenu
							QMenuItem {
								text: qsTr("Törlés")
								icon.source: Qaterial.Icons.trashCan
								onClicked: if (editor) editor.missionLevelInventoryRemove(missionLevel, [inventory])
							}

							Qaterial.Menu {
								title: qsTr("Elhelyezés")

								Repeater {
									model: 5

									QMenuItem {
										text: index > 0 ? qsTr("%1. csatatéren").arg(index) : qsTr("Bárhol")
										onClicked: editor.missionLevelInventoryModify(missionLevel, inventory, function() {
											inventory.block = (index > 0 ? index : -1)
										})
									}
								}
							}
						}

						onRightClickOrPressAndHold: {
							if (index != -1)
								currentIndex = index

							_inventoryContextMenu.popup(mouseX, mouseY)
						}

						function menuOpenFromDelegate(_item) {
							var p = mapFromItem(_item, _item.x, _item.y+_item.height)

							_inventoryContextMenu.popup(p.x, p.y)
						}

						Component.onCompleted: root._inventoryView = _inventoryView
						Component.onDestruction: root._inventoryView = null
					}

				}

				Action {
					id: actionInventoryAdd
					icon.source: Qaterial.Icons.inboxArrowUp
					text: qsTr("Új felszerelés")

					onTriggered: {
						_inventoryModel.clear()

						if (!editor)
							return

						let list = editor.pickableListModel()

						for (let i=0; i<list.length; ++i) {
							let o = list[i]
							o.iconColor = "transparent"
							_inventoryModel.append(o)
						}

						Qaterial.DialogManager.openListView(
									{
										onAccepted: function(index)
										{
											if (index < 0)
												return

											let ml = _inventoryModel.get(index)
											if (ml)
												editor.missionLevelInventoryAdd(missionLevel, ml.id)

										},
										title: qsTr("Felszerelés hozzáadása"),
										model: _inventoryModel
									})
					}

				}
			}



		}

	}


	ListModel {
		id: _inventoryModel
	}



	ListModel {
		id: _chapterModel
	}

	SortFilterProxyModel {
		id: _sortedChapterModel

		sourceModel: _chapterModel

		sorters: StringSorter {
			roleName: "text"
		}

		function reload() {
			_chapterModel.clear()

			if (!missionLevel)
				return

			var l = missionLevel.unlinkedChapterList()

			for (let i=0; i<l.length; ++i) {
				_chapterModel.append({
										 text: l[i].name,
										 chapter: l[i]
									 })
			}
		}
	}


	onMissionChanged: if (!mission) Client.stackPop(root)
	onMissionLevelChanged: if (!missionLevel) Client.stackPop(root)


}

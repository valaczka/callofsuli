import QtQuick
import QtQuick.Controls
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
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

			spacing: 8

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
						ListElement {
							mode: GameMap.Practice
							title: qsTr("Gyakorlás")
						}

						// Exam: missing
					}

					delegate: Qaterial.OutlineButton
					{
						text: title
						visible: mission && (mission.modes & mode)
						icon.source: Qaterial.Icons.play
						highlighted: false
						foregroundColor: Qaterial.Colors.amber200
						onClicked: editor.missionLevelPlay(missionLevel, mode)
					}

				}

			}

			QButton {
				anchors.horizontalCenter: parent.horizontalCenter
				visible: editor && missionLevel && missionLevel.canDelete()
				text: qsTr("Törlés")
				icon.source: Qaterial.Icons.delete_
				bgColor: Qaterial.Colors.red400
				textColor: Qaterial.Colors.white

				topPadding: 12
				bottomPadding: 12
				leftPadding: 6
				rightPadding: 6

				onClicked: editor.missionLevelRemove(missionLevel)
			}


			QButton {
				anchors.left: parent.left

				enabled: editor && missionLevel

				visible: mission && (mission.modes & (GameMap.Exam))

				text: qsTr("LaTeX fájl készítése")

				icon.source: Qaterial.Icons.signText

				onClicked: {
					if (Qt.platform.os == "wasm")
						editor.exportData(MapEditor.ExportExam, "", {
											  missionLevel: missionLevel
										  })
					else
						Qaterial.DialogManager.openFromComponent(_cmpFileExportLatex)
				}
			}


			Row {
				anchors.left: parent.left

				visible: mission && (mission.modes & (GameMap.Action|GameMap.Lite|GameMap.Test))

				Qaterial.ColorIcon {
					color: Qaterial.Style.colorTheme.primaryText
					source: Qaterial.Icons.timer
					iconSize: Qaterial.Style.textField.iconSize
					width: Qaterial.Style.textField.iconWidth
					height: Qaterial.Style.textField.iconWidth
					anchors.verticalCenter: parent.verticalCenter
				}

				Qaterial.LabelBody2 {
					text: qsTr("Időtartam")
					anchors.verticalCenter: parent.verticalCenter
					rightPadding: 5 * Qaterial.Style.pixelSizeRatio
				}

				QSpinBox {
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

				Qaterial.ColorIcon {
					color: Qaterial.Style.colorTheme.primaryText
					source: Qaterial.Icons.heart
					iconSize: Qaterial.Style.textField.iconSize
					width: Qaterial.Style.textField.iconWidth
					height: Qaterial.Style.textField.iconWidth
					anchors.verticalCenter: parent.verticalCenter
				}

				Qaterial.LabelBody2 {
					text: qsTr("Kezdő HP")
					anchors.verticalCenter: parent.verticalCenter
					rightPadding: 5 * Qaterial.Style.pixelSizeRatio
				}

				QSpinBox {
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

				Qaterial.ColorIcon {
					color: Qaterial.Style.colorTheme.primaryText
					source: Qaterial.Icons.messageQuestion
					iconSize: Qaterial.Style.textField.iconSize
					width: Qaterial.Style.textField.iconWidth
					height: Qaterial.Style.textField.iconWidth
					anchors.verticalCenter: parent.verticalCenter
				}

				Qaterial.LabelBody2 {
					text: qsTr("Kérdések aránya")
					anchors.verticalCenter: parent.verticalCenter
					rightPadding: 5 * Qaterial.Style.pixelSizeRatio
				}

				QSpinBox {
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

				Qaterial.ColorIcon {
					color: Qaterial.Style.colorTheme.primaryText
					source: Qaterial.Icons.chartBar
					iconSize: Qaterial.Style.textField.iconSize
					width: Qaterial.Style.textField.iconWidth
					height: Qaterial.Style.textField.iconWidth
					anchors.verticalCenter: parent.verticalCenter
				}

				Qaterial.LabelBody2 {
					text: qsTr("Sikeres teljesítés")
					anchors.verticalCenter: parent.verticalCenter
					rightPadding: 5 * Qaterial.Style.pixelSizeRatio
				}

				QSpinBox {
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


			Row {
				anchors.left: parent.left

				visible: mission && (mission.modes & GameMap.Action)

				Qaterial.ColorIcon {
					color: Qaterial.Style.colorTheme.primaryText
					source: Qaterial.Icons.swordCross
					iconSize: Qaterial.Style.textField.iconSize
					width: Qaterial.Style.textField.iconWidth
					height: Qaterial.Style.textField.iconWidth
					anchors.verticalCenter: parent.verticalCenter
				}

				Qaterial.LabelBody2 {
					text: qsTr("Harcmező:")
					anchors.verticalCenter: parent.verticalCenter
					rightPadding: 10 * Qaterial.Style.pixelSizeRatio
				}

				MouseArea {
					id: _area
					width: _terrainRow.implicitWidth
					height: _terrainRow.implicitHeight

					hoverEnabled: true

					acceptedButtons: Qt.LeftButton

					onClicked: _terrainAddButton.clicked()

					Row {
						id: _terrainRow

						spacing: 15

						Qaterial.Icon
						{
							icon: missionLevel ? (missionLevel.terrainData.name !== "" ? missionLevel.terrainData.thumbnail : Qaterial.Icons.alert): ""
							color: missionLevel && missionLevel.terrainData.name !== "" ? "transparent" : Qaterial.Colors.red500
							width: missionLevel && missionLevel.terrainData.name !== "" ? Qaterial.Style.pixelSize*4.5 : Qaterial.Style.mediumIcon
							height: missionLevel && missionLevel.terrainData.name !== "" ? Qaterial.Style.pixelSize*3 : Qaterial.Style.mediumIcon
						}

						Qaterial.LabelWithCaption {
							anchors.verticalCenter: parent.verticalCenter
							text: missionLevel ? missionLevel.terrainData.displayName : ""
							caption: missionLevel && missionLevel.terrainData.level > 0 ? qsTr("level %1").arg(missionLevel.terrainData.level) : ""
							textColor: Qaterial.Style.foregroundColor
							captionColor: textColor
						}

						Qaterial.RoundButton {
							id: _terrainAddButton
							anchors.verticalCenter: parent.verticalCenter
							icon.source: Qaterial.Icons.pencil
							enabled: missionLevel
							onClicked: {
								let idx = -1

								for (let i=0; i<_sortedTerrainModel.count; ++i) {
									if (_sortedTerrainModel.get(i).fieldName === missionLevel.terrain)
										idx = i
								}

								Qaterial.DialogManager.openListView(
											{
												onAccepted: function(index)
												{
													if (index < 0)
														return

													editor.missionLevelModify(missionLevel, function() {
														missionLevel.terrain = _sortedTerrainModel.get(index).fieldName
													})

												},
												title: qsTr("Harcmező kiválasztása"),
												model: _sortedTerrainModel,
												currentIndex: idx,
												delegate: _terrainDelegate
											})
							}
						}
					}

					Qaterial.ListDelegateBackground
					{
						anchors.fill: parent
						type: Qaterial.Style.DelegateType.Icon
						lines: 1
						pressed: _area.pressed
						rippleActive: _area.containsMouse
						rippleAnchor: _area
					}
				}
			}

			QFormSwitchButton
			{
				visible: mission && (mission.modes & GameMap.Action)
				text: qsTr("Sudden death mód engedélyezve")
				checked: missionLevel && missionLevel.canDeathmatch
				onToggled: editor.missionLevelModify(missionLevel, function() {
					missionLevel.canDeathmatch = checked
				})
			}



			Row {
				anchors.left: parent.left

				visible: mission && (mission.modes & GameMap.Action)

				Qaterial.ColorIcon {
					color: Qaterial.Style.colorTheme.primaryText
					source: Qaterial.Icons.image
					iconSize: Qaterial.Style.textField.iconSize
					width: Qaterial.Style.textField.iconWidth
					height: Qaterial.Style.textField.iconWidth
					anchors.verticalCenter: parent.verticalCenter
				}

				Qaterial.LabelBody2 {
					text: qsTr("Egyéni háttérkép")
					anchors.verticalCenter: parent.verticalCenter
					rightPadding: 5 * Qaterial.Style.pixelSizeRatio
				}

				MapEditorFormImage {
					editor: root.editor
					image: missionLevel ? missionLevel.editorImage : null

					anchors.verticalCenter: parent.verticalCenter

					onModified: editor.missionLevelModify(missionLevel, function() {
						missionLevel.image = id
					})

				}
			}




			Qaterial.Expandable {
				id: _expChapters
				width: parent.width

				expanded: true

				header: QExpandableHeader {
					text: qsTr("Feladatcsoportok")
					icon: Qaterial.Icons.folderMultipleOutline
					expandable: _expChapters
					topPadding: 20
				}

				delegate: Column {
					width: _form.width

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
						icon.source: Qaterial.Icons.folderPlus
						text: qsTr("Új feladatcsoport létrehozása")
						color: Qaterial.Colors.green400

						onClicked: {
							Qaterial.DialogManager.showTextFieldDialog({
																		   textTitle: qsTr("Feladatcsoport neve"),
																		   title: qsTr("Új feladatcsoport létrehozása"),
																		   standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
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
						icon.source: Qaterial.Icons.folderMultiplePlusOutline
						text: qsTr("Létező feladatcsoport hozzáadása")
						color: Qaterial.Colors.purple100

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
											standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
											model: _sortedChapterModel
										})
						}
					}
				}
			}



			Qaterial.Expandable {
				id: _expInventory
				width: parent.width

				visible: mission && (mission.modes & GameMap.Action)

				header: QExpandableHeader {
					text: qsTr("Felszerelés")
					icon: Qaterial.Icons.bagPersonal
					expandable: _expInventory
					topPadding: 20

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
								icon.source: Qaterial.Icons.delete_
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

				delegate: QListView {
					id: _inventoryView

					width: _form.width

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
							icon.source: Qaterial.Icons.delete_
							onClicked: {
								if (!editor)
									return

								let l = _inventoryView.getSelected()

								if (l.length)
									editor.missionLevelInventoryRemove(missionLevel, l)

								_inventoryView.unselectAll()
							}
						}

						Qaterial.Menu {
							title: qsTr("Elhelyezés")

							Repeater {
								model: 5

								QMenuItem {
									text: index > 0 ? qsTr("%1. csatatéren").arg(index) : qsTr("Bárhol")
									onClicked: {
										if (!editor || _inventoryView.currentIndex == -1)
											return

										let inventory = _inventoryView.modelGet(_inventoryView.currentIndex)

										editor.missionLevelInventoryModify(missionLevel, inventory, function() {
											inventory.block = (index > 0 ? index : -1)
										})
									}
								}
							}
						}
					}

					onRightClickOrPressAndHold: (index, mouseX, mouseY) => {
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
				icon.source: Qaterial.Icons.bagPersonalPlus
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



	ListModel {
		id: _terrainModel
	}

	onEditorChanged: {
		if (!editor)
			return

		var l = editor.terrainListModel()
		_terrainModel.clear()

		for (let i=0; i<l.length; ++i)
			_terrainModel.append(l[i])
	}


	SortFilterProxyModel {
		id: _sortedTerrainModel

		property bool onlyFirst: false

		sourceModel: _terrainModel

		filters: [
			ValueFilter {
				enabled: _sortedTerrainModel.onlyFirst
				roleName: "level"
				value: 1
			}
		]

		sorters: [
			StringSorter {
				roleName: "displayName"
				priority: 2
			},
			RoleSorter {
				roleName: "level"
				priority: 1
			}

		]
	}


	Component {
		id: _cmpFileExportLatex

		QFileDialog {
			title: qsTr("Dolgozat LaTeX fájl készítése")
			filters: [ "*.tex" ]
			isSave: true
			suffix: ".tex"
			onFileSelected: file => {
				if (Client.Utils.fileExists(file))
					overrideQuestion(file, false, MapEditor.ExportExam)
				else
					editor.exportData(MapEditor.ExportExam, file, {
										  missionLevel: missionLevel
									  })
				Client.Utils.settingsSet("folder/mapEditor", modelFolder.toString())
			}

			folder: editor.currentFolder()
		}
	}

	function overrideQuestion(file, isNew, type) {
		JS.questionDialog({
							  onAccepted: function()
							  {
								  editor.exportData(type, file, {
														missionLevel: missionLevel
													})
							  },
							  text: qsTr("A fájl létezik. Felülírjuk?\n%1").arg(file),
							  title: qsTr("Mentés másként"),
							  iconSource: Qaterial.Icons.fileAlert
						  })
	}

	Component {
		id: _terrainDelegate

		Qaterial.LoaderItemDelegate {
			highlighted: ListView.isCurrentItem
			text: model.displayName ? model.displayName: ""
			secondaryText: model.level > 0 ? qsTr("level %1").arg(model.level) : ""

			leftSourceComponent: Image
			{
				fillMode: Image.PreserveAspectFit
				source: model.icon
				sourceSize: Qt.size(width, height)
			}

			width: ListView.view.width
			onClicked: ListView.view.select(index)
		}
	}

	onMissionChanged: if (!mission) Client.stackPop(root)
	onMissionLevelChanged: if (!missionLevel) Client.stackPop(root)


}

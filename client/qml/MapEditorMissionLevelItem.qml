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

	readonly property int missionLevelModes: missionLevel
											 ? (missionLevel.modes !== GameMap.Invalid ? missionLevel.modes :
																						 mission ? mission.modes : GameMap.Invalid)
											 : GameMap.Invalid

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
							mode: GameMap.Rpg
							title: qsTr("RPG")
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
					}

					delegate: Qaterial.OutlineButton
					{
						text: title
						visible: missionLevelModes & mode
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




			/*Item {
				width: parent.width
				height: 20
			}*/


			Qaterial.Expandable {
				id: _expModes
				width: parent.width

				expanded: true

				header: QExpandableHeader {
					width: parent.width
					text: qsTr("Lehetséges játékmódok")
					icon: Qaterial.Icons.googleController
					expandable: _expModes
				}

				delegate: QIndentedItem {
					id: _itemModes

					width: _expModes.parent.width

					Column {
						width: parent.width

						QFormCheckButton
						{
							id: _isRpg
							text: qsTr("Akciójáték (RPG)")
							checked: missionLevel && (missionLevel.modes & GameMap.Rpg)
							onToggled: _itemModes.updateCheckButtons()
						}

						QFormCheckButton
						{
							id: _isLite
							text: qsTr("Feladatmegoldás")
							checked: missionLevel && (missionLevel.modes & GameMap.Lite)
							onToggled: _itemModes.updateCheckButtons()
						}

						QFormCheckButton
						{
							id: _isTest
							text: qsTr("Teszt")
							checked: missionLevel && (missionLevel.modes & GameMap.Test)
							onToggled: _itemModes.updateCheckButtons()
						}

						QFormCheckButton
						{
							id: _isPractice
							text: qsTr("Gyakorlás")
							checked: missionLevel && (missionLevel.modes & GameMap.Practice)
							onToggled: _itemModes.updateCheckButtons()
						}

						/*QFormCheckButton			// DOLGOZAT NEM LEHET
						{
							id: _isExam
							text: qsTr("Dolgozat")
							checked: mission && (missionLevelModes & GameMap.Exam)
							onToggled: _form.updateCheckButtons()
						}*/

					}

					function updateCheckButtons() {
						var c = 0

						if (_isRpg.checked)
							c |= GameMap.Rpg

						if (_isLite.checked)
							c |= GameMap.Lite

						if (_isTest.checked)
							c |= GameMap.Test

						if (_isPractice.checked)
							c |= GameMap.Practice

						editor.missionLevelModify(missionLevel, function() {
							missionLevel.modes = c
						})
					}
				}



			}



			Row {
				anchors.left: parent.left

				visible: missionLevelModes & (GameMap.Test|GameMap.Lite)

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
					from: 0
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

				visible: missionLevelModes & (GameMap.Lite|GameMap.Rpg)

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

				visible: missionLevelModes & GameMap.Test

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
							isExam: missionLevelModes & GameMap.Exam

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

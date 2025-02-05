import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

Item {
	id: root

	property MapEditor editor: null

	SortFilterProxyModel {
		id: _model

		sourceModel: editor && editor.map ? editor.map.missionList : null

		sorters: StringSorter {
			roleName: "name"
		}
	}

	QScrollable {
		anchors.fill: parent

		Grid {
			id: _grid
			spacing: 10
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			columns: (root.width >= 580 * Qaterial.Style.pixelSizeRatio) ? 2 : 1

			readonly property double _itemWidth: (width - (columns-1)*columnSpacing) / columns
			readonly property double _itemHeight: 200 * Qaterial.Style.pixelSizeRatio

			Repeater {
				model: _model

				delegate: QCard {
					property MapEditorMission mission: model.qtObject

					width: _grid._itemWidth
					height: _grid._itemHeight

					elevationOnHovered: true

					rippleActive: _area.containsMouse
					ripplePressed: _area.pressed

					onMissionChanged: _tagList.reloadModel()

					contentItem: MouseArea {
						id: _area
						acceptedButtons: Qt.LeftButton
						hoverEnabled: true

						onClicked: loadMission(mission)

						ColumnLayout {
							anchors.fill: parent
							clip: true

							QTagList {
								id: _tagList
								Layout.topMargin: Qaterial.Style.card.horizontalPadding
								Layout.leftMargin: Qaterial.Style.card.horizontalPadding
								Layout.rightMargin: Qaterial.Style.card.horizontalPadding
								Layout.bottomMargin: 4
								Layout.fillWidth: true

								Component.onCompleted: reloadModel()

								Connections {
									target: mission

									function onModesChanged() {
										_tagList.reloadModel()
									}
								}

								function reloadModel() {
									var list = []

									if (mission.modes & GameMap.Rpg)
										list.push({
													  text: qsTr("RPG"),
													  color: Qaterial.Colors.blue800
												  })

									if (mission.modes & GameMap.Lite)
										list.push({
													  text: qsTr("Feladatmegoldás"),
													  color: Qaterial.Colors.cyan800
												  })

									if (mission.modes & GameMap.Test)
										list.push({
													  text: qsTr("Teszt"),
													  color: Qaterial.Colors.green800
												  })

									/*if (mission.modes & GameMap.MultiPlayer)
										list.push({
													  text: qsTr("Multiplayer"),
													  color: Qaterial.Colors.purple500
												  })*/

									if (mission.modes & GameMap.Conquest)
										list.push({
													  text: qsTr("Multiplayer"),
													  color: Qaterial.Colors.brown400
												  })

									if (mission.modes & GameMap.Exam)
										list.push({
													  text: qsTr("Dolgozat"),
													  color: Qaterial.Colors.red600
												  })

									if (mission.modes & GameMap.Practice)
										list.push({
													  text: qsTr("Gyakorlás"),
													  color: Qaterial.Colors.purple500
												  })

									model = list
								}
							}

							RowLayout {
								Layout.leftMargin: Qaterial.Style.card.horizontalPadding
								Layout.rightMargin: Qaterial.Style.card.horizontalPadding
								Layout.fillWidth: true
								Layout.fillHeight: true

								ColumnLayout {
									Layout.fillWidth: true
									Layout.fillHeight: true

									Qaterial.IconLabel {
										font: Qaterial.Style.textTheme.headline5
										wrapMode: Text.Wrap
										maximumLineCount: 2
										horizontalAlignment: Qt.AlignLeft
										text: mission.name
										icon.source: mission.fullMedalImage
										icon.color: "transparent"
										icon.width: 1.5 * font.pixelSize
										icon.height: 1.5 * font.pixelSize
										Layout.fillWidth: true
									}

									Qaterial.LabelBody2
									{
										text: mission.description
										Layout.topMargin: 2
										Layout.bottomMargin: 2
										Layout.fillWidth: true
										Layout.fillHeight: true
										color: Qaterial.Style.secondaryTextColor()
										wrapMode: Text.Wrap
										maximumLineCount: 3
										elide: Text.ElideRight
									}
								}

								Qaterial.Icon {
									icon: Qaterial.Icons.lock
									size: Qaterial.Style.largeIcon
									color: Qaterial.Colors.red400
									visible: mission.lockList.length
									Layout.leftMargin: 5
								}
							}



							RowLayout
							{
								id: _levelRow

								Layout.leftMargin: Qaterial.Style.card.verticalPadding
								Layout.rightMargin: Qaterial.Style.card.verticalPadding
								Layout.fillWidth: true

								readonly property int maxLevelButton: 4

								Repeater {
									model: SortFilterProxyModel {
										id: _sortedList
										sourceModel: mission.levelList
										sorters: [
											RoleSorter {
												roleName: "level"
												sortOrder: Qt.AscendingOrder
											}
										]
									}

									delegate: Qaterial.FlatButton
									{
										property MapEditorMissionLevel missionLevel: model.qtObject
										text: qsTr("Level %1").arg(missionLevel.level)
										onClicked: loadMissionLevel(missionLevel)

										visible: index < _levelRow.maxLevelButton
									}
								}

								Qaterial.FlatButton
								{
									text: qsTr("...")

									visible: _sortedList.count > _levelRow.maxLevelButton

									onClicked: loadMission(mission)
								}

								Qaterial.SquareButton
								{
									foregroundColor: Qaterial.Colors.green400
									///visible: mission.levelList.length < 3
									icon.source: Qaterial.Icons.plus
									onClicked: {
										var m = editor.missionLevelAdd(mission)
										loadMissionLevel(m)
									}
								}

								Item {
									Layout.fillWidth: true
								}

								/*Qaterial.SquareButton
								{
									foregroundColor: Qaterial.Style.iconColor()
									icon.source: Qaterial.Icons.pencil
									onClicked: loadMission(mission)
								}*/

								Qaterial.SquareButton
								{
									foregroundColor: Qaterial.Colors.red500
									icon.source: Qaterial.Icons.delete_
									onClicked: editor.missionRemove(mission)
								}
							}
						}
					}
				}
			}
		}


	}

	Qaterial.Banner
	{
		anchors.top: parent.top
		width: parent.width
		drawSeparator: true
		text: qsTr("Még egyetlen küldetést sem tartalmaz ez a pálya. Hozz létre egyet!")
		iconSource: Qaterial.Icons.trophy
		fillIcon: false
		outlinedIcon: true
		highlightedIcon: true

		action1: qsTr("Létrehozás")

		onAction1Clicked: _actionAdd.trigger()

		enabled: editor && editor.map && editor.map.missionList.length === 0
		visible: editor && editor.map && editor.map.missionList.length === 0
	}

	QFabButton {
		action: _actionAdd
	}

	Action {
		id: _actionAdd
		icon.source: Qaterial.Icons.plus
		onTriggered: {
			Qaterial.DialogManager.showTextFieldDialog({
														   textTitle: qsTr("Küldetés neve"),
														   title: qsTr("Új küldetés létrehozása"),
														   standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
														   onAccepted: function(_text, _noerror) {
															   if (_noerror && _text.length) {
																   var m = editor.missionAdd(_text)
																   loadMission(m)
															   }
														   }
													   })
		}
	}

	function loadMission(m) {
		var o = Client.stackPushPage("MapEditorMissionItem.qml", {
								 mission: m
							 })

		if (o)
			o.missionLevelLoadRequest.connect((ml) => loadMissionLevel(ml))
	}

	function loadMissionLevel(ml) {
		Client.stackPushPage("MapEditorMissionLevelItem.qml", {
								 missionLevel: ml
							 })
	}
}

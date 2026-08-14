import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

QItemGradient {
	id: root

	property RpgGame game: null

	title: game ? game.readableRoom : ""

	//subtitle: game ? game.name + qsTr(" – level %1").arg(game.level): ""

	appBar.rightComponent: Row {
		Qaterial.Icon {
			color: Qaterial.Colors.blue500
			icon: Qaterial.Icons.shieldCrown
		}

		Qaterial.LabelHeadline6 {
			text: num

			color: Qaterial.Colors.blue500

			property int num: game ? game.rpgUserData.token : 0

			Behavior on num {
				NumberAnimation {
					duration: 250
					easing.type: Easing.InOutQuad
				}
			}
		}
	}

	Qaterial.BusyIndicator {
		id: _busyIndicator
		anchors.centerIn: parent
		visible: false
	}


	Component {
		id: _playerDelegate

		Qaterial.ItemDelegate {
			width: ListView.view.width

			text: nickname

			secondaryText: username+" "+playerId

			icon.color: onboard ? Qaterial.Colors.green500 : Qaterial.Style.iconColor()
			icon.source: onboard ? Qaterial.Icons.checkCircle : Qaterial.Icons.accountOutline

			highlighted: game && game.engine && game.engine.getPeerId() == playerId
		}
	}


	Item {
		id: _content

		property real horizontalPadding: Qaterial.Style.horizontalPadding
		property real verticalPadding: Qaterial.Style.horizontalPadding

		anchors.leftMargin: Math.max(horizontalPadding, Client.safeMarginLeft)
		anchors.rightMargin: Math.max(horizontalPadding, Client.safeMarginRight)
		anchors.topMargin: Math.max(verticalPadding, Client.safeMarginTop, root.paddingTop)
		anchors.bottomMargin: Math.max(verticalPadding, Client.safeMarginBottom)

		anchors.fill: parent


		Qaterial.Card {
			visible: game && !game.isRoomCompleted

			outlined: true

			width: parent.width > parent.height ? Math.min(parent.width, 1000) : parent.width
			height: parent.width > parent.height ? Math.min(parent.height, 600) : parent.height

			anchors.centerIn: parent

			contentItem: GridLayout {
				id: _grid1
				columnSpacing: 5
				rowSpacing: 5

				flow: width > height ? GridLayout.LeftToRight : GridLayout.TopToBottom


				ColumnLayout {
					Layout.fillWidth: true
					Layout.fillHeight: true

					RpgSelectTitle {
						Layout.fillWidth: true
						Layout.fillHeight: false

						icon.source: Qaterial.Icons.accountMultipleOutline
						text: qsTr("Team A")
					}

					ListView {
						Layout.fillWidth: true
						Layout.fillHeight: true

						snapMode: ListView.SnapToItem

						clip: true
						model: SortFilterProxyModel {
							sourceModel: game ? game.modelPlayer : null

							filters: ValueFilter {
								roleName: "team"
								value: 1
							}
						}

						delegate: _playerDelegate
					}
				}


				Qaterial.VerticalLineSeparator {
					visible: _grid1.flow == GridLayout.LeftToRight
					Layout.fillHeight: true
					Layout.fillWidth: false
				}

				Qaterial.HorizontalLineSeparator {
					visible: _grid1.flow == GridLayout.TopToBottom
					Layout.fillHeight: false
					Layout.fillWidth: true
				}

				Grid {
					Layout.fillWidth: false
					Layout.fillHeight: false
					Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

					horizontalItemAlignment: Grid.AlignHCenter
					verticalItemAlignment: Grid.AlignVCenter

					columns: _grid1.flow == GridLayout.TopToBottom ? 2 : 1

					QButton {
						//anchors.horizontalCenter: parent.horizontalCenter

						icon.source: Qaterial.Icons.refresh
						text: qsTr("Csere")

						display: _grid1.flow == GridLayout.TopToBottom ? AbstractButton.TextBesideIcon : AbstractButton.IconOnly

						onClicked: game.characterSelect({replaceTeam: true})
					}

					QButton {
						//anchors.horizontalCenter: parent.horizontalCenter

						bgColor: Qaterial.Colors.green500

						icon.source: Qaterial.Icons.checkBold
						text: qsTr("Kész")

						display: _grid1.flow == GridLayout.TopToBottom ? AbstractButton.TextBesideIcon : AbstractButton.IconOnly

						enabled: game && game.engine && (!game.engine.isHost || (game.isAllOnboard && _selectTerrain._selected))

						onClicked: {
							game.characterSelect({ onboard: true })

							if (!game.engine.isHost)
								enabled = false
						}
					}
				}

				Qaterial.VerticalLineSeparator {
					visible: _grid1.flow == GridLayout.LeftToRight
					Layout.fillHeight: true
					Layout.fillWidth: false
				}

				Qaterial.HorizontalLineSeparator {
					visible: _grid1.flow == GridLayout.TopToBottom
					Layout.fillHeight: false
					Layout.fillWidth: true
				}


				ColumnLayout {
					Layout.fillWidth: true
					Layout.fillHeight: true

					RpgSelectTitle {
						Layout.fillWidth: true
						Layout.fillHeight: false

						icon.source: Qaterial.Icons.accountMultipleOutline
						text: qsTr("Team B")
					}

					ListView {
						Layout.fillWidth: true
						Layout.fillHeight: true

						snapMode: ListView.SnapToItem

						clip: true
						model: SortFilterProxyModel {
							sourceModel: game ? game.modelPlayer : null

							filters: ValueFilter {
								roleName: "team"
								value: 2
							}
						}


						delegate: _playerDelegate
					}
				}

				Qaterial.VerticalLineSeparator {
					visible: _grid1.flow == GridLayout.LeftToRight
					Layout.fillHeight: true
					Layout.fillWidth: false
				}

				Qaterial.HorizontalLineSeparator {
					visible: _grid1.flow == GridLayout.TopToBottom
					Layout.fillHeight: false
					Layout.fillWidth: true
				}


				ColumnLayout {
					Layout.fillWidth: _grid1.flow == GridLayout.TopToBottom
					Layout.fillHeight: _grid1.flow == GridLayout.LeftToRight
					Layout.preferredWidth: 200
					Layout.preferredHeight: 200

					RpgSelectTitle {
						visible: _grid1.flow == GridLayout.LeftToRight

						Layout.fillWidth: true
						Layout.fillHeight: false

						icon.source: Qaterial.Icons.earth
						text: qsTr("World")
					}

					Item {
						id: _world

						Layout.fillHeight: true
						Layout.fillWidth: true

						RpgSelectCard {
							id: _selectTerrain

							anchors.centerIn: parent

							readonly property RpgWorldLandData _selected: Client.world ? Client.world.selectedLand : null

							readonly property real _size: Math.min(_world.width, _world.height)

							width: _size
							height: _size

							fullBg: true

							text: _selected ? _selected.name :
											  enabled ? qsTr("World...") : ""
							image: _selected ? _selected.backgroundSource : ""

							enabled: game && game.engine && game.engine.isHost

							onClicked: {
								Client.stackPushPage("RpgWorldSelect.qml", {
														 world: Client.world
													 })
							}

							on_SelectedChanged: {
								if (_selected && game && game.engine && game.engine.isHost)
									game.characterSelect({ terrain: _selected.bindedMap() })
							}

						}
					}
				}
			}
		}


		Qaterial.Card {
			visible: game && game.isRoomCompleted

			outlined: true

			width: Math.min(parent.width, _viewCharacters.contentWidth)
			height: Math.min(parent.height, 650)

			anchors.centerIn: parent

			contentItem: GridLayout {
				id: _grid2

				columns: 2//width > height ? 3 : 2
				columnSpacing: 10
				rowSpacing: 10
				//width: parent.width - 2 * Qaterial.Style.card.horizontalPadding
				//height: parent.height - 2 * Qaterial.Style.card.verticalPadding


				RpgSelectTitle {
					Layout.fillHeight: false
					Layout.fillWidth: true
					Layout.columnSpan: 2

					icon.source: Qaterial.Icons.accountMultipleOutline
					text: qsTr("Characters")
				}

				RpgSelectView {
					id: _viewCharacters

					Layout.fillHeight: true
					Layout.fillWidth: true
					Layout.columnSpan: 2


					delegate: RpgSelectCard {
						id: _selectPlayer

						readonly property bool isTarget: character === game.rpgUserData.target

						text: name
						image: game ? game.getCharacterImage(character) : ""
						selected: (_viewCharacters.selected == "" && game.isCharacterSelect && !locked && !disabled) ||
									  _viewCharacters.selected == character

						disabled: model.disabled
						locked: level < 1
						iconLockColor: isTarget ? Qaterial.Colors.cyan800 : Qaterial.Style.disabledTextColor()

						borderVisible: true

						Qaterial.IconLabel {
							id: _labelLevel

							text: level
							icon.source: Qaterial.Icons.power

							anchors.left: parent.left
							anchors.bottom: parent.bottom
							anchors.leftMargin: 10 * Qaterial.Style.pixelSizeRatio
							anchors.bottomMargin: 10 * Qaterial.Style.pixelSizeRatio

							icon.width: 12 * Qaterial.Style.pixelSizeRatio
							icon.height: 12 * Qaterial.Style.pixelSizeRatio
							spacing: 0
							visible: level > 0
						}

						ProgressBar {
							id: _progress
							visible: ((_selectPlayer.selected) && nextPoint > 0 && level > 0) || isTarget
							anchors.verticalCenter: _labelLevel.top
							anchors.verticalCenterOffset: 3
							anchors.left: _labelLevel.visible ? _labelLevel.right : parent.left
							anchors.right: parent.right
							anchors.leftMargin: 10 * Qaterial.Style.pixelSizeRatio
							anchors.rightMargin: 10 * Qaterial.Style.pixelSizeRatio

							from: 0
							to: level == 0 ? unlock : nextPoint
							value: level == 0 ? game.rpgUserData.token : point

							Material.accent: color

							property color color: isTarget ? Qaterial.Style.iconColor() : Qaterial.Style.accentColor

							Behavior on value {
								NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
							}
						}

						Qaterial.LabelCaption {
							visible: ((_selectPlayer.selected) && nextPoint > 0 && level > 0) || isTarget
							text: isTarget ? game.rpgUserData.token : point
							anchors.left: _progress.left
							anchors.top: _progress.bottom
							color: isTarget ? Qaterial.Style.iconColor() : Qaterial.Style.accentColor
						}

						Qaterial.LabelCaption {
							visible: ((_selectPlayer.selected) && nextPoint > 0 && level > 0) || isTarget
							text: isTarget ? unlock : nextPoint
							anchors.right: _progress.right
							anchors.top: _progress.bottom
							color: isTarget ? Qaterial.Style.iconColor() : Qaterial.Style.accentColor
						}

						Qaterial.Icon {
							icon: Qaterial.Icons.checkCircleOutline
							color: Qaterial.Colors.green400
							anchors.centerIn: parent
							visible: picked
						}

						onClicked: {
							if (locked || !game.isCharacterSelect)
								return;

							_viewCharacters.currentIndex = index
							_viewCharacters.selected = character


							game.characterSelect({character: character})
						}
					}

					model: _modelCharacters
				}
			}
		}
	}

	SortFilterProxyModel {
		id: _modelCharacters
		sourceModel: game ? game.modelCharacters : null

		sorters: [
			FilterSorter {
				ValueFilter {
					roleName: "unlock"
					value: 0
				}
				priority: 2
			},

			FilterSorter {
				ValueFilter {
					roleName: "level"
					value: 0
				}
				priority: 1
				sortOrder: Qt.DescendingOrder
			},

			StringSorter {
				roleName: "name"
				priority: 0
				sortOrder: Qt.AscendingOrder
			}
		]
	}


	Connections {
		target: game

		function onRpgUserDataChanged() {
			autoSelect()
		}

		function onIsCharacterSelectChanged() {
			if (game.isCharacterSelect) {
				Client.sound.playSound("qrc:/sound/voiceover/choose_your_character.mp3", Sound.VoiceoverChannel)
			}
		}
	}


	function autoSelect() {
		if (!_selectTerrain._selected && game && game.rpgUserData.lastTerrain != "" && game.engine && game.engine.isHost) {
			Client.world.select(game.rpgUserData.lastTerrain)
		}
	}

	onGameChanged: {
		if (!game)
			return

		if (!Client.server || !Client.server.user)
			return

		game.characterSelect({ nickname: Client.server.user.fullNickName })

		if (Client.world)
			Client.world.resetLands(game.rpgUserData.terrains)

		autoSelect()
	}

	StackView.onActivated: {
	}

	StackView.onDeactivating: {
	}
}

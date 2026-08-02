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
	property Item parentPage: null

	readonly property bool _isEmpty: game && game.isEmpty

	title: _isEmpty ? "EMPTY" :
					  game ? game.readableRoom + " - " + game.terrain : ""

	subtitle: !_isEmpty && game ? game.name + qsTr(" – level %1").arg(game.level): ""

	appBar.rightComponent: Row {
		Qaterial.Icon {
			color: Qaterial.Style.iconColor()
			icon: Qaterial.Icons.powerCycle
		}

		Qaterial.LabelHeadline6 {
			text: num

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
			outlined: true

			width: Math.min(parent.width, _viewCharacters.contentWidth)
			height: Math.min(parent.height, _isEmpty ? 800 : 500)

			anchors.centerIn: parent

			contentItem: GridLayout {
				id: _grid1

				columns: 2//width > height ? 3 : 2
				columnSpacing: 10
				rowSpacing: 10
				width: parent.width - 2 * Qaterial.Style.card.horizontalPadding
				height: parent.height - 2 * Qaterial.Style.card.verticalPadding


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
						selected: _viewCharacters.selected == character

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
							visible: ((_isEmpty || _selectPlayer.selected) && nextPoint > 0 && level > 0) || isTarget
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
							visible: ((_isEmpty || _selectPlayer.selected) && nextPoint > 0 && level > 0) || isTarget
							text: isTarget ? game.rpgUserData.token : point
							anchors.left: _progress.left
							anchors.top: _progress.bottom
							color: isTarget ? Qaterial.Style.iconColor() : Qaterial.Style.accentColor
						}

						Qaterial.LabelCaption {
							visible: ((_isEmpty || _selectPlayer.selected) && nextPoint > 0 && level > 0) || isTarget
							text: isTarget ? unlock : nextPoint
							anchors.right: _progress.right
							anchors.top: _progress.bottom
							color: isTarget ? Qaterial.Style.iconColor() : Qaterial.Style.accentColor
						}

						Qaterial.AppBarButton {
							anchors.right: parent.right
							anchors.top: parent.top
							anchors.rightMargin: 5 * Qaterial.Style.pixelSizeRatio
							anchors.topMargin: Math.max(5 * Qaterial.Style.pixelSizeRatio, _selectPlayer.labelHeight)
							icon.source: Qaterial.Icons.eye
							icon.color: Qaterial.Colors.green500
							visible: level <= 0 && _isEmpty

							ToolTip.text: qsTr("Preview")

							onClicked: {
								game.loadTutorial(character)
							}

						}


						onClicked: {
							if (locked && !_isEmpty)
								return;

							_viewCharacters.currentIndex = index
							_viewCharacters.selected = character

							if (!_isEmpty)
								game.characterSelect({character: character})
						}
					}

					model: _modelCharacters
				}

				Item {
					id: _preview

					visible: _isEmpty

					readonly property var character: _viewCharacters.currentIndex != -1 ?
														 _modelCharacters.get(_viewCharacters.currentIndex) :
														 null

					property int levelCurrent: 1
					property int levelMin: Math.max(1, character ? character.level : 1)
					readonly property int levelMax: game.getCharactersMetric().maxLevel

					onLevelMinChanged: if (levelCurrent < levelMin) levelCurrent = levelMin

					Layout.fillHeight: true
					Layout.fillWidth: true

					implicitHeight: 50
					implicitWidth: 50

					QButton {
						anchors.top: parent.top
						anchors.left: parent.left
						text: "< " + _preview.levelCurrent

						enabled: _preview.levelCurrent > _preview.levelMin

						onClicked: --_preview.levelCurrent
					}

					QButton {
						anchors.top: parent.top
						anchors.right: parent.right
						text: ">"

						enabled: _preview.levelCurrent < _preview.levelMax

						onClicked: ++_preview.levelCurrent
					}

					Qaterial.LabelBody2 {
						anchors.centerIn: parent
						width: Math.min(implicitWidth, parent.width)
						wrapMode: Text.Wrap

						text: JSON.stringify(game.getCharactersMetric()) + "\n"
							  + (_preview.character ? _preview.character.character : "---") + "\n"
							  + (_preview.character ? _preview.character.level : "---") + "\n"
							  + JSON.stringify(_preview.character) + "\n"
							  + JSON.stringify(game.getCharacterMetricAtLevel(_preview.character.character, _preview.levelCurrent))
					}
				}

				Column {
					id: _world

					visible: !_isEmpty

					Layout.fillHeight: true
					Layout.fillWidth: false
					Layout.preferredWidth: _selectTerrain.implicitWidth
					Layout.preferredHeight: _selectTerrain.implicitHeight
					Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

					RpgSelectTitle {
						id: _worldTitle
						anchors.horizontalCenter: parent.horizontalCenter
						icon.source: Qaterial.Icons.earth
						text: qsTr("World")
					}

					RpgSelectCard {
						id: _selectTerrain

						anchors.horizontalCenter: parent.horizontalCenter

						readonly property RpgWorldLandData _selected: Client.world ? Client.world.selectedLand : null

						readonly property real _size: Math.min(parent.width, parent.height-_worldTitle.height)

						width: _size
						height: _size

						fullBg: true

						text: _selected ? _selected.name :
										  enabled ? qsTr("World...") : ""
						image: _selected ? _selected.backgroundSource : ""

						//selected: enabled
						enabled: game /*&& (game.gameMode == RpgGame.SinglePlayer ||
										  _multiplayer.gameMode == ActionRpgGame.MultiPlayerHost*/
						onClicked: {
							Client.stackPushPage("RpgWorldSelect.qml", {
													 world: Client.world
												 })
						}

					}
				}


				Column {
					Layout.fillHeight: false
					Layout.fillWidth: !_isEmpty
					Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

					spacing: 5

					QButton {
						anchors.horizontalCenter: parent.horizontalCenter

						visible: _isEmpty

						icon.source: Qaterial.Icons.check
						text: qsTr("Select")

						enabled: _preview.character && _preview.character.level <= 0

						onClicked: {
							Client.send(HttpConnection.ApiUser, "rpg/target/%1".arg(_preview.character.character))
							.done(root, function(r){
								game.reloadRpgData()
							})
							.fail(root, JS.failMessage(qsTr("Karakter kiválasztása sikertelen")))
						}
					}


					QButton {
						id: _btnBuy

						anchors.horizontalCenter: parent.horizontalCenter

						text: qsTr("Vásárlás")
						icon.source: Qaterial.Icons.cart
						enabled: _preview.character && _preview.character.level == 0 && game.rpgUserData.token >= _preview.character.unlock

						visible: _isEmpty

						onClicked: {
							if (_isEmpty) {
								Client.send(HttpConnection.ApiUser, "rpg/buy/%1".arg(character))
								.done(root, function(r){
									game.reloadRpgData()
								})
								.fail(root, JS.failMessage(qsTr("Karakter kiválasztása sikertelen")))

							} else
								return;

						}
					}


					QButton {
						anchors.horizontalCenter: parent.horizontalCenter

						icon.source: _isEmpty ? Qaterial.Icons.eye : Qaterial.Icons.play
						text: _isEmpty ? qsTr("Tutorial") : qsTr("Play")

						bgColor: Qaterial.Colors.green500

						topPadding: 15
						bottomPadding: 15
						//leftPadding: 10
						//rightPadding: 10

						enabled: _isEmpty || _viewCharacters.selected != ""

						onClicked: _isEmpty ? game.loadTutorial("")
											: game.characterSelect({
																	   character: _viewCharacters.selected,
																	   terrain: "test",
																	   ready: true
																   })
					}
				}

				Row {
					spacing: 5
					Layout.fillHeight: false
					Layout.fillWidth: false
					Layout.columnSpan: 2

					QButton {
						text: "DROP"

						visible: game.rpgUserData.drops.length > 0

						onClicked: Client.stackPushPage("PageRpgDrop.qml", {
															game: root.game
														})
					}

					/*Repeater {
						model: game ? game.rpgUserData.drops : null

						delegate: QButton {
								id: _dropBtn

								text: "DROP "+modelData.id

								onClicked: {
									console.debug("OPEN", modelData.id)

									Client.send(HttpConnection.ApiUser, "rpg/drop/%1".arg(modelData.id))
									.done(root, function(r){
										game.reloadRpgData()
									})
									.fail(root, JS.failMessage(qsTr("Drop open error")))
								}
							}
					}*/

				}

			}


		}
	}

	/*




		QListView {
			id: _view

			width: parent.width
			height: contentHeight

			model: game ? game.modelPlayer : null

			delegate: Qaterial.ItemDelegate {
				width: ListView.view.width

				text: nickname
				secondaryText: username + " id: " + playerId + " - " + character + " team: " + team

				//secondaryText: owner.nickName + (players.length > 1 ? " +" + (players.length-1) : "")

				icon.source: Qaterial.Icons.accountMultiple

				onClicked: {
					//game.connectLobby(model)
				}
			}


			footer: Qaterial.ItemDelegate {
				width: ListView.view.width
				height: visible ? implicitHeight : 0
				//visible: game && game.canAddEngine
				textColor: Qaterial.Colors.green500
				iconColor: textColor
				icon.source: Qaterial.Icons.play
				text: qsTr("PLAY")

				onClicked: game.characterSelect({
													character: "character01a",
													terrain: "test",
													ready: true
												})
			}


		}

*/

	Timer {
		id: _timerOldCurrency
		interval: 3000
		repeat: false
		triggeredOnStart: false

		onTriggered: {
			if (root.parentPage && root.parentPage.StackView.status == StackView.Active)
				Qaterial.DialogManager.showDialog(
							{
								text: qsTr("A megújult akciójátékba áthozzuk az eddigi pénzedet: %1\nVálaszd ki, melyik karakterek között akarod egyenlő mértékben szétosztani").arg(game.rpgUserData.oldCurrency),
								title: qsTr("Konvertálás az új játékra"),
								iconSource: Qaterial.Icons.cash100,
								iconColor: Qaterial.Style.accentColor,
								textColor: Qaterial.Style.accentColor,
								iconFill: false,
								iconSize: Qaterial.Style.roundIcon.size,
								standardButtons: DialogButtonBox.Ok,
								onAccepted: function() { loadCurrencyDialog() }
							})
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
	}


	ListModel {
		id: _currencyModel
	}

	function loadCurrencyDialog() {
		_currencyModel.clear()

		for (let i=0; i<_modelCharacters.count; ++i) {
			let ch = _modelCharacters.get(i)
			if (ch.level > 0)
				_currencyModel.append({
										  character: ch.character,
										  text: ch.name
									  })
		}

		Qaterial.DialogManager.openCheckListView(
					{
						onAccepted: function(indexList)
						{
							if (indexList.length === 0)
								return

							var l = []

							for (let i=0; i<indexList.length; ++i) {
								l.push(_currencyModel.get(indexList[i]).character)
							}

							Client.send(HttpConnection.ApiUser, "rpg/upgrade", {
								list: l
							})
							.done(root, function(r){
								game.reloadRpgData()
								Client.snack(qsTr("Sikeres konvertálás"))
							})
							.fail(root, JS.failMessage(qsTr("Konvertálás sikertelen")))
						},
						title: qsTr("Pénz szétosztása"),
						standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
						model: _currencyModel
					})
	}

	function autoSelect() {
		if (_viewCharacters.selected == "" && game && game.rpgUserData.lastCharacter != "") {
			_viewCharacters.selected = game.rpgUserData.lastCharacter
			game.characterSelect({character: game.rpgUserData.lastCharacter})
		}

		if (!_selectTerrain._selected && game && game.rpgUserData.lastTerrain != "") {
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
		if (!_isEmpty)
			Client.sound.playSound("qrc:/sound/voiceover/choose_your_character.mp3", Sound.VoiceoverChannel)

		if (_isEmpty && game.rpgUserData.oldCurrency > 0)
			_timerOldCurrency.start()
	}

	StackView.onDeactivating: {
	}
}

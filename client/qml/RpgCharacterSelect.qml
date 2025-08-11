import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SortFilterProxyModel
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

QItemGradient {
	id: root

	property ActionRpgGame game: null

	readonly property ActionRpgMultiplayerGame _multiplayer: game && (game instanceof ActionRpgMultiplayerGame) ? game : null
	property bool _isFirst: true

	title: (_multiplayer ? _multiplayer.readableEngineId + " | " : "") +
		   (game ? game.name + qsTr(" – level %1").arg(game.level) : "")

	signal marketRequest()

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

			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			height: Math.min(parent.height, _multiplayer ? 500 : 300)

			anchors.centerIn: parent

			contentItem: Item {
				GridLayout {
					id: _grid1

					columns: width > height ? 3 : 2
					columnSpacing: 10
					rowSpacing: 10
					width: parent.width - 2 * Qaterial.Style.card.horizontalPadding
					height: parent.height - 2 * Qaterial.Style.card.verticalPadding

					anchors.centerIn: parent

					RpgSelectCard {
						id: _selectTerrain
						property RpgUserWallet wallet: null

						Layout.fillHeight: true
						Layout.fillWidth: true

						text: wallet ? wallet.readableName :
									   enabled ? qsTr("Válassz...") : ""
						image: wallet ? wallet.image : ""
						locked: !wallet
						selected: enabled
						enabled: !_multiplayer || _multiplayer.gameMode == ActionRpgGame.MultiPlayerHost
						onClicked: {
							if (!Client.server.user.wallet.world) {
								Client.messageWarning(qsTr("Nem sikerült betölteni a világokat!"))
								return
							}

							/*_modelSelector.marketType = RpgMarket.Map
							_dialogWallet = _selectTerrain.wallet
							_dialogAcceptFunc = function(w) {
								_selectTerrain.wallet = w
							}

							Qaterial.DialogManager.openFromComponent(_cmpSelectDialog)*/

							Client.stackPushPage("RpgWorldSelect.qml", {
													 world: Client.server.user.wallet.world
												 })
						}

						RpgSelectTitle {
							anchors.horizontalCenter: parent.horizontalCenter
							icon.source: Qaterial.Icons.earth
							text: qsTr("World")
						}
					}


					RpgSelectCard {
						id: _selectCharacter
						property RpgUserWallet wallet: null

						Layout.fillHeight: true
						Layout.fillWidth: true

						text: wallet ? wallet.baseReadableName : qsTr("Válassz...")
						image: wallet ? wallet.image : ""
						subImage: wallet ? wallet.subImage : ""
						locked: !wallet || !wallet.available
						selected: true
						onClicked: {
							_modelSelector.marketType = RpgMarket.Skin
							_dialogWallet = _selectCharacter.wallet
							_dialogAcceptFunc = function(w) {
								_selectCharacter.wallet = w

								if (_multiplayer)
									_multiplayer.selectCharacter(w.market.name)
							}

							Qaterial.DialogManager.openFromComponent(_cmpSelectDialog)
						}

						RpgSelectTitle {
							anchors.horizontalCenter: parent.horizontalCenter
							icon.source: Qaterial.Icons.humanMale
							text: qsTr("Character")
						}
					}


					Item {
						Layout.fillHeight: true
						Layout.fillWidth: true
						Layout.columnSpan: _grid1.columns === 2 ? 2 : 1

						implicitHeight: _buttonCol.implicitHeight
						implicitWidth: _buttonCol.implicitWidth

						Column {
							id: _buttonCol
							anchors.centerIn: parent
							spacing: 10 * Qaterial.Style.pixelSizeRatio

							QButton {
								id: _btnPlay

								anchors.horizontalCenter: parent.horizontalCenter

								icon.source: Qaterial.Icons.play
								icon.width: 28 * Qaterial.Style.pixelSizeRatio
								icon.height: 28 * Qaterial.Style.pixelSizeRatio

								bgColor: Qaterial.Colors.green700
								textColor: Qaterial.Colors.white
								topPadding: 10 * Qaterial.Style.pixelSizeRatio
								bottomPadding: 10 * Qaterial.Style.pixelSizeRatio
								leftPadding: 40 * Qaterial.Style.pixelSizeRatio
								rightPadding: 40 * Qaterial.Style.pixelSizeRatio

								outlined: !enabled

								enabled: _selectTerrain.wallet && _selectCharacter.wallet &&
										 _selectCharacter.wallet.available &&
										 (!_multiplayer || _multiplayer.playersModel.count > 1 || Client.debug)

								text: qsTr("Play")

								onClicked: {
									Client.Utils.settingsSet("rpg/skin", _selectCharacter.wallet.market.name)

									if (!_multiplayer || _multiplayer.gameMode == ActionRpgGame.MultiPlayerHost)
										Client.Utils.settingsSet("rpg/world", _selectTerrain.wallet.market.name)

									if (_multiplayer) {
										_btnPlay.enabled = false
										_busyIndicator.visible = true

										_multiplayer.selectionCompleted = true
									} else {
										_grid1.visible = false
										_busyIndicator.visible = true

										game.selectCharacter(_selectTerrain.wallet.market.name,
															 _selectCharacter.wallet.market.name)
									}
								}
							}

							Row {
								anchors.horizontalCenter: parent.horizontalCenter
								spacing: 15 * Qaterial.Style.pixelSizeRatio

								Qaterial.AppBarButton {
									icon.source: Qaterial.Icons.help
									ToolTip.text: qsTr("Súgó")
									onClicked: Qaterial.DialogManager.openFromComponent(_cmpKeyboardDialog)
									icon.width: Qaterial.Style.mediumIcon
									icon.height: Qaterial.Style.mediumIcon
									icon.color: Qaterial.Style.iconColor()
								}

								Qaterial.AppBarButton {
									icon.source: Qaterial.Icons.cartOutline
									ToolTip.text: qsTr("Vásárlás")
									onClicked: marketRequest()
									icon.width: Qaterial.Style.mediumIcon
									icon.height: Qaterial.Style.mediumIcon
									icon.color: Qaterial.Style.iconColor()
								}
							}

						}

					}


					Item {
						id: _otherPlayerItem

						visible: _multiplayer

						Layout.fillHeight: true
						Layout.fillWidth: true
						Layout.columnSpan: _grid1.columns

						implicitWidth: 150
						implicitHeight: 150

						RpgSelectTitle {
							id: _otherPlayerTitle
							anchors.left: parent.left

							icon.source: Qaterial.Icons.accountMultipleOutline
							text: qsTr("Players")
						}

						RpgSelectView {
							id: _viewPlayers

							anchors.left: parent.left
							anchors.right: parent.right
							//anchors.bottom: parent.bottom
							//anchors.top: _weaponTitle.bottom

							readonly property real _spacing: 5 * Qaterial.Style.pixelSizeRatio
							readonly property real _maxWidth: (_otherPlayerItem.width / (_multiplayer ? _multiplayer.maxPlayers+1 : 1))
															  - _spacing

							height: Math.max(90, Math.min(_maxWidth, _otherPlayerItem.height-_otherPlayerTitle.height))

							y: _otherPlayerTitle.height


							delegate: RpgSelectCard {
								id: _selectPlayer

								height: _viewPlayers.height
								width: _viewPlayers.height
								text: nickname
								image: game ? game.getCharacterImage(character) : ""
								selected: completed
								scale: 1.0

								QButton {
									visible: _multiplayer && _multiplayer.gameMode == ActionRpgGame.MultiPlayerHost &&
											 playerId != _multiplayer.playerId

									bgColor: Qaterial.Colors.red500
									icon.source: Qaterial.Icons.closeCircle

									anchors.right: parent.right
									anchors.top: parent.top
									anchors.margins: 3

									onClicked: _multiplayer.banOutPlayer(playerId)
								}
							}

							footer: Row {
								id: _placeholder

								spacing: _viewPlayers.spacing

								readonly property int num: _multiplayer ? Math.max(0, _multiplayer.maxPlayers-_multiplayer.playersModel.count) : 0

								visible: num > 0 && _multiplayer && !_multiplayer.locked && _multiplayer.gameMode == ActionRpgGame.MultiPlayerHost

								/*Repeater {
									model: _placeholder.num

									delegate: QButton {
										height: _viewPlayers.height
										width: _viewPlayers.height
										enabled: false

										icon.width: width * 0.4
										icon.height: height * 0.4
										icon.source: Qaterial.Icons.accountPlusOutline
									}
								}*/

								QButton {
									visible: _placeholder.num > 0

									height: _viewPlayers.height
									width: _viewPlayers.height

									flat: true
									outlined: false

									foregroundColor: Qaterial.Colors.red600
									outlinedColor: Qaterial.Colors.red500

									icon.source: Qaterial.Icons.lock
									icon.width: width * 0.3
									icon.height: height * 0.3

									onClicked: _multiplayer.lockEngine()
								}
							}

							model: _multiplayer ? _multiplayer.playersModel : null
						}

					}
				}

			}
		}
	}

	SortFilterProxyModel {
		id: _modelSelector
		sourceModel: Client.server ? Client.server.user.wallet : null

		property int marketType: 0

		filters: AllOf {
			ValueFilter {
				roleName: "marketType"
				value: _modelSelector.marketType > 0 ? _modelSelector.marketType : RpgMarket.Invalid
				enabled: _modelSelector.marketType > 0
			}
			ValueFilter {
				roleName: "available"
				value: true
			}
		}

		sorters: [
			RoleSorter {
				roleName: "available"
				priority: 1
				sortOrder: Qt.DescendingOrder
			},

			StringSorter {
				roleName: "sortName"
				priority: 0
				sortOrder: Qt.AscendingOrder
			}
		]
	}


	property RpgUserWallet _dialogWallet: null
	property var _dialogAcceptFunc: null

	Component {
		id: _cmpSelectDialog

		Qaterial.ModalDialog
		{
			id: _dialog
			//horizontalPadding: 0

			title: switch (_modelSelector.marketType) {
				   case RpgMarket.Map:
					   return qsTr("World")
				   case RpgMarket.Skin:
					   return qsTr("Character")
				   default:
					   return ""
				   }

			dialogImplicitWidth: 1200

			property RpgUserWallet selectedWallet: _dialogWallet

			contentItem: RpgSelectView {
				id: _dialogView

				implicitHeight: 200*Qaterial.Style.pixelSizeRatio

				delegate: RpgSelectCard {
					property RpgUserWallet wallet: model.qtObject
					height: _dialogView.height
					width: _dialogView.height
					text: wallet.baseReadableName
					image: wallet.image
					subImage: wallet.subImage
					locked: !wallet.available
					selected: wallet == _dialog.selectedWallet
					onClicked: {
						if (wallet && wallet.available)
							_dialog.selectedWallet = wallet
						else
							_dialog.selectedWallet = null
					}
				}

				model: _modelSelector
			}

			onSelectedWalletChanged: {
				let idx = -1
				if (selectedWallet) {
					for (let i=0; i<_modelSelector.count; ++i) {
						if (_modelSelector.get(i).qtObject === selectedWallet) {
							idx = i
							break
						}
					}
				}

				if (idx != -1)
					_dialogView.positionViewAtIndex(idx, ListView.Contain)
			}

			standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok

			onAccepted: if (_dialogAcceptFunc && selectedWallet && selectedWallet.available)
							_dialogAcceptFunc(selectedWallet)

			Component.onCompleted: selectedWalletChanged()
		}
	}

	Component {
		id: _cmpKeyboardDialog

		RpgKeyboardInfoDialog { }
	}


	Timer {
		id: _reloadTimer
		interval: 30000
		repeat: true
		triggeredOnStart: true
		onTriggered: Client.server.user.wallet.reload()
	}


	function getWallet(_type, _name) {
		if (!Client.server)
			return

		let _model = Client.server.user.wallet

		for (let n=0; n<_model.count; ++n) {
			let w = _model.get(n)
			if (w.market.type === _type && w.market.name === _name)
				return w
		}

		return null
	}



	Connections {
		target: Client.server ? Client.server.user.wallet.world : null

		function onSelectedLandChanged() {
			_selectTerrain.wallet = Client.server.user.wallet.worldGetSelectedWallet()

			if (_multiplayer && _multiplayer.gameMode == ActionRpgGame.MultiPlayerHost)
				_multiplayer.selectTerrain(_selectTerrain.wallet.market.name)
		}
	}


	Connections {
		target: Client.server ? Client.server.user.wallet : null

		function onReloaded() {
			if (!_isFirst)
				return

			let w = getWallet(RpgMarket.Skin, Client.Utils.settingsGet("rpg/skin", ""))
			_selectCharacter.wallet = w

			if (_multiplayer && w)
				_multiplayer.selectCharacter(w.market.name)

			let t = Client.server.user.wallet.worldGetSelectedWallet()

			if (_multiplayer && _multiplayer.gameMode == ActionRpgGame.MultiPlayerHost && t)
				_multiplayer.selectTerrain(t.market.name)

			if (Client.server.user.wallet.world) {
				if (!_multiplayer || _multiplayer.gameMode == ActionRpgGame.MultiPlayerHost)
					Client.server.user.wallet.world.select(Client.Utils.settingsGet("rpg/world", ""))
				else if (_multiplayer && _multiplayer.gameMode == ActionRpgGame.MultiPlayerGuest)
					_selectTerrain.wallet = Client.server.user.wallet.worldGetSelectedWallet()
			}


			_isFirst = false
		}
	}

	StackView.onActivated: {
		Client.sound.playSound("qrc:/sound/voiceover/choose_your_character.mp3", Sound.VoiceoverChannel)

		if (!Client.server)
			return

		if (Client.server.user.wallet.world) {
			let w = Client.server.user.wallet.worldGetSelectedWallet()

			_selectTerrain.wallet = w
			if (w && _multiplayer && _multiplayer.gameMode == ActionRpgGame.MultiPlayerHost)
				_multiplayer.selectTerrain(w.market.name)
		}

		_reloadTimer.start()
	}
}

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

	title: game ? game.name + qsTr(" – level %1").arg(game.level): ""

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
			height: Math.min(parent.height, 500)

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

						text: wallet ? wallet.readableName : qsTr("Válassz...")
						image: wallet ? wallet.image : ""
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
										 _selectCharacter.wallet.available

								text: qsTr("Play")

								onClicked: {
									Client.Utils.settingsSet("rpg/skin", _selectCharacter.wallet.market.name)

									if (!_multiplayer || _multiplayer.gameMode == ActionRpgGame.MultiPlayerHost)
										Client.Utils.settingsSet("rpg/world", _selectTerrain.wallet.market.name)

									/*let noW = []

									for (let i=0; i<_viewWeapons.model.count; ++i) {
										let wpn = _viewWeapons.model.get(i)
										if (!_viewWeapons.selectedList.includes(wpn.market.name))
											noW.push(wpn.market.name)
									}

									Client.Utils.settingsSet("rpg/disabledWeapons", noW.join(","))*/

									let disList = game.getDisabledWeapons(_selectCharacter.wallet.market.name)

									let wList = []
									for (let n=0; n<_viewWeapons.model.count; ++n) {
										let w = _viewWeapons.model.get(n).market.name
										if (!disList.includes(w))
											wList.push(w)
									}


									if (_multiplayer) {
										_btnPlay.enabled = false
										_busyIndicator.visible = true

										_multiplayer.selectWeapons(wList)
										_multiplayer.selectionCompleted = true
									} else {
										_grid1.visible = false
										_busyIndicator.visible = true

										game.selectCharacter(_selectTerrain.wallet.market.name,
															 _selectCharacter.wallet.market.name,
															 wList)
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
									onClicked: game.marketRequest()
									icon.width: Qaterial.Style.mediumIcon
									icon.height: Qaterial.Style.mediumIcon
									icon.color: Qaterial.Style.iconColor()
								}
							}

						}

					}


					Item {
						id: _weaponItem

						Layout.fillHeight: true
						Layout.fillWidth: true
						Layout.columnSpan: _grid1.columns

						implicitWidth: 150
						implicitHeight: 150

						RpgSelectTitle {
							id: _weaponTitle
							anchors.left: parent.left

							icon.source: Qaterial.Icons.swordCross
							text: qsTr("Weapons")
						}

						RpgSelectView {
							id: _viewWeapons

							anchors.left: parent.left
							anchors.right: parent.right
							//anchors.bottom: parent.bottom
							//anchors.top: _weaponTitle.bottom

							readonly property real _height: _weaponItem.height-_weaponTitle.height

							height: Math.min(_height, 120)

							y: _weaponTitle.height + Math.max(0, (_height-height)/2)


							delegate: RpgSelectCard {
								id: _selectWeapon

								property RpgUserWallet wallet: model.qtObject

								height: _viewWeapons.height
								width: _viewWeapons.height
								text: wallet.readableName
								image: wallet.image
								bulletCount: wallet.market.cost == 0 ? -1 : wallet.amount //wallet.bullet ? wallet.bullet.amount : -1
								//selected: _viewWeapons.selectedList.includes(wallet.market.name)
								selected: (wallet.market.cost == 0 || wallet.amount > 0) && !disabled
								/*onClicked: {
									if (selected)
										_viewWeapons.unselectMore([wallet])
									else
										_viewWeapons.selectMore([wallet])
								}*/

								onWalletChanged: checkDisabled()

								Connections {
									target: _selectCharacter

									function onWalletChanged() {
										_selectWeapon.checkDisabled()
									}
								}

								function checkDisabled() {
									if (!wallet || !game || !_selectCharacter.wallet) {
										disabled = false
										return
									}

									let wList = game.getDisabledWeapons(_selectCharacter.wallet.market.name)

									disabled = wList.includes(wallet.market.name)
								}
							}

							model: SortFilterProxyModel {
								sourceModel: Client.server ? Client.server.user.wallet : null

								filters: AllOf {
									ValueFilter {
										roleName: "marketType"
										value: RpgMarket.Weapon
									}
									ValueFilter {
										roleName: "available"
										value: true
									}
								}

								sorters: [
									StringSorter {
										roleName: "sortName"
										priority: 0
										sortOrder: Qt.AscendingOrder
									}
								]
							}
						}

					}

					Column {
						id: _playersItem

						Layout.fillHeight: true
						Layout.fillWidth: true
						Layout.columnSpan: _grid1.columns

						visible: _multiplayer


						Repeater {
							model: _multiplayer ? _multiplayer.playersModel : null

							delegate: Qaterial.ItemDelegate {
								width: parent.width
								anchors.horizontalCenter: parent.horizontalCenter
								text: username + " " + nickname + " - " + character + ": " + completed

								icon.source: Qaterial.Icons.account

								onClicked: {

								}
							}
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
					text: wallet.readableName
					image: wallet.image
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

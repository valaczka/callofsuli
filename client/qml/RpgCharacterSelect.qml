import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import SortFilterProxyModel 0.2
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

QItemGradient {
	id: root

	property ActionRpgGame game: null

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
			height: Math.min(parent.height, 600)

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

						text: wallet ? wallet.readableName : qsTr("Válassz...")
						image: wallet ? wallet.image : ""
						locked: !wallet || !wallet.available
						selected: true
						onClicked: {
							_modelSelector.marketType = RpgMarket.Map
							_dialogWallet = _selectTerrain.wallet
							_dialogAcceptFunc = function(w) {
								_selectTerrain.wallet = w
							}

							Qaterial.DialogManager.openFromComponent(_cmpSelectDialog)
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
										 _selectTerrain.wallet.available && _selectCharacter.wallet.available

								text: qsTr("Play")

								onClicked: {
									Client.Utils.settingsSet("rpg/skin", _selectCharacter.wallet.market.name)
									Client.Utils.settingsSet("rpg/world", _selectTerrain.wallet.market.name)

									let noW = []

									for (let i=0; i<_viewWeapons.model.count; ++i) {
										let wpn = _viewWeapons.model.get(i)
										if (!_viewWeapons.selectedList.includes(wpn.market.name))
											noW.push(wpn.market.name)
									}

									Client.Utils.settingsSet("rpg/disabledWeapons", noW.join(","))

									_grid1.visible = false
									_busyIndicator.visible = true

									game.selectCharacter(_selectTerrain.wallet.market.name,
														 _selectCharacter.wallet.market.name,
														 _viewWeapons.selectedList)
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
									icon.source: Qaterial.Icons.bank
									ToolTip.text: qsTr("Bank")
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

							height: Math.min(_height, 200)

							y: _weaponTitle.height + Math.max(0, (_height-height)/2)


							delegate: RpgSelectCard {
								property RpgUserWallet wallet: model.qtObject

								height: _viewWeapons.height
								width: _viewWeapons.height
								text: wallet.readableName
								image: wallet.image
								bulletCount: wallet.bullet ? wallet.bullet.amount : -1
								selected: _viewWeapons.selectedList.includes(wallet.market.name)
								onClicked: {
									if (selected)
										_viewWeapons.unselectMore([wallet])
									else
										_viewWeapons.selectMore([wallet])
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
		target: Client.server ? Client.server.user.wallet : null

		function onReloaded() {
			if (!_isFirst)
				return

			_selectTerrain.wallet = getWallet(RpgMarket.Map, Client.Utils.settingsGet("rpg/world", ""))
			_selectCharacter.wallet = getWallet(RpgMarket.Skin, Client.Utils.settingsGet("rpg/skin", ""))

			let sList = []

			let wList = Client.Utils.settingsGet("rpg/disabledWeapons", "").split(",")
			for (let n=0; n<_viewWeapons.model.count; ++n) {
				let w = _viewWeapons.model.get(n)
				if (!wList.includes(w.market.name))
					sList.push(w)
			}

			_viewWeapons.selectMore(sList)

			_isFirst = false
		}
	}

	StackView.onActivated: {
		Client.sound.playSound("qrc:/sound/voiceover/choose_your_character.mp3", Sound.VoiceoverChannel)

		if (!Client.server)
			return

		Client.server.user.wallet.reload()
	}
}

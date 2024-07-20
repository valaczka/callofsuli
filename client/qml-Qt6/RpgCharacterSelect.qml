import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SortFilterProxyModel
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as J

QItemGradient {
	id: root

	property ActionRpgGame game: null

	property bool _isFirst: true

	title: game ? game.name + qsTr(" – level %1").arg(game.level): ""

	appBar.rightComponent: Qaterial.AppBarButton {
		icon.source: Qaterial.Icons.bank
		enabled: game
		onClicked: game.marketRequest()
	}

	Qaterial.BusyIndicator {
		id: _busyIndicator
		anchors.centerIn: parent
		visible: false
	}

	QScrollable {
		id: _scrollable

		anchors.fill: parent

		horizontalPadding: 0
		verticalPadding: 0
		leftPadding: 0
		rightPadding: 0

		refreshEnabled: true
		onRefreshRequest: if (Client.server)
							  Client.server.user.wallet.reload()

		Item {
			width: parent.width
			height: root.paddingTop
		}


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

			enabled: _viewCharacter.selected != "" && _viewTerrain.selected != ""
			text: qsTr("Play")

			onClicked: {
				Client.Utils.settingsSet("rpg/skin", _viewCharacter.selected)
				Client.Utils.settingsSet("rpg/world", _viewTerrain.selected)

				let noW = []

				for (let i=0; i<_viewWeapons.model.count; ++i) {
					let wpn = _viewWeapons.model.get(i)
					if (!_viewWeapons.selectedList.includes(wpn.market.name))
						noW.push(wpn.market.name)
				}

				Client.Utils.settingsSet("rpg/disabledWeapons", noW.join(","))

				_scrollable.visible = false
				_busyIndicator.visible = true

				game.selectCharacter(_viewTerrain.selected, _viewCharacter.selected, _viewWeapons.selectedList)
			}
		}

		Item {
			height: 20 * Qaterial.Style.pixelSizeRatio
			width: parent.width
		}


		RpgSelectTitle {
			anchors.left: parent.left
			icon.source: Qaterial.Icons.earth
			text: qsTr("World")
		}


		RpgSelectView {
			id: _viewTerrain
			width: root.width

			implicitHeight: 125*Qaterial.Style.pixelSizeRatio

			delegate: RpgSelectCard {
				property RpgUserWallet wallet: model.qtObject

				height: _viewTerrain.implicitHeight
				width: _viewTerrain.implicitHeight
				text: wallet.readableName
				image: wallet.image
				locked: !wallet.available
				selected: wallet.market.name === _viewTerrain.selected
				onClicked: _viewTerrain.selectOne(wallet)
			}

			model: SortFilterProxyModel {
				sourceModel: Client.server ? Client.server.user.wallet : null

				filters: AllOf {
					ValueFilter {
						roleName: "marketType"
						value: RpgMarket.Map
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
		}


		RpgSelectTitle {
			anchors.left: parent.left
			icon.source: Qaterial.Icons.humanMale
			text: qsTr("Skin")
		}


		RpgSelectView {
			id: _viewCharacter
			width: root.width

			implicitHeight: 150*Qaterial.Style.pixelSizeRatio

			delegate: RpgSelectCard {
				property RpgUserWallet wallet: model.qtObject
				height: _viewCharacter.implicitHeight
				width: _viewCharacter.implicitHeight
				text: wallet.readableName
				image: wallet.image
				locked: !wallet.available
				selected: wallet.market.name === _viewCharacter.selected
				onClicked: _viewCharacter.selectOne(wallet)
			}

			model: SortFilterProxyModel {
				sourceModel: Client.server ? Client.server.user.wallet : null

				filters: AllOf {
					ValueFilter {
						roleName: "marketType"
						value: RpgMarket.Skin
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
		}

		RpgSelectTitle {
			anchors.left: parent.left
			icon.source: Qaterial.Icons.swordCross
			text: qsTr("Weapons")
		}


		RpgSelectView {
			id: _viewWeapons
			width: root.width

			delegate: RpgSelectCard {
				property RpgUserWallet wallet: model.qtObject

				height: _viewWeapons.implicitHeight
				width: _viewWeapons.implicitHeight
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

		Item {
			height: 20 * Qaterial.Style.pixelSizeRatio
			width: parent.width
		}


		Qaterial.Expandable {
			id: _expKeyboard
			width: Math.min(root.width, 450 * Qaterial.Style.pixelSizeRatio, Qaterial.Style.maxContainerSize)

			visible: Qt.platform.os === "linux" ||
					 Qt.platform.os === "windows" ||
					 Qt.platform.os === "osx" ||
					 Qt.platform.os === "wasm"

			anchors.horizontalCenter: parent.horizontalCenter

			expanded: false

			header: QExpandableHeader {
				text: qsTr("Segítség a billentyűzet használatához")
				icon: Qaterial.Icons.keyboardOutline
				expandable: _expKeyboard
				topPadding: 20
			}

			delegate: GridLayout {
				width: _expKeyboard.width
				columns: 2
				columnSpacing: 5
				rowSpacing: 3

				Repeater {
					model: [
						[ "W", "A", "S", "D",
						 "1", "2", "3", "4", "6", "7", "8", "9",
						 "Arrow_Up", "Arrow_Down", "Arrow_Left", "Arrow_Right", "Page_Up", "Page_Down", "Home", "End",
						], qsTr("Mozgás"),
						[ "0", "Space", "Insert" ], qsTr("Lövés, vágás, ütés"),
						[ "E", "Enter_Alt", "Enter_Tall" ], qsTr("Láda kinyitása"),
						[ "Del", "Q" ], qsTr("Fegyver váltás"),
						[ "5", "X" ], qsTr("Átjáró használata"),
						[ "C" ], qsTr("Varázsbot használata"),
						[ "Tab" ], qsTr("Térkép")
					]

					delegate: Item {
						id: _helperItem
						readonly property bool isKeyItem: index % 2 == 0
						readonly property var keys: isKeyItem ? modelData : null
						readonly property string text: isKeyItem ? "" : modelData

						implicitWidth: isKeyItem ? _flow.implicitWidth : _text.implicitWidth
						implicitHeight: isKeyItem ? _flow.implicitHeight : _text.implicitHeight

						Layout.maximumWidth: isKeyItem ? _expKeyboard.width*0.4 : -1
						Layout.fillWidth: true

						Flow {
							id: _flow
							anchors.fill: parent
							visible: _helperItem.isKeyItem
							spacing: 0

							Repeater {
								model: _helperItem.isKeyItem ? _helperItem.keys : null
								delegate: Image {
									source: "qrc:/internal/keyboard/%1_Key_Dark.png".arg(modelData)
									fillMode: Image.Pad
									smooth: false
									width: 40
									height: 40
								}
							}
						}

						Qaterial.LabelBody1 {
							id: _text
							anchors.fill: parent
							visible: !_helperItem.isKeyItem
							text: _helperItem.text
							color: Qaterial.Style.secondaryTextColor()
						}
					}
				}
			}
		}

	}


	function getWallet(_model, _name) {
		for (let n=0; n<_model.count; ++n) {
			let w = _model.get(n)
			if (w.market.name === _name)
				return w
		}

		return null
	}


	Connections {
		target: Client.server ? Client.server.user.wallet : null

		function onReloaded() {
			if (!_isFirst)
				return

			_viewCharacter.selectOne(getWallet(_viewCharacter.model, Client.Utils.settingsGet("rpg/skin", "")))
			_viewTerrain.selectOne(getWallet(_viewTerrain.model, Client.Utils.settingsGet("rpg/world", "")))

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

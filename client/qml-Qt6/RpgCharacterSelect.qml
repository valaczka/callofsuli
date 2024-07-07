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

	title: game ? game.name + qsTr(" – level %1").arg(game.level): ""

	appBar.rightComponent: Qaterial.AppBarButton {
		icon.source: Qaterial.Icons.shopping
		//onClicked: Client.stackPushPage("PageStudentSettings.qml")
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

		/*refreshEnabled: true
		onRefreshRequest: if (map)
							  map.updateSolver()*/

		Item {
			width: parent.width
			height: root.paddingTop
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
				height: _viewTerrain.implicitHeight
				width: _viewTerrain.implicitHeight
				text: name
				image: "qrc:/internal/img/metalbg3.png"
				locked: !available
				selected: _viewTerrain.currentIndex == index
				onClicked: selectIndex(_viewTerrain, index)
			}

			model: SortFilterProxyModel {
				sourceModel: _modelTerrain

				/*filters: ValueFilter {
					roleName: "lockDepth"
					value: -1
					inverted: true
				}*/

				sorters: [
					RoleSorter {
						roleName: "available"
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
				height: _viewCharacter.implicitHeight
				width: _viewCharacter.implicitHeight
				text: name
				image: model.image
				locked: !available
				selected: _viewCharacter.currentIndex == index
				onClicked: selectIndex(_viewCharacter, index)
			}

			model: SortFilterProxyModel {
				sourceModel: _modelCharacter

				/*filters: ValueFilter {
					roleName: "available"
					value: true
				}*/

				sorters: [
					RoleSorter {
						roleName: "available"
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
				height: _viewWeapons.implicitHeight
				width: _viewWeapons.implicitHeight
				text: name
				image: "qrc:/rpg/"+weaponId+"/market.jpg"
				bulletCount: bullet
				selected: model.selected
				onClicked: _viewWeapons.model.setProperty(index, "selected", !selected)
			}

			model: _modelWeapons
		}

		Item {
			height: 40 * Qaterial.Style.pixelSizeRatio
			width: parent.width
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

			enabled: _viewCharacter.currentIndex != -1 && _viewTerrain.currentIndex != -1
			text: qsTr("Play")

			onClicked: {
				let ch = _viewCharacter.model.get(_viewCharacter.currentIndex).characterId
				Client.Utils.settingsSet("rpg/skin", ch)

				let t = _viewTerrain.model.get(_viewTerrain.currentIndex).terrainId
				Client.Utils.settingsSet("rpg/world", t)

				let w = []
				let noW = []

				for (let i=0; i<_viewWeapons.model.count; ++i) {
					let wpn = _viewWeapons.model.get(i)
					if (wpn.selected)
						w.push(wpn.weaponId)
					else
						noW.push(wpn.weaponId)
				}


				Client.Utils.settingsSet("rpg/disabledWeapons", noW.join(","))

				_scrollable.visible = false
				_busyIndicator.visible = true

				game.selectCharacter(t, ch, w)
			}
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
						[ "E", "Enter_Alt", "Enter_Tall" ], qsTr("Tárgy felvétele"),
						[ "Del", "Q" ], qsTr("Fegyver váltás"),
						[ "5", "X" ], qsTr("Átjáró használata"),
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


	ListModel {
		id: _modelTerrain

		function reload() {
			clear()
			_viewTerrain.currentIndex = -1

			if (!game)
				return

			let list = game.getTerrainList()

			for (let i=0; i<list.length; ++i) {
				append(list[i])
			}
		}
	}

	ListModel {
		id: _modelCharacter

		function reload() {
			clear()

			_viewCharacter.currentIndex = -1

			if (!game)
				return

			let list = game.getCharacterList()

			for (let i=0; i<list.length; ++i) {
				append(list[i])
			}
		}
	}


	ListModel {
		id: _modelWeapons

		function reload() {
			clear()

			if (!game)
				return

			let list = game.getWeaponList()

			for (let i=0; i<list.length; ++i) {
				let l=list[i]
				l.selected = true
				append(l)
			}
		}
	}


	function selectIndex(_view, _index)
	{
		if (_index >= 0 && _index < _view.model.count &&
				_view.model.get(_index).available === true) {
			_view.currentIndex = _index
			_view.positionViewAtIndex(_index, ListView.Contain)
		} else
			_view.currentIndex = -1
	}



	function selectItem(_view, _field, _item)
	{
		if (_item === "") {
			_view.currentIndex = -1
			return
		}

		for (let n=0; n<_view.model.count; ++n) {
			let p = _view.model.get(n)
			if (p[_field] === _item &&
					p.available === true) {
				_view.currentIndex = n
				_view.positionViewAtIndex(n, ListView.Contain)
				return
			}
		}

		_view.currentIndex = -1
	}


	Component.onCompleted: {
		_modelTerrain.reload()
		_modelCharacter.reload()
		_modelWeapons.reload()
	}

	StackView.onActivated: {
		Client.sound.playSound("qrc:/sound/voiceover/choose_your_character.mp3", Sound.VoiceoverChannel)

		selectItem(_viewCharacter, "characterId", Client.Utils.settingsGet("rpg/skin", ""))
		selectItem(_viewTerrain, "terrainId", Client.Utils.settingsGet("rpg/world", ""))
		let wList = Client.Utils.settingsGet("rpg/disabledWeapons", "").split(",")
		for (let n=0; n<_viewWeapons.model.count; ++n) {
			let w = _viewWeapons.model.get(n).weaponId
			if (wList.includes(w))
				_viewWeapons.model.setProperty(n, "selected", false)
		}
	}
}

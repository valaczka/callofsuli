import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as J

QScrollable {
	id: root

	property ActionRpgGame game: null
	readonly property real groupWidth: Math.min(width, 450 * Qaterial.Style.pixelSizeRatio, Qaterial.Style.maxContainerSize)

	contentCentered: true

	QLabelInformative {
		visible: _groupBoxCharacter.visible
		text: qsTr("Válassz karaktert:")
		bottomPadding: 10 * Qaterial.Style.pixelSizeRatio
		topPadding: root.paddingTop
	}

	Qaterial.GroupBox {
		id: _groupBoxCharacter
		title: qsTr("Karakter")

		width: groupWidth

		anchors.horizontalCenter: parent.horizontalCenter
		inlineTitle: true

		enabled: true
		//visible: game && game.hostMode == ConquestGame.ModeHost && enabled

		Column {
			width: parent.width

			Repeater {
				model: game ? game.characterList : null

				delegate: Qaterial.FullLoaderItemDelegate {
					width: parent.width
					height: 65

					leftSourceComponent: Image
					{
						fillMode: Image.PreserveAspectFit
						source: modelData ? modelData.image : ""
						width: 55
						height: 55
					}

					contentSourceComponent: Qaterial.LabelBody1 {
						text: modelData ? modelData.displayName : ""
						verticalAlignment: Text.AlignVCenter
						elide: implicitWidth > width ? Text.ElideRight : Text.ElideNone
					}

					onClicked: {
						_groupBoxCharacter.enabled = false
						onClicked: game.selectCharacter(modelData.id)
					}
				}
			}
		}
	}


	Qaterial.Expandable {
		id: _expKeyboard
		width: groupWidth

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
			width: groupWidth
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

					Layout.maximumWidth: isKeyItem ? groupWidth*0.4 : -1
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



	StackView.onActivated: {
		Client.sound.playSound("qrc:/sound/voiceover/choose_your_character.mp3", Sound.VoiceoverChannel)
	}
}

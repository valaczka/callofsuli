import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.ModalDialog
{
	id: root

	title: qsTr("Segítség a billentyűzet használatához")

	dialogImplicitWidth: 600
	autoFocusButtons: true

	contentItem: Flickable
	{
		id: _flickable
		width: parent.width

		implicitHeight: Math.min(400, _layout.implicitHeight)
		interactive: contentHeight > height

		contentHeight: _layout.height
		flickableDirection: Flickable.VerticalFlick

		boundsBehavior: Flickable.StopAtBounds
		boundsMovement: Flickable.StopAtBounds

		ScrollIndicator.vertical: Qaterial.ScrollIndicator {}

		clip: true


		GridLayout {
			id: _layout
			columns: 2
			columnSpacing: 5
			rowSpacing: 3

			width: _flickable.width

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
					[ "C" ], qsTr("Super power"),
					[ "Tab" ], qsTr("Térkép")
				]

				delegate: Item {
					id: _helperItem
					readonly property bool isKeyItem: index % 2 == 0
					readonly property var keys: isKeyItem ? modelData : null
					readonly property string text: isKeyItem ? "" : modelData

					implicitWidth: 200//isKeyItem ? _flow.implicitWidth : _text.implicitWidth
					implicitHeight: isKeyItem ? _flow.implicitHeight : _text.implicitHeight

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


	standardButtons: DialogButtonBox.Ok
}

import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
	id: control

	title: qsTr("Beállítások")
	icon: CosStyle.iconPreferences

	property real buttonSize: CosStyle.twoLineHeight*3
	readonly property real _contentWidth: Math.min(width-10, 600)

	Flickable {
		id: flickable
		width: Math.min(parent.width, contentWidth)
		height: Math.min(parent.height, contentHeight)
		anchors.centerIn: parent

		contentWidth: grid.width
		contentHeight: grid.height

		clip: true

		flickableDirection: Flickable.VerticalFlick
		boundsBehavior: Flickable.StopAtBounds

		ScrollIndicator.vertical: ScrollIndicator { }


		Grid {
			id: grid
			columns: Math.floor(_contentWidth/(buttonSize+spacing))
			spacing: buttonSize*0.2

			bottomPadding: 20

			Repeater {
				id: groupRepeater

				model: ListModel {
					ListElement {
						title: qsTr("Szerver beállítások")
						icon: "image://font/Material Icons/\ue869"
						page: "ServerSettings"
						textcolor: "white"
						background: "#940000"
					}

					ListElement {
						title: qsTr("Osztályok és felhasználók")
						icon: "image://font/Material Icons/\ue7ef"
						page: "Users"
						textcolor: "white"
						background: "#940000"
					}
				}

				QCard {
					id: groupItem
					height: buttonSize
					width: buttonSize

					required property string background
					required property string textcolor
					required property string icon
					required property string title
					required property string page

					backgroundColor: background

					onClicked: {
						JS.createPage("Admin", { page: page })
					}

					Column {
						anchors.centerIn: parent

						QFontImage {
							id: image
							anchors.horizontalCenter: parent.horizontalCenter
							size: groupItem.width*0.5
							color: groupItem.textcolor
							visible: icon
							icon: groupItem.icon
						}

						QLabel {
							id: title
							color: groupItem.textcolor
							font.weight: Font.DemiBold
							width: groupItem.width*0.8
							horizontalAlignment: Text.AlignHCenter
							wrapMode: Text.Wrap
							maximumLineCount: 2
							elide: Text.ElideRight
							font.capitalization: Font.AllUppercase
							text: groupItem.title
						}

					}
				}
			}

		}

	}

	onPopulated: {
	}

}

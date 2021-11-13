import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


GridLayout {
	id: grid

	columns: parent.width > parent.height ? 2 : 1

	property alias flick: flick
	property alias container: container

	//default property alias flickContent: flick.contentItem

	Item {
		Layout.fillWidth: true
		Layout.fillHeight: true

		implicitHeight: 50
		implicitWidth: 50

		Flickable {
			id: flick

			width: parent.width-20
			height: Math.min(parent.height-10, flick.contentHeight)
			anchors.centerIn: parent

			clip: true

			/*contentWidth: ctItem.width
			contentHeight: ctItem.height*/

			boundsBehavior: Flickable.StopAtBounds
			flickableDirection: Flickable.VerticalFlick

			ScrollIndicator.vertical: ScrollIndicator { }



		}

		Rectangle {
			id: rectLine
			anchors.top: grid.columns > 1 ? parent.top : parent.bottom
			anchors.left: grid.columns > 1 ? parent.right : parent.left
			height: grid.columns > 1 ? parent.height : 1
			width: grid.columns > 1 ? 1 : parent.width

			gradient: Gradient {
				orientation: grid.columns > 1 ? Gradient.Vertical : Gradient.Horizontal
				GradientStop { position: 0.0; color: "transparent" }
				GradientStop { position: 0.1; color: CosStyle.colorAccent }
				GradientStop { position: 0.9; color: CosStyle.colorAccent }
				GradientStop { position: 1.0; color: "transparent" }
			}
		}
	}



	GameQuestionTileContainer {
		id: container
		Layout.fillWidth: !(grid.columns > 1)
		Layout.maximumWidth: (grid.columns > 1) ? Math.min(Math.max(implicitWidth, grid.parent.width*0.2), grid.parent.width*0.4) : -1
		Layout.fillHeight: (grid.columns > 1)
		Layout.maximumHeight: (grid.columns > 1) ? -1 : Math.min(Math.max(implicitHeight, grid.parent.height*0.3), grid.parent.height*0.4)
	}

}

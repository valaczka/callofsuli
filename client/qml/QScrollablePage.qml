import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


Qaterial.ScrollablePage {
	id: control

	property string closeQuestion: ""					// Kérdés a lap bezárása előtt
	property string closeDisabled: ""					// Teljes mértékben tiltjuk a lap bezárását
	property var stackPopFunction: null					// Visszalépés előtt végrehajtandó metódus (return: true - vissza lehet lépni, false - nem lehet visszalépni)

	property alias appBar: appBar
	property alias subtitle: appBar.subtitle

	enabled: StackView.view && StackView.index == StackView.view.depth-1

	default property alias realContentData: realContent.data

	pane.padding: Qaterial.Style.horizontalPadding

	header: AppBar {
		id: appBar
		title: control.title
	}

	background: Item {
		anchors.fill: parent
		Image {
			id: bgImage
			anchors.fill: parent
			fillMode: Image.PreserveAspectCrop
			source: "qrc:/internal/img/villa.png"
		}
	}

	Item {
		id: realContent

		width: parent.width
	}

}

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as J

QPage {
	id: control

	title: qsTr("MultiPlayer ") + (game ? (game.multiPlayerMode == MultiPlayerGame.MultiPlayerHost ? "Host" : "Client") : "???")

	property MultiPlayerGame game: null
	//property string closeDisabled: qsTr("A játék előkészítése alatt nem lehet bezárni a lapot!")
	//property string closeQuestion: qsTr("Biztosan megszakítod a játékot?")
	property var onPageClose: function() { if (game) game.gameAbort() }

	readonly property int stackViewIndex: StackView.index

	QScrollable {
		anchors.fill: parent

		Column {
			id: col
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize) //parent.width-(Math.max(Client.safeMarginLeft, Client.safeMarginRight, 10)*2)
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: 10

			Item {
				width: parent.width
				height: 20
			}

			CosImage {
				id: cosImage
				anchors.horizontalCenter: parent.horizontalCenter
				width: Math.min(parent.width*0.7, 800)
				glow: false
			}

			Qaterial.LabelBody2 {
				id: labelVersion
				horizontalAlignment: Qt.AlignHCenter
				wrapMode: Text.Wrap
				width: parent.width
				color: Qaterial.Style.primaryTextColor()

				padding: 20

				text: qsTr("Verzió: ")+Qt.application.version+"\n© 2012-2023 Valaczka János Pál"
			}

			QButton {
				anchors.horizontalCenter: parent.horizontalCenter
				text: "HOST"
				onClicked: if (game)
							   game.multiPlayerMode = MultiPlayerGame.MultiPlayerHost
			}

			QButton {
				anchors.horizontalCenter: parent.horizontalCenter
				text: "CLIENT"
				onClicked: if (game)
							   game.multiPlayerMode = MultiPlayerGame.MultiPlayerClient
			}

			QButton {
				anchors.horizontalCenter: parent.horizontalCenter
				text: "START"
				onClicked: game.start()
			}
		}
	}
}

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as J

Page {
	id: root

	property ConquestGameAdjacencySetup game: null

	ConquestScene {
		id: _scene
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.right: _view.left
		game: root.game
	}

	QListView {
		id: _view
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.right: parent.right
		width: Math.min(parent.width*0.25, 250 * Qaterial.Style.pixelSizeRatio)

		model: game ? game.landDataList : null

		delegate: QItemDelegate {
			text: landId

			highlighted: game && game.currentLandId == landId

			onClicked: {
				game.currentLandId = landId
			}
		}

		footer: QButton {
			highlighted: true
			icon.source: Qaterial.Icons.contentSave
			display: AbstractButton.TextBesideIcon
			text: qsTr("Mentés")
			enabled: game
			onClicked: game.save()
		}
	}
}

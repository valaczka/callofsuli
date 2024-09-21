import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as J

QScrollable {
	id: root

	property ConquestGame game: null
	readonly property real groupWidth: Math.min(width, 450 * Qaterial.Style.pixelSizeRatio, Qaterial.Style.maxContainerSize)

	contentCentered: true

	QLabelInformative {
		visible: _groupBoxWorld.visible
		text: qsTr("Válassz pályát:")
		bottomPadding: 10 * Qaterial.Style.pixelSizeRatio
		topPadding: root.paddingTop
	}

	Qaterial.GroupBox {
		id: _groupBoxWorld
		title: qsTr("Pálya")

		width: groupWidth

		anchors.horizontalCenter: parent.horizontalCenter
		inlineTitle: true

		enabled: true
		visible: game && game.hostMode == ConquestGame.ModeHost && enabled

		Column {
			width: parent.width

			Repeater {
				model: game ? game.worldListSelect : null

				delegate: Qaterial.LoaderItemDelegate {
					text: modelData ? modelData.description : ""

					leftSourceComponent: Image
					{
						fillMode: Image.PreserveAspectFit
						source: modelData ? "qrc:/content/%1/bg.png".arg(modelData.name) : ""
						sourceSize: Qt.size(width, height)
					}

					width: parent.width
					onClicked: {
						_groupBoxWorld.enabled = false
							game.sendWebSocketMessage({
														  cmd: "play",
														  engine: game.engineId,
														  world: modelData.name
													  })
					}
				}
			}
		}
	}

	Row {
		anchors.horizontalCenter: parent.horizontalCenter
		spacing: 5
		visible: (game && game.hostMode == ConquestGame.ModeGuest) || !_groupBoxWorld.enabled

		Qaterial.BusyIndicator {
			anchors.verticalCenter: parent.verticalCenter
		}

		Qaterial.LabelBody1 {
			anchors.verticalCenter: parent.verticalCenter
			text: qsTr("Betöltés...")
		}
	}

}

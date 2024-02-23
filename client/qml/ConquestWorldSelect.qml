import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

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
					text: modelData

					leftSourceComponent: Image
					{
						fillMode: Image.PreserveAspectFit
						source: modelData ? "qrc:/conquest/%1/bg.png".arg(modelData) : ""
						sourceSize: Qt.size(width, height)
					}

					width: parent.width
					onClicked: {
						_groupBoxWorld.enabled = false
							game.sendWebSocketMessage({
														  cmd: "play",
														  engine: game.engineId,
														  world: modelData
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

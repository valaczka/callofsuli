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

	Item {
		width: parent.width
		height: root.paddingTop
	}

	Label {
		width: parent.width
		horizontalAlignment: Text.AlignHCenter
		leftPadding: Math.max(20 * Qaterial.Style.pixelSizeRatio, Client.safeMarginLeft)
		rightPadding: Math.max(20 * Qaterial.Style.pixelSizeRatio, Client.safeMarginRight)
		font.family: "HVD Peace"
		font.pixelSize: Qaterial.Style.textTheme.headline3.pixelSize
		text: game ? game.missionName() : ""

		wrapMode: Text.Wrap
		bottomPadding: 10 * Qaterial.Style.pixelSizeRatio
	}

	Qaterial.LabelBody1 {
		anchors.horizontalCenter: parent.horizontalCenter
		horizontalAlignment: Text.AlignHCenter
		text: game ? qsTr("Level %1").arg(game.missionLevel()) : ""
		bottomPadding: 20 * Qaterial.Style.pixelSizeRatio
	}

	Qaterial.GroupBox {
		title: qsTr("Csatlakozás meglévő csapathoz")

		width: groupWidth

		anchors.horizontalCenter: parent.horizontalCenter
		inlineTitle: true

		enabled: game && game.engineId === -1
		visible: game && game.engineId === -1 && !_btnNewGroup.aboutToCreate

		Column {
			width: parent.width

			QButton {
				icon.source: Qaterial.Icons.refresh
				text: qsTr("Frissítés")
				anchors.horizontalCenter: parent.horizontalCenter
				onClicked: game.getEngineList()
				enabled: game

				Timer {
					running: game && game.engineId === -1 &&
							 (game.config.gameState === ConquestConfig.StateConnect || game.config.gameState === ConquestConfig.StateInvalid)
					triggeredOnStart: true
					interval: 2000
					repeat: true
					onTriggered: parent.clicked()
				}
			}

			Repeater {
				model: game ? game.engineModel : null

				delegate: Qaterial.ItemDelegate {
					width: parent.width
					anchors.horizontalCenter: parent.horizontalCenter
					text: owner

					icon.source: Qaterial.Icons.account

					onClicked: {
						game.sendWebSocketMessage({
													  cmd: "connect",
													  engine: engineId
												  })
					}
				}
			}
		}
	}


	Qaterial.GroupBox {
		title: qsTr("Csapattagok")

		width: groupWidth

		anchors.horizontalCenter: parent.horizontalCenter
		inlineTitle: true

		enabled: game && game.engineId !== -1
		visible: game && game.engineId !== -1

		Column {
			width: parent.width

			Repeater {
				model: game ? game.maxPlayersCount : null

				delegate: Qaterial.LoaderItemDelegate {
					readonly property var _player: game && game.playersModel.count > index ? game.playersModel.get(index) : null

					width: parent.width
					anchors.horizontalCenter: parent.horizontalCenter
					text: _player ? _player.fullNickName +
									(game && game.config.userHost === _player.username ? qsTr(" (host)") : "")
								  : qsTr("-- szabad hely --")

					highlighted: _player && Client.server && _player.username === Client.server.user.username

					textColor: _player ? Qaterial.Style.iconColor() : Qaterial.Style.disabledTextColor()

					leftSourceComponent: Qaterial.Icon
					{
						size: 32 * Qaterial.Style.pixelSizeRatio
						icon: _player ? "qrc:/character/%1/thumbnail.png".arg(_player.character) : Qaterial.Icons.accountQuestionOutline
						sourceSize: Qt.size(width, height)
						color: _player ? "transparent" : Qaterial.Style.disabledTextColor()
					}
				}
			}
		}
	}

	QLabelInformative {
		visible: _groupBoxCharacter.visible
		text: qsTr("Válaszd ki a karakteredet:")
		bottomPadding: 10 * Qaterial.Style.pixelSizeRatio
		topPadding: 20 * Qaterial.Style.pixelSizeRatio
	}

	Qaterial.GroupBox {
		id: _groupBoxCharacter
		title: qsTr("Karakter")

		width: groupWidth

		anchors.horizontalCenter: parent.horizontalCenter
		inlineTitle: true

		enabled: game
		visible: game && game.playerId === -1 && (game.engineId !== -1 || _btnNewGroup.aboutToCreate)

		Column {
			width: parent.width

			Repeater {
				id: _repeaterCharacter

				delegate: Qaterial.LoaderItemDelegate {
					text: modelData ? modelData.name : ""

					leftSourceComponent: Image
					{
						fillMode: Image.PreserveAspectFit
						source: modelData ? "qrc:/character/%1/thumbnail.png".arg(modelData.id) : ""
						sourceSize: Qt.size(width, height)
					}

					enabled: {
						if (!game || !modelData)
							return false

						for (let i=0; i<game.playersModel.count; ++i)
							if (game.playersModel.get(i).character === modelData.id)
								return false

						return true
					}

					width: parent.width
					onClicked: {
						if (game.engineId === -1) {
							game.gameCreate(modelData.id)
						} else {
							game.sendWebSocketMessage({
														  cmd: "enroll",
														  engine: game.engineId,
														  character: modelData.id
													  })
						}
					}
				}
			}
		}

		Component.onCompleted: {
			let list = Object.keys(Client.availableCharacters())

			let model = []

			for (let i=0; i<list.length; ++i) {
				let key = list[i]
				model.push({
							   id: key,
							   name: Client.availableCharacters()[key].name
						   })
			}

			_repeaterCharacter.model = model
		}
	}


	QDashboardGrid {
		anchors.horizontalCenter: parent.horizontalCenter

		QDashboardButton {
			id: _btnNewGroup

			property bool aboutToCreate: false

			icon.source: Qaterial.Icons.accountMultiplePlus
			text: qsTr("Új csapat")
			visible: game && game.engineId === -1 && !aboutToCreate
			onClicked: aboutToCreate = true
		}

		QDashboardButton {
			icon.source: Qaterial.Icons.exitToApp
			text: qsTr("Csapat elhagyása")
			visible: game && game.engineId !== -1
			onClicked: game.sendWebSocketMessage({
										  cmd: "leave",
										  engine: game.engineId
									  })
			bgColor: Qaterial.Colors.red500
			textColor: Qaterial.Colors.white
		}

		QDashboardButton {
			icon.source: Qaterial.Icons.play
			text: qsTr("Play")
			visible: game && game.hostMode == ConquestGame.ModeHost
			enabled: game && game.hostMode == ConquestGame.ModeHost && game.engineId > 0 && game.playersModel.count > 1
			onClicked: game.sendWebSocketMessage({"cmd": "start", "engine": game.engineId})
			bgColor: Qaterial.Colors.green500
			textColor: Qaterial.Colors.white
		}
	}


	Connections {
		target: game

		function onEngineIdChanged() {
			if (game.engineId === -1)
				_btnNewGroup.aboutToCreate = false
		}
	}
}

import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import CallOfSuli
import SortFilterProxyModel
import Qt5Compat.GraphicalEffects
import "./QaterialHelper" as Qaterial


Rectangle {
	id: root

	property ConquestGame game: null
	property bool pushMapDown: false


	color: Qaterial.Colors.black

	property bool _fitToScreen: false

	signal animationUpReady()
	signal animationDownReady()

	Flickable {
		id: _flick
		contentWidth: _container.width
		contentHeight: _container.height

		anchors.fill: parent

		boundsBehavior: Flickable.DragAndOvershootBounds
		flickableDirection: Flickable.HorizontalAndVerticalFlick

		Item {
			id: _container
			width: Math.max(_scene.width * _scene.scale, _flick.width)
			height: Math.max(_scene.height * _scene.scale, _flick.height)

			ConquestSceneMap {
				id: _scene
				anchors.horizontalCenter: parent.horizontalCenter
				anchors.verticalCenter: parent.verticalCenter
				game: root.game
				flickable: _flick

				onWidthChanged: fitToScreen()
				onHeightChanged: fitToScreen()
				onZoomPerformed: _fitToScreen = false
			}

			PinchHandler {
				target: _scene
				enabled: !pushMapDown
				persistentTranslation: Qt.point(0,0)

				scaleAxis.enabled: true
				scaleAxis.minimum: 0.3
				scaleAxis.maximum: 5.0

				onScaleChanged: _fitToScreen = false
				onRotationChanged: _fitToScreen = false
			}
		}
	}

	Row {
		id: _playerRow

		anchors.left: parent.left
		anchors.leftMargin: Math.max(5 * Qaterial.Style.pixelSizeRatio, Client.safeMarginLeft)
		anchors.top: parent.top
		anchors.topMargin: Math.max(5 * Qaterial.Style.pixelSizeRatio, Client.safeMarginTop)

		spacing: 5 * Qaterial.Style.pixelSizeRatio

		readonly property real _itemWidth: Math.min((root.width -x -Client.safeMarginRight)/
													Math.max((game ? game.playersModel.count : 1), 1),
													250 * Qaterial.Style.pixelSizeRatio)


		GameButton {
			size: 25

			anchors.verticalCenter: parent.verticalCenter

			color: Qaterial.Colors.red800
			border.color: "white"
			border.width: 1

			fontImage.icon: Qaterial.Icons.close
			fontImage.color: "white"
			fontImageScale: 0.7

			onClicked: {
				Client.stackPop()
			}
		}

		Repeater {
			model: SortFilterProxyModel {
				sourceModel: game ? game.playersModel : null

				sorters: [
					FilterSorter {
						filters: ValueFilter {
							enabled: Client.server
							roleName: "username"
							value: Client.server ? Client.server.user.username : ""
						}
						priority: 2
					},
					RoleSorter {
						roleName: "playerId"
						priority: 1
					}
				]

			}
			delegate: ConquestPlayerItem {
				id: _cpItem
				width: _playerRow._itemWidth - _playerRow.spacing
				anchors.verticalCenter: parent.verticalCenter

				visible: game && game.config.gameState !== ConquestConfig.StateFinished

				username: model.username
				character: model.character
				fullNickName: model.fullNickName
				xp: model.xp
				hp: model.hp
				streak: model.streak
				playerId: model.playerId
				online: model.online
				game: root.game
				targetFighter1: _battleRow.itemFighter1
				targetFighter2: _battleRow.itemFighter2
			}
		}
	}



	GameButton {
		id: _setttingsButton
		size: 30

		anchors.right: parent.right
		anchors.rightMargin: Math.max(5 * Qaterial.Style.pixelSizeRatio, Client.safeMarginRight)
		anchors.top: (_playerRow.x+_playerRow.width >= x) ? _playerRow.bottom : parent.top
		anchors.topMargin: (_playerRow.x+_playerRow.width >= x) ?
							   5 * Qaterial.Style.pixelSizeRatio :
							   Math.max(5 * Qaterial.Style.pixelSizeRatio, Client.safeMarginTop)


		color: Qaterial.Colors.white
		border.color: fontImage.color
		border.width: 2

		fontImage.icon: Qaterial.Icons.cog
		fontImage.color: Qaterial.Colors.blueGray600
		fontImageScale: 0.7

		onClicked: {
			Qaterial.DialogManager.openFromComponent(_settingsDialog)
		}
	}

	Component {
		id: _settingsDialog
		Qaterial.ModalDialog
		{

			dialogImplicitWidth: 400 * Qaterial.Style.pixelSizeRatio

			horizontalPadding: 0
			bottomPadding: 1
			drawSeparator: true

			title: qsTr("Beállítások")

			standardButtons: DialogButtonBox.Close
			contentItem: QScrollable {
				leftPadding: 10 * Qaterial.Style.pixelSizeRatio
				rightPadding: 10 * Qaterial.Style.pixelSizeRatio
				topPadding: 5 * Qaterial.Style.pixelSizeRatio
				bottomPadding: 10 * Qaterial.Style.pixelSizeRatio

				SettingsSound {
					width: parent.width
				}
			}
		}
	}

	ConquestBattleInfo {
		id: _battleRow
		anchors.centerIn: parent
		game: root.game
	}

	GameButton {
		id: _btnZoomFit
		size: 50

		anchors.left: parent.left
		anchors.bottom: parent.bottom

		anchors.leftMargin: Math.max(10, Client.safeMarginLeft)
		anchors.bottomMargin: Math.max(10, Client.safeMarginBottom)

		visible: game && !_fitToScreen

		enabled: !_fitToScreen

		color: "transparent"
		border.color: fontImage.color
		border.width: 1

		opacity: 0.7

		fontImage.icon: Qaterial.Icons.fitToScreen
		fontImage.color: "white"
		fontImageScale: 0.6
		//fontImage.anchors.horizontalCenterOffset: -2

		onClicked: {
			fitToScreen()
		}
	}


	ConquestTurnChart {
		anchors.right: parent.right
		anchors.bottom: parent.bottom
		anchors.bottomMargin: Math.max(10 * Qaterial.Style.pixelSizeRatio, Client.safeMarginBottom)
		anchors.rightMargin: Math.max(10 * Qaterial.Style.pixelSizeRatio, Client.safeMarginRight)
		game: root.game

		visible: game && game.config.gameState !== ConquestConfig.StateFinished
	}


	ConquestFinishInfo {
		anchors.centerIn: parent
		width: Math.min(implicitWidth, parent.width
						- Math.max(5 * Qaterial.Style.pixelSizeRatio, Client.safeMarginLeft)
						- Math.max(5 * Qaterial.Style.pixelSizeRatio, Client.safeMarginRight))
		height: Math.min(implicitHeight, parent.height
						 - Math.max(5 * Qaterial.Style.pixelSizeRatio, Client.safeMarginTop)
						 - Math.max(5 * Qaterial.Style.pixelSizeRatio, Client.safeMarginBottom))
		game: root.game

		visible: game && game.config.gameState === ConquestConfig.StateFinished
	}



	GameMessageList {
		id: _messageList

		anchors.horizontalCenter: parent.horizontalCenter

		y: parent.height*0.25

		width: Math.min(450*Qaterial.Style.pixelSizeRatio, parent.width-Client.safeMarginLeft-Client.safeMarginRight)

		//visible: !gameScene.zoomOverview && itemsVisible
	}

	GameQuestionAction {
		id: _question

		anchors.fill: parent
		anchors.topMargin: _playerRow.height
		anchors.bottomMargin: Client.safeMarginBottom
		anchors.leftMargin: Client.safeMarginLeft
		anchors.rightMargin: Client.safeMarginRight

		game: root.game
	}

	Qaterial.Icon {
		id: _iconBlock
		color: Qaterial.Colors.red500
		anchors.centerIn: _question
		visible: false
		size: 150 * Qaterial.Style.pixelSizeRatio
		icon: Qaterial.Icons.blockHelper
	}

	DropShadow {
		anchors.fill: _iconBlock
		source: _iconBlock
		visible: _question.permanentDisabled && _question.state === "started"
		samples: 13
		radius: 6
		color: Qaterial.Colors.black
		horizontalOffset: 6
		verticalOffset: 6
	}



	ConquestProgressBar {
		anchors.bottom: parent.bottom
		width: parent.width
		game: root.game
	}


	GamePainHud {
		id: _painhudImage
		anchors.fill: parent
		z: 10

		property int _oldHP: -1
		readonly property int hp: game ? game.hp : -1

		onHpChanged: {
			if (hp < _oldHP)
				play()
			else if (hp > _oldHP && _oldHP != -1)
				_messageList.message("+%1 HP".arg(hp-_oldHP), Qaterial.Colors.red400)

			_oldHP = hp
		}
	}


	property int _oldXP: -1
	readonly property int xp: game ? game.xp : -1

	onXpChanged: {
		if (_oldXP != -1) {
			let d = xp-_oldXP

			if (d > 0) {
				_messageList.message("+%1 XP".arg(d), Qaterial.Colors.green400)
			} else {
				_messageList.message("%1 XP".arg(d), Qaterial.Colors.red400)
			}
		}

		_oldXP = xp
	}




	states: [
		State {
			name: "mapDown"

			PropertyChanges {
				target: _scene
				scale: Math.min(_flick.width/_scene.width, _flick.height/_scene.height, 1.0) * 0.75
				rotation: 0
				restoreEntryValues: false
			}

			AnchorChanges {
				target: _scene
				anchors.verticalCenter: undefined
				anchors.bottom: _container.bottom
			}
		},

		State {
			name: "fit"

			PropertyChanges {
				target: _scene
				scale: Math.min(_flick.width/_scene.width, _flick.height/_scene.height, 1.0)
				rotation: 0
				restoreEntryValues: false
			}

			AnchorChanges {
				target: _scene
				anchors.verticalCenter: _container.verticalCenter
				anchors.bottom: undefined
			}
		}
	]


	transitions: [
		Transition {
			from: "*"
			to: "mapDown"

			SequentialAnimation {
				ParallelAnimation {
					AnchorAnimation {
						duration: 500
					}

					PropertyAnimation {
						target: _scene
						property: "scale"
						duration: 500
					}

					PropertyAnimation {
						target: _scene
						property: "rotation"
						duration: 500
					}

					PropertyAnimation {
						target: _scene.transformRotation
						property: "angle"
						duration: 500
						to: 75
					}

					PropertyAnimation {
						target: _scene.transformTranslate
						property: "y"
						duration: 500
					}
				}
				ScriptAction {
					script: root.animationDownReady()
				}
			}
		},
		Transition {
			from: "*"
			to: "fit"

			SequentialAnimation {

				ParallelAnimation {
					AnchorAnimation {
						duration: 500
					}

					PropertyAnimation {
						target: _scene
						properties: "scale"
						duration: 500
					}

					PropertyAnimation {
						target: _scene
						properties: "rotation"
						duration: 500
					}

					PropertyAnimation {
						target: _scene.transformRotation
						property: "angle"
						duration: 500
						to: 0
					}

					PropertyAnimation {
						target: _scene.transformTranslate
						property: "y"
						duration: 500
						to: 0
					}
				}
				ScriptAction {
					script: {
						_scene._prevScale = _scene.scale
						_fitToScreen = true
						_flick.returnToBounds()
						root.animationUpReady()
					}
				}
			}
		}
	]


	Connections {
		target: game

		function onMapDownRequest() {
			state = ""
			state = "mapDown"
		}

		function onMapUpRequest() {
			state = ""
			state = "fit"
		}

		/*function onHurt() {
			_painhudImage.play()
		}*/
	}

	function fitToScreen() {
		if (!game)
			return

		state = ""
		state = "fit"
	}

	onWidthChanged: if (_fitToScreen) fitToScreen()
	onHeightChanged: if (_fitToScreen) fitToScreen()


	Component.onCompleted: {
		if (game) {
			game.messageList = _messageList
			game.defaultMessageColor = Qaterial.Style.iconColor()
		}
	}

	Component.onDestruction: {
		if (game)
			game.messageList = null
	}

}

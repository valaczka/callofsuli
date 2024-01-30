import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import SortFilterProxyModel
import Qt.labs.animation
import CallOfSuli
import Qaterial as Qaterial
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


			Item {
				id: _scene

				anchors.horizontalCenter: parent.horizontalCenter
				anchors.verticalCenter: parent.verticalCenter
				transformOrigin: Item.Center

				property real _prevScale: 1.0

				onScaleChanged: {
					if ((width * scale) > _flick.width) {
						var xoff = (_flick.width / 2 + _flick.contentX) * scale / _prevScale;
						_flick.contentX = xoff - _flick.width / 2
					}
					if ((height * scale) > _flick.height) {
						var yoff = (_flick.height / 2 + _flick.contentY) * scale / _prevScale;
						_flick.contentY = yoff - _flick.height / 2
					}
					_prevScale=scale;
				}

				BoundaryRule on scale {
					minimum: 0.3
					maximum: 5.0
				}


				width: game ? game.worldSize.width : implicitWidth
				height: game ? game.worldSize.height : implicitHeight

				onWidthChanged: fitToScreen()
				onHeightChanged: fitToScreen()

				Image {
					source: game && game.config.world.name != "" ? "qrc:/conquest/"+game.config.world.name+"/bg.png" : ""
					anchors.fill: parent
					fillMode: Image.PreserveAspectFit
				}

				Repeater {
					model: game ? game.landDataList : null

					delegate: ConquestLand {
						landData: model.qtObject
					}
				}

				Image {
					source: game && game.config.world.name != "" ? "qrc:/conquest/"+game.config.world.name+"/over.png" : ""
					anchors.fill: parent
					fillMode: Image.PreserveAspectFit
				}


				transform: [
					Rotation {
						id: _sceneRotation
						origin.x: _scene.width/2
						origin.y: _scene.height/2
						axis.x: 1
						axis.y: 0
						axis.z: 0
					},
					Translate {
						id: _sceneTranslate
					}
				]


				/*Desaturate {
				id: gameSaturate

				anchors.fill: gameScene
				source: gameScene

				opacity: 0.0
				visible: desaturation

				desaturation: 1.0

				Behavior on opacity { NumberAnimation { duration: 750 } }
			}*/


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
		anchors.top: parent.top

		spacing: 5

		Repeater {
			model: SortFilterProxyModel {
				sourceModel: game ? game.playersModel : null

				sorters: [
					FilterSorter {
						filters: ValueFilter {
							roleName: "username"
							value: Client.server.user.username
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
				username: model.username
				theme: model.theme
				xp: model.xp
				playerId: model.playerId

				readonly property bool _isFighter1: game && game.fighter1.playerId === _cpItem.playerId &&
													game.currentStage == ConquestTurn.StageBattle

				readonly property bool _isFighter2: game && game.fighter2.playerId === _cpItem.playerId &&
													game.currentStage == ConquestTurn.StageBattle

				states: [
					State {
						name: "nofight"
						when: !_cpItem._isFighter1 && !_cpItem._isFighter2
						ParentChange {
							target: _cpItem.playerItem
							parent: _cpItem.placeholder
							x: (_cpItem.placeholder.width- _cpItem.playerItem.width)/2
							y: (_cpItem.placeholder.height- _cpItem.playerItem.height)/2
						}
					},
					State {
						name: "reparented1"
						when: _cpItem._isFighter1
						ParentChange {
							target: _cpItem.playerItem
							parent: _fighter1
							x: (_fighter1.width- _cpItem.playerItem.width)/2
							y: (_fighter1.height- _cpItem.playerItem.height)/2
						}
					},
					State {
						name: "reparented2"
						when: _cpItem._isFighter2
						ParentChange {
							target: _cpItem.playerItem
							parent: _fighter2
							x: (_fighter2.width- _cpItem.playerItem.width)/2
							y: (_fighter2.height- _cpItem.playerItem.height)/2
						}
					}
				]

				transitions: Transition {
					ParentAnimation {
						NumberAnimation {
							properties: "x,y"
							duration: 350
						}
					}
				}
			}
		}
	}

	Row {
		id: _battleRow

		anchors.centerIn: parent

		visible: game && game.fighter1.playerId !== -1 && game.fighter2.playerId !== -1

		spacing: 5

		Rectangle {
			id: _fighter1
			width: 120
			height: 120
			color: "white"
		}

		Rectangle {
			id: _fighter2
			width: 120
			height: 120
			color: "white"
		}
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

	WheelHandler {
		target: _scene
		acceptedModifiers: Qt.ControlModifier
		property: "scale"
		onWheel: _fitToScreen = false
	}



	states: [
		State {
			name: "mapDown"
			when: pushMapDown

			PropertyChanges {
				target: _scene
				scale: Math.min(_flick.width/_scene.width, _flick.height/_scene.height, 1.0) * 0.75
				rotation: 0
			}

			AnchorChanges {
				target: _scene
				anchors.verticalCenter: undefined
				anchors.bottom: _container.bottom
			}

			PropertyChanges {
				target: _sceneTranslate
				y: {
					let s = Math.min(_flick.width/_scene.width, _flick.height/_scene.height, 1.0) * 0.75
					let h = _scene.height*s

					return Math.max(0, _scene.height-h)*0.65
				}
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
						duration: 350
					}

					PropertyAnimation {
						target: _scene
						property: "scale"
						duration: 350
					}

					PropertyAnimation {
						target: _scene
						property: "rotation"
						duration: 350
					}

					PropertyAnimation {
						target: _sceneRotation
						property: "angle"
						duration: 350
						to: 75
					}

					PropertyAnimation {
						target: _sceneTranslate
						property: "y"
						duration: 350
					}
				}
				ScriptAction {
					script: root.animationDownReady()
				}
			}
		},
		Transition {
			from: "mapDown"
			to: "*"

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
						target: _sceneRotation
						property: "angle"
						duration: 500
						to: 0
					}

					PropertyAnimation {
						target: _sceneTranslate
						property: "y"
						duration: 500
					}
				}
				ScriptAction {
					script: root.animationUpReady()
				}
			}
		}
	]


	Connections {
		target: game

		function onMapDownRequest() {
			pushMapDown = true
		}

		function onMapUpRequest() {
			pushMapDown = false
		}
	}

	function fitToScreen() {
		if (!game)
			return

		let s = Math.min(_flick.width/_scene.width, _flick.height/_scene.height, 1.0)
		_scene.scale = s
		_scene._prevScale = s
		_fitToScreen = true
		_flick.returnToBounds()
	}

	onWidthChanged: if (_fitToScreen) fitToScreen()
	onHeightChanged: if (_fitToScreen) fitToScreen()

}

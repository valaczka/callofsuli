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

	property alias playerRowLeftMargin: _playerRow.anchors.leftMargin


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
		anchors.top: parent.top
		anchors.topMargin: 2

		spacing: 5 * Qaterial.Style.pixelSizeRatio

		readonly property real _itemWidth: Math.min((parent.width -x -Client.Utils.safeMarginRight)/
													(game ? game.playersModel.count : 1),
													300 * Qaterial.Style.pixelSizeRatio)

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
				width: _playerRow._itemWidth - _playerRow.spacing
				username: model.username
				theme: model.theme
				xp: model.xp
				playerId: model.playerId
				game: root.game
				targetFighter1: _battleRow.itemFighter1
				targetFighter2: _battleRow.itemFighter2
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

	WheelHandler {
		target: _scene
		acceptedModifiers: Qt.ControlModifier
		property: "scale"
		onWheel: _fitToScreen = false
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
	}

	function fitToScreen() {
		if (!game)
			return

		state = ""
		state = "fit"
	}

	onWidthChanged: if (_fitToScreen) fitToScreen()
	onHeightChanged: if (_fitToScreen) fitToScreen()

}

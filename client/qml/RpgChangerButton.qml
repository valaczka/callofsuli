import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


Item {
	id: root

	width: mainButton.width
	height: mainButton.height

	property RpgChangerImpl changer: null

	property bool quickActionsEnabled: true

	property real iconSize: 40//Qt.platform.os === "android" || Qt.platform.os === "ios" ? 40 : 30
	property real menuRadius: 68
	property real minSwipeDistance: 24

	property int holdDelay: Qt.styleHints.mousePressAndHoldInterval

	property var actions: [
		{
			id: "weapon",
			color: Qaterial.Colors.red700,
			icon: Qaterial.Icons.bullet,
			angle: 180
		},
		{
			id: "defender",
			color: Qaterial.Colors.green700,
			icon: Qaterial.Icons.chessRook,
			angle: -90
		},
		{
			id: "utility",
			color: Qaterial.Colors.amber700,
			icon: Qaterial.Icons.compassRose,
			angle: 90
		}
	]


	enabled: changer

	property bool menuVisible: false
	property bool pointerDown: false
	property int selectedIndex: -1

	property real pressX: 0
	property real pressY: 0
	property real currentX: 0
	property real currentY: 0

	function degToRad(deg) {
		return deg * Math.PI / 180.0
	}

	function actionCenter(index) {
		const action = actions[index]
		const a = degToRad(action.angle)

		return Qt.point(
					root.width / 2 + Math.cos(a) * menuRadius,
					root.height / 2 + Math.sin(a) * menuRadius
					)
	}

	function distance(x1, y1, x2, y2) {
		const dx = x1 - x2
		const dy = y1 - y2
		return Math.sqrt(dx * dx + dy * dy)
	}

	function swipeDistance() {
		return distance(pressX, pressY, currentX, currentY)
	}

	function updateSelection(x, y) {
		currentX = x
		currentY = y

		if (!menuVisible) {
			selectedIndex = -1
			return
		}

		if (swipeDistance() < minSwipeDistance) {
			selectedIndex = -1
			return
		}

		let bestIndex = -1
		let bestDistance = 999999

		for (let i = 0; i < actions.length; ++i) {
			const p = actionCenter(i)
			const d = distance(x, y, p.x, p.y)

			if (d < bestDistance) {
				bestDistance = d
				bestIndex = i
			}
		}

		selectedIndex = bestIndex
	}

	function hideMenu() {
		menuVisible = false
		selectedIndex = -1
	}

	Rectangle {
		id: dimCircle

		anchors.centerIn: parent
		width: root.menuRadius * 2.2 + root.iconSize
		height: width
		radius: width / 2

		visible: opacity > 0
		opacity: root.menuVisible ? 0.22 : 0.0
		color: "#000000"

		Behavior on opacity {
			NumberAnimation {
				duration: 90
			}
		}
	}

	Repeater {
		id: actionRepeater

		model: root.actions.length


		GameButton {
			id: actionItem

			required property int index

			readonly property var action: root.actions[index]
			readonly property point targetPos: root.actionCenter(index)
			readonly property bool selected: root.selectedIndex === index

			size: root.iconSize

			x: root.width / 2 - width / 2
			y: root.height / 2 - height / 2

			opacity: root.menuVisible ? 1.0 : 0.0
			scale: actionItem.selected ? 1.18 :
										 root.menuVisible ? 1.0 : 0.4
			visible: opacity > 0

			states: State {
				name: "open"
				when: root.menuVisible

				PropertyChanges {
					actionItem.x: actionItem.targetPos.x - actionItem.width / 2
					actionItem.y: actionItem.targetPos.y - actionItem.height / 2
				}
			}

			transitions: Transition {
				NumberAnimation {
					properties: "x,y,opacity"
					duration: 110
					easing.type: Easing.OutCubic
				}
			}

			Behavior on scale {
				NumberAnimation {
					duration: 70
				}
			}

			color: actionItem.selected ? actionItem.action.color : "transparent"
			border.color: fontImage.color
			border.width: actionItem.selected ? 3 : 2

			fontImage.icon: actionItem.action.icon
			fontImage.color: actionItem.selected ? Qaterial.Colors.white : actionItem.action.color
			fontImageScale: 0.7
		}


		/*Rectangle {
				anchors.fill: parent
				radius: width / 2
				color: actionItem.selected
					   ? root.iconSelectedColor
					   : root.iconColor

				border.width: actionItem.selected ? 3 : 1
				border.color: actionItem.selected ? "white" : "#708090"

				scale: actionItem.selected ? 1.18 : 1.0

				Behavior on scale {
					NumberAnimation {
						duration: 70
					}
				}

				Behavior on color {
					ColorAnimation {
						duration: 70
					}
				}
			}*/
	}


	GameButton {
		id: mainButton
		//size: Qt.platform.os === "android" || Qt.platform.os === "ios" ? 40 : 30
		size: 40

		anchors.centerIn: parent

		tap.enabled: false

		color: "transparent"

		border.width: root.menuVisible ? 3 : 2
		border.color: root.menuVisible ? Qaterial.Style.iconColor() : fontImage.color

		fontImage.icon: {
			if (changer.game.controlledPlayer && changer.game.gameItem.usingGamepad) {
				if (changer.game.controlledPlayer.currentJoystickIcon == "")
					return "qrc:/internal/game/target1.svg"
				else
					return changer.game.controlledPlayer.currentJoystickIcon
			} else {
				return Qaterial.Icons.creation
			}
		}

		fontImage.color: root.menuVisible ? Qaterial.Style.iconColor() :
											(changer.game.controlledPlayer && changer.game.gameItem.usingGamepad) ?
												(changer.game.controlledPlayer.joystickMode === RpgPlayer.JoystickModeControl ?
													 Qaterial.Colors.green700 :
													 changer.game.controlledPlayer.joystickMode === RpgPlayer.JoystickModeTarget ?
														 Qaterial.Colors.red700 :
														 changer.game.controlledPlayer.joystickMode === RpgPlayer.JoystickModeUtility ?
															 Qaterial.Colors.amber700 :
															 Qaterial.Colors.white) : Qaterial.Colors.white


		fontImageScale: 0.7


		scale: root.menuVisible ? 0.92 : 1.0

		Behavior on scale {
			NumberAnimation {
				duration: 80
			}
		}
	}


	MouseArea {
		id: mouseArea

		anchors.fill: parent

		acceptedButtons: Qt.LeftButton
		hoverEnabled: true
		preventStealing: true

		pressAndHoldInterval: root.holdDelay

		onPressed: function(mouse) {
			root.pointerDown = true
			root.pressX = mouse.x
			root.pressY = mouse.y
			root.currentX = mouse.x
			root.currentY = mouse.y
			root.selectedIndex = -1
		}

		onPositionChanged: function(mouse) {
			root.updateSelection(mouse.x, mouse.y)
		}

		onPressAndHold: function(mouse) {
			if (!root.quickActionsEnabled)
				return

			root.menuVisible = true
			root.updateSelection(mouse.x, mouse.y)
		}

		onReleased: function(mouse) {
			root.pointerDown = false
			root.currentX = mouse.x
			root.currentY = mouse.y

			if (root.menuVisible) {
				root.updateSelection(mouse.x, mouse.y)

				const idx = root.selectedIndex
				root.hideMenu()

				if (idx >= 0 && idx < root.actions.length) {
					changer.use(root.actions[idx].id)
				}

				return
			}

			root.hideMenu()

			const moved = root.swipeDistance()

			if (moved < root.minSwipeDistance) {
				//root.openSettingsRequested()
				changer.open()
			}
		}

		onCanceled: {
			root.pointerDown = false
			root.hideMenu()
		}
	}
}

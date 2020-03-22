import Bacon2D 1.0
import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import QtGraphicalEffects 1.0

PhysicsEntity {
	id: root
	sleepingAllowed: false
	width: spriteSequence.width
	height: spriteSequence.height
	bodyType: Body.Dynamic
	fixedRotation: true

	gravityScale: 5

	property GameEntityPrivate entityPrivate: null
	property alias spriteSequence: spriteSequence

	property bool facingLeft: false

	property alias hpProgress: hpProgress
	property int hpValue: 0
	property color hpColor: "white"
	property alias hpVisible: hpProgress.visible

	property alias glowColor: glow.color
	property bool glowEnabled: false
	property bool _glowForced: true

	property alias overlayColor: overlay.color
	property bool overlayEnabled: false

	property bool _qrcDataFacingLeft: false

	Connections {
		target: entityPrivate
		function onQrcDataChanged(qrcData) {
			if (qrcData)
				_qrcDataFacingLeft = qrcData.facingLeft
		}
	}

	onEntityPrivateChanged: if (entityPrivate.qrcData)
								_qrcDataFacingLeft = entityPrivate.qrcData.facingLeft

	readonly property bool isInverse: entityPrivate ?
										  ((facingLeft && !_qrcDataFacingLeft) || (!facingLeft && _qrcDataFacingLeft)) :
										  false

	readonly property bool isWalking: Array("walk", "walk2", "walk3", "walk4", "walk5").includes(spriteSequence.currentSprite)
	readonly property bool isRunning: Array("run", "run2", "run3", "run4", "run5", "runend").includes(spriteSequence.currentSprite)
	readonly property bool isFalling: Array("fall", "fall2", "fall3", "fall4", "fall5", "fallend", "falldeath", "falldeath2").includes(spriteSequence.currentSprite)


	onWidthChanged: if (entityPrivate) entityPrivate.updateFixtures(spriteSequence.currentSprite, isInverse)
	onHeightChanged: if (entityPrivate) entityPrivate.updateFixtures(spriteSequence.currentSprite, isInverse)

	onHpValueChanged: {
		if (entityPrivate && entityPrivate.maxHp>0)
			hpProgress.to = entityPrivate.maxHp
		else if (hpValue>hpProgress.to)
			hpProgress.to = hpValue
	}

	ProgressBar {
		id: hpProgress
		visible: false

		width: entityPrivate ? entityPrivate.boundBox.width+6 : 30
		x: entityPrivate ? entityPrivate.boundBox.x-3 : 0
		y: entityPrivate ? entityPrivate.boundBox.y-10 : -5
		height: 2

		from: 0
		to: (entityPrivate && entityPrivate.maxHp>0) ? entityPrivate.maxHp : 0
		value: hpValue

		Behavior on value {
			NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
		}

		background: Rectangle {
			implicitWidth: 200
			implicitHeight: 2
			color: hpColor
			radius: 0
			opacity: 0.3
		}

		contentItem: Item {
			implicitWidth: 200
			implicitHeight: 2

			Rectangle {
				width: hpProgress.visualPosition * parent.width
				height: parent.height
				radius: 0
				color: hpColor
			}
		}
	}


	SpriteSequence {
		id: spriteSequence
		width: 35
		height: 35

		running: entityPrivate && entityPrivate.cosGame.running

		anchors.horizontalCenter: root.horizontalCenter
		anchors.bottom: root.bottom

		onCurrentSpriteChanged: if (entityPrivate) {
									var d = entityPrivate.qrcData.sprites[currentSprite]
									spriteSequence.width = d.frameWidth
									spriteSequence.height = d.frameHeight
									entityPrivate.updateFixtures(currentSprite, isInverse)

									if (Array("dead", "dead2", "dead3", "dead4", "falldeath", "falldeath2", "falldead", "burn", "burndead").includes(currentSprite))
										entityPrivate.isAlive = false
								} else {
									spriteSequence.width = 35
									spriteSequence.height = 35
								}

		transform: Rotation {
			id: rotation
			origin.x: spriteSequence.width/2
			origin.y: spriteSequence.height/2
			axis.x: 0; axis.y: 1; axis.z: 0
			angle: 0
		}

		states: State {
			name: "back"
			PropertyChanges { target: rotation; angle: 180 }
			when: root.isInverse
		}

		transitions: Transition {
			NumberAnimation { target: rotation; property: "angle"; duration: 150 }
		}

		sprites: []

	}


	Glow {
		id: glow
		opacity: glowEnabled || _glowForced ? 1.0 : 0.0
		visible: opacity != 0

		source: spriteSequence
		anchors.fill: spriteSequence

		radius: 4
		samples: 9

		Behavior on opacity {
			NumberAnimation { duration: 200 }
		}

		transform: rotation
	}


	ColorOverlay {
		id: overlay
		source: spriteSequence
		anchors.fill: spriteSequence
		opacity: overlayEnabled ? 1.0 : 0.0
		visible: opacity != 0

		Behavior on opacity {
			NumberAnimation { duration: 300 }
		}

		transform: rotation
	}


	Component {
		id: spriteComponent

		Sprite {  }
	}


	Connections {
		target: entityPrivate ? entityPrivate : null
		function onDie() {
			dieAnimation.start()
		}
	}

	SequentialAnimation {
		id: dieAnimation
		running: false
		PropertyAnimation {
			target: root
			property: "opacity"
			to: 0
			duration: 150
		}
		ScriptAction {
			script: root.destroy()
		}
	}



	Timer {
		id: glowForcedDelay
		interval: 1500
		repeat: false
		triggeredOnStart: false
		onTriggered: _glowForced = false
	}

	Component.onCompleted: glowForcedDelay.start()



	function loadSprites() {
		if (!entityPrivate)
			return

		var qd = entityPrivate.qrcData.sprites

		if (!qd)
			return

		var sprites = Object.keys(qd)

		for (var i=-1; i<sprites.length; i++) {
			var sname = (i === -1) ? "idle" : sprites[i]

			if (sname === "idle" && i !== -1)
				continue

			var sdata = qd[sname]
			if (!sdata)
				continue

			var obj = spriteComponent.createObject(spriteSequence)
			obj.name = sname
			obj.source = entityPrivate.qrcDirName+"/"+sdata.source
			obj.frameCount = sdata.frameCount ? sdata.frameCount : 1
			obj.frameWidth = sdata.frameWidth ? sdata.frameWidth : 1
			obj.frameHeight = sdata.frameHeight ? sdata.frameHeight : 1
			obj.frameX = sdata.frameX ? sdata.frameX : 0
			obj.frameY = sdata.frameY ? sdata.frameY : 0
			obj.frameDuration = sdata.frameDuration ? sdata.frameDuration : 100
			obj.frameDurationVariation = sdata.frameDurationVariation ? sdata.frameDurationVariation : 0
			obj.to = sdata.to ? sdata.to : {}
			spriteSequence.sprites.push(obj)
		}
	}


}


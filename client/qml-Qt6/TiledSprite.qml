import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli

AnimatedSprite {
	id: root

	property TiledObjectImpl baseObject: null
	property int baseLoops: 0
	property bool waitForEnd: false

	anchors.fill: parent
	loops: waitForEnd ? 1 : baseLoops > 0 ? baseLoops : AnimatedSprite.Infinite
	finishBehavior: AnimatedSprite.FinishAtFinalFrame

	visible: false

	readonly property bool _active: visible && baseObject && baseObject.inVisibleArea

	interpolate: false
	running: false

	on_ActiveChanged: {
		if (_active)
			start()
		else
			stop()
	}

	/*Binding on source {
		when: !_active
		value: ""
	}*/
}

/*
import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli


Loader {
	id: root

	required property int frameCount
	required property int frameDuration
	required property int frameHeight
	required property int frameWidth
	required property int frameX
	required property int frameY
	required property int baseLoops
	required property url baseSource

	property bool waitForEnd: false
	property TiledObjectImpl baseObject: null

	signal finished()

	//asynchronous: true

	//active: baseObject && baseObject.inVisibleArea && visible
	sourceComponent: _cmp //baseObject && baseObject.inVisibleArea && visible ? _cmp : undefined

	onSourceComponentChanged: {
		if (sourceComponent === undefined)
			finished()
	}

	onLoaded: item.start()

	Component {
		id: _cmp

		AnimatedSprite {
			anchors.fill: parent
			loops: waitForEnd ? 1 : baseLoops > 0 ? baseLoops : AnimatedSprite.Infinite
			finishBehavior: AnimatedSprite.FinishAtFinalFrame
			source: baseSource //baseObject && baseObject.inVisibleArea ? baseSource : ""
			frameCount: root.frameCount
			frameX: root.frameX
			frameY: root.frameY
			frameWidth: root.frameWidth
			frameHeight: root.frameHeight
			frameDuration: root.frameDuration

			onFinished: root.finished()
		}
	}

}
*/

import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
	id: root

	property int fortress: 3
	property int _current: 3

	implicitWidth: Math.max(_imgLast.implicitWidth, _imgFirst.implicitWidth)
	implicitHeight:  Math.max(_imgLast.implicitHeight, _imgFirst.implicitHeight)

	Image {
		id: _imgLast

		anchors.fill: parent

		property int tower: _current

		fillMode: Image.PreserveAspectFit

		source: tower >= 0 ? "qrc:/internal/game/tower"+tower+".png" : ""

		visible: opacity

		opacity: 0.0
	}

	Image {
		id: _imgFirst

		anchors.fill: parent

		property int tower: _current

		fillMode: Image.PreserveAspectFit

		source: tower >= 0 ? "qrc:/internal/game/tower"+tower+".png" : ""

		visible: opacity
	}

	SequentialAnimation {
		id: _animShow

		PauseAnimation {
			duration: 450
		}

		PropertyAnimation {
			target: _imgFirst
			property: "opacity"
			from: 0.0
			to: 1.0
			duration: 750
			easing.type: Easing.OutQuad
		}

		ScriptAction {
			script: {
				_imgLast.opacity = 0.0
			}
		}
	}

	SequentialAnimation {
		id: _animHide

		ScriptAction {
			script: {
				_imgLast.opacity = 1.0
			}
		}

		PauseAnimation {
			duration: 450
		}

		PropertyAnimation {
			target: _imgFirst
			property: "opacity"
			from: 1.0
			to: 0.0
			duration: 750
			easing.type: Easing.OutQuad
		}
	}


	onFortressChanged: {
		if (fortress > _current) {
			_imgLast.tower = _current
			_imgLast.opacity = 1.0
			_imgFirst.opacity = 0.0
			_imgFirst.tower = fortress
			_animShow.start()
		} else {
			_imgLast.tower = fortress
			_imgLast.opacity = 1.0
			_imgFirst.opacity = 1.0
			_imgFirst.tower = _current
			_animHide.start()
		}

		_current = fortress
	}

}

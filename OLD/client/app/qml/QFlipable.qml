import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style"


Flipable {
	id: flipable

	implicitHeight: implicitWidth
	implicitWidth: fontSize+8

	property real fontSize: 24

	property string frontIcon: CosStyle.iconUnchecked
	property string backIcon: CosStyle.iconChecked

	property color color: CosStyle.colorAccent
	property color backColor: color

	property bool flipped: false

	property bool contentVisible: true
	property alias mouseArea: area

	front: QFontImage {
		visible: contentVisible
		anchors.fill: parent
		icon: flipable.frontIcon
		size: flipable.fontSize
		color: area.containsMouse ? Qt.lighter(flipable.color, 1.3) : flipable.color
	}

	back: QFontImage {
		visible: contentVisible
		anchors.fill: parent
		icon: flipable.backIcon
		size: flipable.fontSize
		color: area.containsMouse ? Qt.lighter(flipable.backColor, 1.3) : flipable.backColor
	}


	transform: Rotation {
		id: rotation
		origin.x: flipable.width/2
		origin.y: flipable.height/2
		axis.x: 0; axis.y: 1; axis.z: 0
		angle: 0
	}

	states: State {
		name: "back"
		PropertyChanges { target: rotation; angle: 180 }
		when: flipped
	}

	transitions: Transition {
		NumberAnimation { target: rotation;
			property: "angle";
			duration: 225
			easing.type: Easing.InOutCubic
		}
	}

	MouseArea {
		id: area
		anchors.fill: parent
		enabled: true
		acceptedButtons: Qt.LeftButton
		hoverEnabled: true
	}
}

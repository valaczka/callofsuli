import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

Item {
	id: root

	property int horizontalAlignment: Qt.AlignHCenter
	property int verticalAlignment: Qt.AlignVCenter

	property alias contentOpacity: _loader.opacity

	property real fixedWidth: 0
	property real fixedHeight: 0

	property real widthRatio: -1
	property real heightRatio: -1

	property real rectangleRadius: 0

	property alias contentComponent: _loader.sourceComponent

	readonly property alias rectangleComponent: _cmpRectangle
	readonly property alias ellipseComponent: _cmpEllipse

	implicitWidth: fixedWidth > 0 ? fixedWidth : 24
	implicitHeight: fixedHeight > 0 ? fixedHeight : 24

	Loader {
		id: _loader

		sourceComponent: _cmpRectangle

		width: fixedWidth>0 ? fixedWidth : parent.width*widthRatio
		height: fixedHeight>0 ? fixedHeight : parent.height*heightRatio

		property double widthRatio: 1.0
		property double heightRatio: 1.0

		anchors.verticalCenter: parent.verticalCenter
		anchors.horizontalCenter: parent.horizontalCenter

		opacity: 0.7

		states: [
			State {
				when: root.horizontalAlignment == Qt.AlignLeft
				AnchorChanges {
					target: _loader
					anchors.right: undefined
					anchors.horizontalCenter: undefined
					anchors.left: parent.left
				}
			},
			State {
				when: root.horizontalAlignment == Qt.AlignRight
				AnchorChanges {
					target: _loader
					anchors.right: parent.right
					anchors.horizontalCenter: undefined
					anchors.left: undefined
				}
			}
		]


		StateGroup {
			states: [
				State {
					when: root.verticalAlignment == Qt.AlignTop
					AnchorChanges {
						target: _loader
						anchors.top: parent.top
						anchors.verticalCenter: undefined
						anchors.bottom: undefined
					}
				},
				State {
					when: root.verticalAlignment == Qt.AlignBottom
					AnchorChanges {
						target: _loader
						anchors.top: undefined
						anchors.verticalCenter: undefined
						anchors.bottom: parent.bottom
					}
				}
			]
		}
	}

	Item {
		id: _overlay
		anchors.fill: _loader
		visible: false

		LinearGradient {
			id: _gradient

			x: width*delta
			y: 0

			width: Math.max(_loader.width, _loader.height)
			height: Math.max(_loader.width, _loader.height)
			visible: true
			gradient: Gradient {
				GradientStop {
					position: 0.0
					color: "transparent"
				}
				GradientStop {
					position: 0.5
					color: "#505050"
				}
				GradientStop {
					position: 1.0
					color: "transparent"
				}
			}

			property double delta: -1.0

			start: Qt.point(width*0, 0)
			end: Qt.point(width, 0)

			SequentialAnimation {
				running: true
				loops: Animation.Infinite

				NumberAnimation {
					target: _gradient
					property: "delta"
					duration: 350
					easing.type: Easing.InOutQuad
					from: -1.0
					to: 0.0
				}

				NumberAnimation {
					target: _gradient
					property: "delta"
					duration: 350
					easing.type: Easing.InQuad
					to: 1.0
				}
				PauseAnimation {
					duration: 750
				}
			}
		}
	}

	OpacityMask {
		anchors.fill: _overlay
		source: _overlay
		maskSource: _loader
	}


	Component {
		id: _cmpRectangle

		Rectangle {
			radius: root.rectangleRadius
			color: "#202020"
		}
	}

	Component {
		id: _cmpEllipse

		Rectangle {
			radius: Math.min(height,width)/2
			color: "#202020"
		}
	}

	Component.onCompleted: resetRatios()

	function resetRatios() {
		if (widthRatio <= 0)
			_loader.widthRatio = 0.3 + Math.random()*0.65
		else
			_loader.widthRatio = widthRatio

		if (heightRatio <= 0)
			_loader.heightRatio = 0.5 + Math.random()*0.5
		else
			_loader.heightRatio = heightRatio
	}
}

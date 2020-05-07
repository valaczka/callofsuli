import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS

TextArea {
	id: control

	padding: 6
	leftPadding: padding + 2 + rect1.width
	bottomPadding: 2
	rightPadding: padding + 2

	Behavior on leftPadding { NumberAnimation { duration: 100 } }

	font.family: "Special Elite"
	font.pixelSize: CosStyle.pixelSize
	font.weight: Font.Medium

	property color textColor: CosStyle.colorAccent
	property bool lineVisible: true
	property bool tooltipEnabled: true

	opacity: enabled ? 1 : 0.5
	color: control.enabled ?
			   textColor :
			   "white"
	selectionColor: JS.setColorAlpha(CosStyle.colorPrimary, 0.4)
	selectedTextColor: color
	verticalAlignment: TextInput.AlignTop

	placeholderTextColor: JS.setColorAlpha(CosStyle.colorPrimary, 0.7)

	ToolTip.text: placeholderText
	ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
	ToolTip.visible: tooltipEnabled && hovered && ToolTip.text.length

	Behavior on color {  ColorAnimation { duration: 150 } }

	wrapMode: TextEdit.WordWrap




	property bool _textModified: false

	signal textModified()

	onActiveFocusChanged: if (activeFocus)
							  _textModified = false


	onTextChanged: _textModified = true

	onEditingFinished: if (_textModified) {
						   textModified()
					   }


	//! [background]
	background: Rectangle {
		width: control.width
		height: control.height
		color: "transparent"
		border.width: 0

		Rectangle {
			id: rect1
			x: 0
			height: rectLine.y
			width: 5

			visible: control.activeFocus && !control.readOnly && control.lineVisible
			color: CosStyle.colorAccent
		}

		Rectangle {
			id: rectLine
			x: 0
			y: control.height
			width: control.width
			height: 1
			visible: control.enabled && !control.readOnly && control.lineVisible
			border.width: 0
			color: "transparent"

			property color _color: control.activeFocus ?
									   CosStyle.colorAccentLighter :
									   (control.hovered ?
											CosStyle.colorPrimaryLighter :
											CosStyle.colorPrimary)

			Behavior on _color {  ColorAnimation { duration: 150 } }

			LinearGradient {
				anchors.fill: parent
				start: Qt.point(0, 0)
				end: Qt.point(width, 0)
				gradient: Gradient {
					GradientStop { position: 0.0; color: "transparent" }
					GradientStop { position: 0.3; color: rectLine._color }
					GradientStop { position: 0.7; color: rectLine._color }
					GradientStop { position: 1.0; color: "transparent" }
				}
			}
		}
	}
	//! [background]
}

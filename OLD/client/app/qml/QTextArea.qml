import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS

TextArea {
	id: control

	padding: 6
	leftPadding: padding + 2 //+ rect1.width
	bottomPadding: 2
	rightPadding: padding + 2

	Behavior on leftPadding { NumberAnimation { duration: 100 } }

	font.family: "Special Elite"
	font.pixelSize: CosStyle.pixelSize*0.95
	font.weight: Font.Medium

	property color textColor: CosStyle.colorAccent
	property bool lineVisible: true
	property bool tooltipEnabled: true
	property alias minimumHeight: bgRect.implicitHeight

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
		id: bgRect
		width: control.width
		height: control.height
		color: "transparent"
		border.width: 0

		implicitWidth: 50

		/*Rectangle {
			id: rect1
			x: 0
			height: rectLine.y
			width: 5

			visible: !control.readOnly && control.lineVisible //control.activeFocus &&
			color: CosStyle.colorAccent
		}*/

		Rectangle {
			id: rectLine
			anchors.left: parent.left
			anchors.top: parent.top
			width: 1
			height: control.height
			visible: control.enabled && !control.readOnly && control.lineVisible
			border.width: 0

			property color _color: control.activeFocus ?
									   CosStyle.colorAccentLighter :
									   (control.hovered ?
											CosStyle.colorPrimaryLighter :
											CosStyle.colorPrimary)

			Behavior on _color {  ColorAnimation { duration: 150 } }


			gradient: Gradient {
				orientation: Gradient.Vertical
				GradientStop { position: 0.0; color: "transparent" }
				GradientStop { position: 0.3; color: rectLine._color }
				GradientStop { position: 0.7; color: rectLine._color }
				GradientStop { position: 1.0; color: "transparent" }
			}
		}
	}
	//! [background]
}

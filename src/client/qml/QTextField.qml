/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS


TextField {
	id: control


//	implicitWidth: placeholder.implicitWidth + leftPadding + rightPadding
	implicitHeight: CosStyle.pixelSize + topPadding + bottomPadding + 8

	topPadding: 6
	leftPadding: 8 + (activeFocus ? rect1.width : 0 )
	bottomPadding: 0
	rightPadding: clearLabel.width

	Behavior on leftPadding { NumberAnimation { duration: 100 } }

	font.family: "Special Elite"
	font.pixelSize: CosStyle.pixelSize*0.95
	font.weight: Font.Medium
	passwordCharacter: "*"

	property color textColor: CosStyle.colorAccent
	property bool lineVisible: true
	property bool tooltipEnabled: true

	opacity: enabled ? 1 : 0.5
	color: control.enabled ?
			   textColor :
			   "white"
	selectionColor: JS.setColorAlpha(CosStyle.colorPrimary, 0.4)
	selectedTextColor: color
	verticalAlignment: TextInput.AlignVCenter

	placeholderTextColor: JS.setColorAlpha(CosStyle.colorPrimaryLighter, 0.7)

	ToolTip.text: placeholderText
	ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
	ToolTip.visible: tooltipEnabled && hovered && ToolTip.text.length


	Binding on textColor {
		when: !acceptableInput
		value: CosStyle.colorErrorLighter
	}

	Behavior on color {  ColorAnimation { duration: 150 } }


	property bool _textModified: false

	signal textModified()

	onActiveFocusChanged: if (activeFocus)
							  _textModified = false


	onTextChanged: _textModified = true

	onEditingFinished: if (_textModified) {
						   textModified()
					   }


	QFontImage {
		id: clearLabel
		icon: CosStyle.iconClear
		color: clearArea.containsMouse ? CosStyle.colorErrorLight : CosStyle.colorErrorLighter


		width: size+10
		height: parent.height

		Behavior on color { ColorAnimation { duration: 125 } }

		visible: control.text.length && !control.readOnly && (control.hovered || control.activeFocus)

		anchors.verticalCenter: parent.verticalCenter
		anchors.right: parent.right
	}

	MouseArea {
		id: clearArea
		anchors.fill: clearLabel
		hoverEnabled: true
		acceptedButtons: Qt.LeftButton
		onClicked: {
			control.clear()
			textModified()
		}
	}

	/*Text {
		id: placeholder
		x: control.leftPadding
		y: control.topPadding
		width: control.width - (control.leftPadding + control.rightPadding)
		height: control.height - (control.topPadding + control.bottomPadding)

		text: control.placeholderText
		font.pixelSize: CosStyle.pixelSize*0.8
		font.weight: Font.Thin
		color: "white"
		horizontalAlignment: control.horizontalAlignment
		verticalAlignment: control.verticalAlignment
		visible: !control.length && !control.preeditText &&
				 (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
		elide: Text.ElideRight
	}*/

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
			visible: control.enabled && !control.readOnly && control.lineVisible && (control.hovered || control.activeFocus)
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

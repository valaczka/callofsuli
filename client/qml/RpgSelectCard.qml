import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.Card {
	id: control

	property bool locked: false
	property bool selected: false
	property bool disabled: false
	property bool fullBg: false
	property alias image: _image.source
	property alias text: _label.text
	property alias mouseArea: _area
	property alias borderVisible: _border.visible
	property alias iconLockColor: _iconLock.color
	readonly property alias labelHeight: _label.height

	default property alias containerContent: _container.data

	signal clicked()

	outlined: true

	Binding {
		target: control
		property: "backgroundColor"
		value: "transparent"
		when: locked && !selected
	}

	Binding {
		target: control
		property: "backgroundColor"
		value: Qaterial.Colors.cyan700
		when: selected && !locked && !disabled
	}

	borderColor: "transparent"

	implicitWidth: 150
	implicitHeight: 150

	elevation: locked ? 0 : Qaterial.Style.card.activeElevation

	scale: _area.pressed ? 0.9 : selected ? 1.0 : 0.95

	Behavior on scale {
		NumberAnimation { duration: 125 }
	}

	readonly property color textColor: selected ? Qaterial.Colors.amber300 :
												  locked ? Qaterial.Style.colorTheme.disabledText :
														   Qaterial.Style.primaryTextColor()

	contentItem: Item {
		width: parent.width
		height: parent.height

		layer.enabled: true
		layer.effect: OpacityMask
		{
			maskSource: Rectangle
			{
				width: control.width
				height: control.height
				radius: control.radius
			}
		}


		Image
		{
			id: _image

			visible: !locked && !disabled

			fillMode: fullBg ? Image.PreserveAspectCrop : Image.PreserveAspectFit
			horizontalAlignment: Image.AlignHCenter
			verticalAlignment: Image.AlignVCenter
			width: Math.min(parent.width, sourceSize.width)
			height: Math.min(parent.height - (fullBg ? 0 : _label.height), sourceSize.height)
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.bottom: parent.bottom

			opacity: selected ? 1.0 : 0.7

			Behavior on opacity {
				NumberAnimation { duration: 125 }
			}
		}

		Desaturate {
			visible: locked || disabled

			anchors.fill: _image
			source: _image

			desaturation: 1.0

			opacity: 0.4
		}

		MouseArea {
			id: _area
			anchors.fill: parent
			acceptedButtons: Qt.LeftButton

			onClicked: control.clicked()
		}

		Qaterial.Icon {
			id: _iconLock
			anchors.centerIn: parent
			icon: Qaterial.Icons.lockOutline
			visible: locked
			size: parent.height*0.4
			color: Qaterial.Colors.cyan800
		}

		Item {
			id: _container

			anchors.fill: parent
		}


		Qaterial.Label
		{
			id: _label

			font: Qaterial.Style.textTheme.caption

			anchors.left: parent.left
			anchors.right: parent.right
			anchors.top: parent.top
			anchors.leftMargin: Qaterial.Style.card.horizontalPadding
			anchors.rightMargin: Qaterial.Style.card.horizontalPadding
			anchors.topMargin: Qaterial.Style.card.verticalPadding

			//bottomPadding: 10 * Qaterial.Style.pixelSizeRatio

			lineHeight: 0.8
			textFormat: Text.StyledText
			wrapMode: Text.Wrap

			color: control.textColor

			clip: true

			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter
		}


		Glow {
			anchors.fill: _label
			source: _label
			//visible: !locked && selected
			color: "black"
			radius: 1
			spread: 0.9
			samples: 5
		}

		Rectangle {
			id: _border
			anchors.fill: parent
			color: "transparent"
			radius: control.radius
			border.width: selected ? 3 : 1
			border.color: selected ? Qaterial.Colors.cyan300 :
									 control.enabled ? Qaterial.Style.dividersColor() :
													   Qaterial.Style.disabledDividersColor()
			visible: !locked
		}


		Qaterial.Icon {
			anchors.centerIn: parent
			icon: Qaterial.Icons.close
			visible: control.disabled && !locked
			size: parent.height*0.7
			color: Qaterial.Colors.red600
		}

	}


}

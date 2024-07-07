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
	property int bulletCount: -1
	property alias image: _image.source
	property alias text: _label.text

	signal clicked()

	width: ListView.view ? ListView.view.implicitHeight : 50
	height: ListView.view ? ListView.view.implicitHeight : 50

	outlined: true

	Binding {
		target: control
		property: "backgroundColor"
		value: "transparent"
		when: locked
	}

	borderColor: "transparent"


	elevation: locked ? 0 : Qaterial.Style.card.activeElevation

	scale: _area.pressed ? 0.85 : selected ? 1.0 : 0.9

	Behavior on scale {
		NumberAnimation { duration: 125 }
	}

	readonly property color textColor: selected ? Qaterial.Colors.cyan300 :
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
			height: Math.min(sourceSize.height, parent.height)
			width: parent.width
			fillMode: Image.PreserveAspectCrop

			visible: !locked

			opacity: selected ? 1.0 : 0.7

			Behavior on opacity {
				NumberAnimation { duration: 125 }
			}
		}

		Desaturate {
			id: _saturate

			visible: locked

			anchors.fill: _image
			source: _image

			desaturation: 1.0

			opacity: 0.4
		}


		Qaterial.Icon {
			anchors.centerIn: parent
			icon: Qaterial.Icons.lock
			visible: locked
			size: parent.height*0.4
			enabled: false
		}

		Qaterial.Label
		{
			id: _label

			font: Qaterial.Style.textTheme.caption

			anchors.left: parent.left
			anchors.right: parent.right
			anchors.bottom: parent.bottom
			anchors.leftMargin: Qaterial.Style.card.horizontalPadding
			anchors.rightMargin: Qaterial.Style.card.horizontalPadding
			anchors.bottomMargin: Qaterial.Style.card.verticalPadding

			lineHeight: 0.8
			textFormat: Text.StyledText
			wrapMode: Text.Wrap

			color: control.textColor

			clip: true

			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter
		}


		Rectangle {
			color: "black"
			anchors.centerIn: _bullet
			width: _bullet.width + 2*Qaterial.Style.card.verticalPadding
			height: _bullet.height + 2*Qaterial.Style.card.verticalPadding
			opacity: 0.9
			visible: _bullet.visible
		}

		Qaterial.IconLabel {
			id: _bullet

			visible: bulletCount > -1

			icon.source: Qaterial.Icons.bullet
			text: bulletCount

			anchors.top: parent.top
			anchors.right: parent.right
			anchors.topMargin: Qaterial.Style.card.verticalPadding
			anchors.rightMargin: Qaterial.Style.card.verticalPadding

			color: Qaterial.Colors.blue500
			font.family: Qaterial.Style.textTheme.body2.family
			font.pixelSize: Qaterial.Style.textTheme.body2.pixelSize
			font.weight: Font.DemiBold
			icon.width: Qaterial.Style.textTheme.body2.pixelSize * 1.2
			icon.height: Qaterial.Style.textTheme.body2.pixelSize * 1.2
			spacing: 2 * Qaterial.Style.pixelSizeRatio
		}

		Glow {
			anchors.fill: _bullet
			source: _bullet
			visible: _bullet.visible
			color: "black"
			radius: 1
			spread: 0.9
			samples: 5
		}

		Glow {
			anchors.fill: _label
			source: _label
			visible: !locked && selected
			color: "black"
			radius: 1
			spread: 0.9
			samples: 5
		}

		Rectangle {
			anchors.fill: parent
			color: "transparent"
			radius: control.radius
			border.width: selected ? 3 : 1
			border.color: selected ? Qaterial.Colors.cyan300 :
									 control.enabled ? Qaterial.Style.dividersColor() :
													   Qaterial.Style.disabledDividersColor()
			visible: !locked
		}

		MouseArea {
			id: _area
			anchors.fill: parent
			acceptedButtons: Qt.LeftButton

			onClicked: control.clicked()
		}
	}


}

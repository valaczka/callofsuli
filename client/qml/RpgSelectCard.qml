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
	property int bulletCount: -1
	property alias image: _image.source
	property alias text: _label.text
	property alias mouseArea: _area
	property alias subImage: _subImage.source

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

			fillMode: Image.PreserveAspectCrop
			horizontalAlignment: Image.AlignHCenter
			verticalAlignment: Image.AlignVCenter
			width: Math.min(parent.width, sourceSize.width)
			height: Math.min(parent.height, sourceSize.height)
			anchors.centerIn: parent

			visible: !locked && !disabled

			opacity: selected ? 1.0 : 0.7

			Behavior on opacity {
				NumberAnimation { duration: 125 }
			}

			Image {
				id: _subImage

				visible: source != ""

				readonly property real imgSize: Math.min(parent.width*0.35, parent.height*0.35)

				width: imgSize
				height: imgSize

				anchors.right: parent.right
				anchors.bottom: parent.bottom

				anchors.rightMargin: parent.width*0.1
				anchors.bottomMargin: parent.height*0.1

				fillMode: Image.PreserveAspectFit
			}

			Rectangle {
				anchors.fill: _subImage
				visible: _subImage.visible
				color: "transparent"
				border.width: 2
				border.color: control.locked ? Qaterial.Colors.black : Qaterial.Colors.cyan300
			}

		}

		Desaturate {
			id: _saturate

			visible: locked || disabled

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
			color: Qaterial.Colors.cyan800
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

		Qaterial.Label {
			id: _bullet

			visible: bulletCount > -1

			//icon.source: Qaterial.Icons.bullet
			text: bulletCount

			anchors.top: parent.top
			anchors.right: parent.right
			anchors.topMargin: Qaterial.Style.card.verticalPadding
			anchors.rightMargin: Qaterial.Style.card.verticalPadding

			color: Qaterial.Colors.blue500
			font.family: Qaterial.Style.textTheme.body2.family
			font.pixelSize: Qaterial.Style.textTheme.body2.pixelSize
			font.weight: Font.DemiBold
			/*icon.width: Qaterial.Style.textTheme.body2.pixelSize * 1.2
			icon.height: Qaterial.Style.textTheme.body2.pixelSize * 1.2
			spacing: 2 * Qaterial.Style.pixelSizeRatio*/
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
			//visible: !locked && selected
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


		Qaterial.Icon {
			anchors.centerIn: parent
			icon: Qaterial.Icons.close
			visible: control.disabled && !locked
			size: parent.height*0.7
			color: Qaterial.Colors.red600
		}

		MouseArea {
			id: _area
			anchors.fill: parent
			acceptedButtons: Qt.LeftButton

			onClicked: control.clicked()
		}
	}


}

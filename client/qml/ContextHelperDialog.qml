import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.ModalDialog
{
	id: root

	title: qsTr("Tudtad?")

	dialogImplicitWidth: 600
	autoFocusButtons: true

	property url image: ""
	property string description: ""
	property string iconSource: ""
	property color iconColor: Qaterial.Style.iconColor()

	contentItem: Loader {
		sourceComponent: image != "" ? _cmpImage : _cmpIcon
	}

	Component {
		id: _cmpIcon

		Item
		{
			width: parent.width

			Binding on implicitHeight
			{
				value: Math.floor(Math.max(_icon.implicitHeight, _text.implicitHeight)) + _check.implicitHeight
				delayed: true
			}

			Qaterial.RoundColorIcon
			{
				id: _icon

				anchors.verticalCenter: parent.verticalCenter
				highlighted: true
				fill: false
				source: root.iconSource
				color: root.iconColor
				iconSize: Qaterial.Style.roundIcon.size
				visible: source != ""
			}

			Flickable
			{
				id: _flickable

				x: _icon.visible ? (_icon.width + root.horizontalPadding) : 0
				width: parent.width - x

				implicitHeight: _text.implicitHeight
				interactive: contentHeight > height

				contentHeight: _text.height
				flickableDirection: Flickable.VerticalFlick

				boundsBehavior: Flickable.StopAtBounds
				boundsMovement: Flickable.StopAtBounds

				ScrollIndicator.vertical: Qaterial.ScrollIndicator {}

				clip: true

				Qaterial.LabelBody2
				{
					id: _text

					text: root.description
					color: Qaterial.Style.primaryTextColor()
					wrapMode: Text.Wrap

					bottomPadding: 5 * Qaterial.Style.pixelSizeRatio
				}
			}

			Qaterial.CheckButton {
				id: _check
				anchors.left: _flickable.left
				anchors.bottom: parent.bottom
				text: qsTr("Ne jelenjen meg többet")
				checked: !Client.contextHelper.enabled
				onToggled: Client.contextHelper.enabled = !checked
			}
		}

	}

	Component {
		id: _cmpImage

		Flickable
		{
			id: _flickable2

			/*width: parent.width
			height: parent.height*/

			implicitHeight: Math.max(400, _layout.implicitHeight)
			interactive: contentHeight > height

			contentHeight: _layout.height
			flickableDirection: Flickable.VerticalFlick

			boundsBehavior: Flickable.StopAtBounds
			boundsMovement: Flickable.StopAtBounds

			ScrollIndicator.vertical: Qaterial.ScrollIndicator {}

			clip: true

			GridLayout {
				id: _layout
				columns: root.width > root.height ? 2 : 1
				columnSpacing: 10 * Qaterial.Style.pixelSizeRatio
				rowSpacing: 5 * Qaterial.Style.pixelSizeRatio

				width: _flickable2.width
				height: Math.max(implicitHeight, _flickable2.height)

				Binding on implicitHeight
				{
					value: (_layout.columns > 1 ? Math.max(_image.implicitHeight, _text2.implicitHeight)
												  : _image.implicitHeight + _text2.implicitHeight)
						   + _layout.rowSpacing + _check2.implicitHeight

					delayed: true
				}

				AnimatedImage {
					id: _image

					Layout.fillWidth: true
					Layout.fillHeight: true

					horizontalAlignment: Image.AlignHCenter
					verticalAlignment: Image.AlignVCenter

					fillMode: Image.PreserveAspectFit

					source: root.image
				}

				Qaterial.LabelBody2
				{
					id: _text2

					text: root.description
					color: Qaterial.Style.primaryTextColor()
					wrapMode: Text.Wrap

					verticalAlignment: Text.AlignVCenter

					Layout.fillWidth: true
					Layout.fillHeight: true

					bottomPadding: 5 * Qaterial.Style.pixelSizeRatio
				}

				Qaterial.CheckButton {
					id: _check2

					Layout.fillWidth: true
					Layout.columnSpan: _layout.columns

					text: qsTr("Ne jelenjen meg többet")
					checked: !Client.contextHelper.enabled
					onToggled: Client.contextHelper.enabled = !checked
				}
			}
		}

	}

	standardButtons: DialogButtonBox.Ok

	onAboutToHide: Client.contextHelper.dialogPresent = false
	onAboutToShow: Client.contextHelper.dialogPresent = true
}

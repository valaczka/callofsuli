import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


SpinBox {
	id: control

	implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
							contentItem.implicitWidth + leftPadding + rightPadding)
	implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
							 implicitContentHeight + topPadding + bottomPadding,
							 up.implicitIndicatorHeight, down.implicitIndicatorHeight)


	padding: 0
	leftPadding: padding + (control.mirrored ? (up.indicator ? up.indicator.width : 0) : (down.indicator ? down.indicator.width : 0))
	rightPadding: padding + (control.mirrored ? (down.indicator ? down.indicator.width : 0) : (up.indicator ? up.indicator.width : 0))

	font: Qaterial.Style.textTheme.subtitle1

	property double backgroundImplicitWidth: Qaterial.Style.toolButton.minWidth
	property double backgroundImplicitHeight: Qaterial.Style.toolButton.minHeight

	property bool clipRipple: true
	property color rippleColor: Qaterial.Style.rippleColor(Qaterial.Style.RippleBackground.Background)

	up.indicator: Rectangle {
		id: _up
		color: "transparent"
		implicitWidth: Qaterial.Style.toolButton.minWidth
		implicitHeight: Qaterial.Style.toolButton.minHeight
		y: (control.height-height)/2
		x: control.mirrored ? 0 : control.width - width

		Qaterial.Ripple
		{
			id: _rippleUp

			width: parent.width
			height: parent.height
			anchors.centerIn: parent

			pressed: control.up.pressed
			anchor: _up
			active: control.up.pressed || control.up.hovered
			color: control.rippleColor
		}

		Qaterial.Icon {
			anchors.centerIn: parent
			color: Qaterial.Style.foregroundColor
			icon: control.mirrored ? Qaterial.Icons.minus : Qaterial.Icons.plus
		}
	}


	down.indicator: Rectangle {
		id: _down
		color: "transparent"
		implicitWidth: Qaterial.Style.toolButton.minWidth
		implicitHeight: Qaterial.Style.toolButton.minHeight
		y: (control.height-height)/2
		x: control.mirrored ? control.width - width : 0

		Qaterial.Ripple
		{
			id: _rippleDown

			width: parent.width
			height: parent.height
			anchors.centerIn: parent

			pressed: control.down.pressed
			anchor: _down
			active: control.down.pressed || control.down.hovered
			color: control.rippleColor
		}

		Qaterial.Icon {
			anchors.centerIn: parent
			color: Qaterial.Style.foregroundColor
			icon: control.mirrored ? Qaterial.Icons.plus : Qaterial.Icons.minus
		}
	}

	validator: IntValidator {
		locale: control.locale.name
		bottom: Math.min(control.from, control.to)
		top: Math.max(control.from, control.to)
	}

	contentItem: TextField {
		text: control.displayText

		font: control.font

		color: enabled ? Qaterial.Style.primaryTextColor() : Qaterial.Style.hintTextColor()
		selectionColor: Qaterial.Style.accentColor
		selectedTextColor: Qaterial.Style.shouldReverseForegroundOnAccent ? Qaterial.Style.primaryTextColorReversed() : Qaterial.Style.primaryTextColor()

		verticalAlignment: TextInput.AlignVCenter
		horizontalAlignment: TextInput.AlignHCenter

		readOnly: !control.editable
		validator: control.validator
		inputMethodHints: control.inputMethodHints
		clip: width < implicitWidth
	}


	background: Item
	{
		implicitWidth: control.backgroundImplicitWidth
		implicitHeight: control.backgroundImplicitHeight
	}
}

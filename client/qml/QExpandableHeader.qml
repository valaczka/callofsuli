import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Row {
	id: control

	property Qaterial.Expandable expandable: null
	property string text
	property string icon

	width: expandable ? expandable.width : _btn.implicitWidth+_iconLabel.implicitWidth

	Qaterial.RoundButton {
		id: _btn
		icon.source: expandable && expandable.expanded ? Qaterial.Icons.plus : Qaterial.Icons.minus
		anchors.verticalCenter: parent.verticalCenter
		onClicked: expandable.expanded = !expandable.expanded
		visible: expandable
	}

	Qaterial.IconLabel
	{
		id: _iconLabel
		width: parent.width-(_btn.visible ? _btn.width : 0)
		anchors.verticalCenter: parent.verticalCenter
		horizontalAlignment: Qt.AlignLeft
		font: Qaterial.Style.textTheme.body2Upper
		wrapMode: Label.Wrap
		text: control.text
		icon.source: control.icon

		MouseArea {
			anchors.fill: parent
			acceptedButtons: Qt.LeftButton
			onClicked: if (expandable) expandable.expanded = !expandable.expanded
		}
	}
}

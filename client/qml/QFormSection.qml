import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


Item {
	id: root

	property string text: ""
	property string icon: ""

	property double topPadding: 20

	implicitWidth: _iconLabel.implicitWidth
	implicitHeight: _iconLabel.implicitHeight+topPadding

	Qaterial.IconLabel
	{
		id: _iconLabel
		horizontalAlignment: Qt.AlignLeft
		font: Qaterial.Style.textTheme.body2Upper
		wrapMode: Text.Wrap
		anchors.bottom: parent.bottom
		width: parent.width
		text: root.text
		icon.source: root.icon
	}
}

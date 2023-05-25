import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Rectangle {
	id: control

	property Qaterial.Expandable expandable: null
	property string text
	property string icon
	property bool separator: true
	property alias rightSource: _loader.source
	property alias rightSourceComponent: _loader.sourceComponent

	color: "transparent"

	width: expandable ? expandable.width : _btn.implicitWidth+_iconLabel.implicitWidth+(_loader.item ? _loader.item.implicitWidth : 0)
	height: _headerRow.height

	RowLayout {
		id: _headerRow
		width: parent.width
		spacing: 5

		Qaterial.RoundButton {
			id: _btn
			icon.source: expandable && expandable.expanded ? Qaterial.Icons.plus : Qaterial.Icons.minus
			Layout.alignment: Qt.AlignCenter
			onClicked: expandable.expanded = !expandable.expanded
			visible: expandable
		}

		Qaterial.IconLabel
		{
			id: _iconLabel
			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.alignment: Qt.AlignCenter
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

		Loader {
			id: _loader
			Layout.alignment: Qt.AlignCenter
		}
	}

	Qaterial.HorizontalLineSeparator {
		width: parent.width
		anchors.bottom: parent.bottom
		visible: control.separator
	}
}

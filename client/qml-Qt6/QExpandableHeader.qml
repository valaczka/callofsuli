import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

Rectangle {
	id: control

	property Qaterial.Expandable expandable: null
	property string text
	property string icon
	property bool separator: true
	property alias rightSource: _loader.source
	property alias rightSourceComponent: _loader.sourceComponent

	property double topPadding: 0

	property bool expanded: expandable ? expandable.expanded : false
	property alias button: _btn

	signal clicked()

	color: "transparent"

	//width: expandable ? expandable.width : implicitWidth
	height: implicitHeight

	implicitWidth: _btn.implicitWidth+_iconLabel.implicitWidth+(_loader.item ? _loader.item.implicitWidth : 0)
	implicitHeight: _headerRow.height+topPadding

	RowLayout {
		id: _headerRow
		width: parent.width
		y: control.topPadding
		spacing: 5


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
				onClicked: {
					if (expandable) expandable.expanded = !expandable.expanded
					control.clicked()
				}
			}
		}

		Loader {
			id: _loader
			Layout.alignment: Qt.AlignCenter
		}

		Qaterial.RoundButton {
			id: _btn
			icon.source: control.expanded ? Qaterial.Icons.chevronUp : Qaterial.Icons.chevronDown
			Layout.alignment: Qt.AlignCenter
			onClicked: {
				if (expandable) expandable.expanded = !expandable.expanded
				control.clicked()
			}
		}

	}

	Qaterial.HorizontalLineSeparator {
		width: parent.width
		anchors.bottom: parent.bottom
		visible: control.separator
	}
}

import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Qaterial.Expandable {
	id: root
	width: parent.width

	property var refreshFunc: null

	property string _text: ""

	expanded: true

	onExpandedChanged: refresh()

	header: QExpandableHeader {
		text: qsTr("Előnézet")
		icon: Qaterial.Icons.eyeSettings
		expandable: root

		rightSourceComponent: Qaterial.RoundButton {
			icon.source: Qaterial.Icons.refresh
			icon.color: Qaterial.Style.iconColor()
			onClicked: refresh()
			visible: root.expanded
		}
	}

	delegate: QIndentedItem {
		width: root.width

		Qaterial.LabelBody2 {
			id: textPreview
			width: parent.width
			wrapMode: Text.Wrap
			textFormat: Text.MarkdownText
			leftPadding: 20
			rightPadding: 20
			text: root._text
			color: Qaterial.Colors.cyan400
		}
	}

	function refresh() {
		if (!expanded)
			return

		if (!refreshFunc)
			return

		_text = refreshFunc()
	}
}

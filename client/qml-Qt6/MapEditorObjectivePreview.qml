import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

Qaterial.Expandable {
	id: root
	width: parent.width

	property var refreshFunc: null

	property string _text: ""

	expanded: true

	onExpandedChanged: refresh()

	header: QExpandableHeader {
		text: qsTr("Előnézet")
		icon: Qaterial.Icons.eyeOutline
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

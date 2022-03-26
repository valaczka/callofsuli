import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QCollapsible {
	id: collapsiblePreview

	property var refreshFunc: null

	title: qsTr("Előnézet")
	collapsed: true

	onCollapsedChanged: refresh()

	rightComponent: QToolButton {
		icon.source: CosStyle.iconRefresh
		onClicked: refresh()
	}

	QLabel {
		id: textPreview
		width: parent.width
		wrapMode: Text.Wrap
		textFormat: Text.MarkdownText
		leftPadding: 20
		rightPadding: 20
	}

	function refresh() {
		if (collapsiblePreview.collapsed)
			return

		if (!refreshFunc)
			return

		var d = refreshFunc()
		textPreview.text = d.text
	}
}

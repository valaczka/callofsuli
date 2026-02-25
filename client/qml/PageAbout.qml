import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

QPage {
	id: control

	title: qsTr("Névjegy")

	QScrollable {
		anchors.fill: parent

		Column {
			id: col
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize) //parent.width-(Math.max(Client.safeMarginLeft, Client.safeMarginRight, 10)*2)
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: 10

			Item {
				width: parent.width
				height: 20
			}

			CosImage {
				id: cosImage
				anchors.horizontalCenter: parent.horizontalCenter
				width: Math.min(parent.width*0.7, 800)
				glow: false
			}

			Qaterial.LabelBody2 {
				id: labelVersion
				horizontalAlignment: Qt.AlignHCenter
				wrapMode: Text.Wrap
				width: parent.width
				color: Qaterial.Style.primaryTextColor()

				padding: 20

				text: qsTr("Verzió: ")+Qt.application.version+"\n© 2012-2026 Valaczka János Pál"
			}

			Qaterial.LabelHeadline6 {
				text: qsTr("Credits")
				topPadding: 15
				bottomPadding: 5
			}

			Qaterial.LabelBody2 {
				width: parent.width
				wrapMode: Text.Wrap
				text: Client.Utils.fileContentRead(":/CREDITS.md")
				textFormat: Text.MarkdownText
				bottomPadding: 15

				linkColor: Qaterial.Colors.cyan500

				onLinkActivated: link => Client.Utils.openUrl(link)
			}

			Qaterial.LabelHeadline6 {
				text: qsTr("License")
				topPadding: 15
				bottomPadding: 5
			}

			Qaterial.LabelBody2 {
				width: parent.width
				wrapMode: Text.Wrap
				text: Client.Utils.fileContentRead(":/license.txt")
				textFormat: Text.MarkdownText
				bottomPadding: 15

				onLinkActivated: link => Client.Utils.openUrl(link)
			}

			Qaterial.LabelHeadline6 {
				text: qsTr("Rendszer")
				bottomPadding: 5
				//visible: Client.debug
			}

			Qaterial.LabelBody2 {
				width: parent.width
				wrapMode: Text.Wrap
				text: Client.getSystemInfo()
				textFormat: Text.MarkdownText
				bottomPadding: 15
				//visible: Client.debug
			}
		}

	}
}

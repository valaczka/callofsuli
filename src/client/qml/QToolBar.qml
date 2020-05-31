import QtQuick 2.7
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.14
import "Style"
import "JScript.js" as JS

ToolBar {
	id: control

	property alias title: animatedTitle.str
	property alias subtitle: labelSubtitle.text
	property alias backButton: backButton
	property string backButtonIcon: CosStyle.iconBack
	property alias menuLoader: menuLoader

	default property alias rowContent: mainRow.data

	height: Math.max(CosStyle.baseHeight, 48)

	Material.primary: "transparent"

	RowLayout {
		id: mainRow
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.verticalCenter: parent.verticalCenter
		anchors.leftMargin: backButton.visible ? 0 : 10

		QToolButton {
			id: backButton

			Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

			icon.source: backButtonIcon
		}

		Column {
			id: titleColumn

			Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
			Layout.fillWidth: false

			spacing: -5

			Row {
				id: rowAnimatedTitle
				spacing: 0

				QLabel {
					anchors.verticalCenter: rowAnimatedTitle.verticalCenter
					text: "["
					font.weight: Font.Thin
					font.pixelSize: CosStyle.pixelSize * 1.2
					color: CosStyle.colorPrimaryLight
				}

				QAnimatedText {
					id: animatedTitle
					anchors.verticalCenter: rowAnimatedTitle.verticalCenter
					font.pixelSize: CosStyle.pixelSize * 1.2
					font.weight: Font.DemiBold
					font.capitalization: Font.AllUppercase
					color: CosStyle.colorAccentLighter
				}

				QLabel {
					anchors.verticalCenter: rowAnimatedTitle.verticalCenter
					text: "]"
					font.weight: Font.Thin
					color: CosStyle.colorPrimaryLight
					font.pixelSize: CosStyle.pixelSize * 1.2
				}
			}

			QLabel {
				id: labelSubtitle
				font.pixelSize: CosStyle.pixelSize*0.8
				font.weight: Font.Normal
				color: CosStyle.colorPrimaryLighter
			}
		}

		Loader {
			id: menuLoader
			Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
			visible: !animatedTitle.running
		}

		Item {
			Layout.fillWidth: true
		}
	}

	function resetTitle() {
		animatedTitle.resetStr()
	}

}

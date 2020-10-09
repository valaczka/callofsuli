import QtQuick 2.7
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.3
import QtGraphicalEffects 1.0
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

	rightPadding: 10

	default property alias rowContent: mainRow.data

	height: Math.max(CosStyle.baseHeight, 48, mainRow.height+10)

	Material.primary: "transparent"

	background: Rectangle {
		id: rect
		color: "transparent"
		implicitHeight: 32

		DropShadow {
			id: dropshadow
			anchors.fill: border2
			horizontalOffset: 3
			verticalOffset: 3
			color: JS.setColorAlpha("black", 0.75)
			source: border2
		}

		BorderImage {
			id: border2
			source: "qrc:/img/border3.svg"
			visible: false

			//sourceSize.height: 141
			//sourceSize.width: 414

			anchors.fill: parent
			anchors.rightMargin: dropshadow.horizontalOffset
			border.top: 10
			border.left: 5
			border.right: 80
			border.bottom: 10

			horizontalTileMode: BorderImage.Repeat
			verticalTileMode: BorderImage.Repeat
		}


		Image {
			id: metalbg
			source: "qrc:/img/metalbg2.png"
			visible: false
			fillMode: Image.Tile
			anchors.fill: border2
		}

		OpacityMask {
			id: opacity1
			anchors.fill: border2
			source: metalbg
			maskSource: border2
		}
	}

	RowLayout {
		id: mainRow
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.verticalCenter: parent.verticalCenter
		anchors.leftMargin: backButton.visible ? 0 : 10
		anchors.verticalCenterOffset: -2

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

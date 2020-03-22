import QtQuick 2.7
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.3
import "Style"
import "JScript.js" as JS

ToolBar {
	id: control

	property alias title: animatedTitle.str
	property alias backButton: backButton
	property alias rightLoader: rightLoader

	Material.primary: CosStyle.colorPrimaryDark

	RowLayout {
		anchors.fill: parent
		anchors.leftMargin: backButton.visible ? 0 : 10

		ToolButton {
			id: backButton

			Material.foreground: CosStyle.colorPrimaryLight
			Component.onCompleted: JS.setIconFont(backButton, "M\ue5c4")
		}


		Row {
			id: rowAnimatedTitle
			spacing: 0

			Layout.fillWidth: true

			Label {
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

			Label {
				anchors.verticalCenter: rowAnimatedTitle.verticalCenter
				text: "]"
				font.weight: Font.Thin
				color: CosStyle.colorPrimaryLight
				font.pixelSize: CosStyle.pixelSize * 1.2

			}
		}

		Loader {
			id: rightLoader
		}
	}

}

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
	property string backButtonIcon: CosStyle.iconBack
	//property alias rightLoader: rightLoader
	property alias menuLoader: menuLoader

	default property alias rowContent: mainRow.data

	//height: CosStyle.baseHeight

	Material.primary: "transparent" //CosStyle.colorPrimaryDark

	RowLayout {
		id: mainRow
		anchors.fill: parent
		anchors.leftMargin: backButton.visible ? 0 : 10

		QToolButton {
			id: backButton

			icon.source: backButtonIcon
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

			Loader {
				id: menuLoader
				visible: !animatedTitle.running
			}
		}

		/*Loader {
			id: rightLoader
			Layout.fillWidth: false
		}*/
	}

	function resetTitle() {
		animatedTitle.resetStr()
	}

}

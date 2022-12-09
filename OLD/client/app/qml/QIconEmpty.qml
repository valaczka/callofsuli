import QtQuick 2.15
import "."
import "Style"
import "JScript.js" as JS

Column {
	id: control

	property color color: "#90FFFFFF"
	property alias size: image.size
	property alias icon: image.icon
	property alias text: label.text
	property alias textWidth: label.width

	property QTabContainer tabContainer: null
	property bool _populated: tabContainer ? false : true
	readonly property bool _visible: tabContainer && tabContainer.tabPage && tabContainer.tabPage.activity ?
								_populated && !tabContainer.tabPage.activity.isBusy :
								_populated

	spacing: 10

	QFontImage {
		id: image
		icon: CosStyle.iconStudentQuestion
		size: CosStyle.pixelSize*7
		color: control.color
		anchors.horizontalCenter: parent.horizontalCenter
		visible: _visible
	}

	QLabel {
		id: label
		anchors.horizontalCenter: parent.horizontalCenter
		color: control.color
		font.pixelSize: CosStyle.pixelSize*1.7
		font.weight: Font.Light
		horizontalAlignment: Text.AlignHCenter
		wrapMode: Text.Wrap
		text: qsTr("Sajnos ez a lista üres")
		visible: _visible
	}

	Connections {
		target: tabContainer
		function onPopulated() {
			_populated = true
		}
	}

}

import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QCollapsible {
	id: collapsible
	title: qsTr("Egész számok összeadása, kivonása")

	property string moduleData: ""

	/*property bool editable: false

	rightComponent: QToolButton {
		icon.source: CosStyle.iconEdit
		onClicked: editable = true
	}*/


	QLabel {
		padding: 10
		width: parent.width
		wrapMode: Text.Wrap
		text: qsTr("Ez a modul a kívánt mennyiségben 2 egész számot állít elő, melyek beállítás szerint összeadhatók egymással vagy kivonhatóak egymásból.")
	}


	function getData() {
		return moduleData
	}

}


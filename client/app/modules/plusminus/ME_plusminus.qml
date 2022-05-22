import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QLabel {
	id: label

	property var moduleData: ({})

	signal modified()

	padding: 10
	width: parent.width
	wrapMode: Text.Wrap
	text: qsTr("Ez a modul a kívánt mennyiségben 2 egész számot állít elő, melyek beállítás szerint összeadhatók egymással vagy kivonhatóak egymásból.")


	function getData() {
		var r = {}
		return r
	}

}


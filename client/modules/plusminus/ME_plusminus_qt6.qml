import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QFormColumn {
	id: root

	width: parent.width

	property Item objectiveEditor: null
	property MapEditorStorage storage: null
	property MapEditorObjective objective: null

	property bool readOnly: true

	Qaterial.LabelBody2 {
		width: parent.width

		wrapMode: Text.Wrap
		text: qsTr("Ez az adatbank a kívánt mennyiségben 2 egész számot állít elő, melyek beállítás szerint összeadhatók egymással vagy kivonhatóak egymásból.")
	}


	function saveData() {

	}

	function loadData() {

	}

	function previewData() {
		return ({})
	}
}

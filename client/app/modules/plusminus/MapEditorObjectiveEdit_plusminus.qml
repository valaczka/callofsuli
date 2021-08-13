import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QAccordion {
	id: control

	property string objectiveUuid: ""
	property var moduleData: null




	function reloadData() {
		/*if (!moduleData)
			return

		var d = moduleData.objectiveData

		if (d) {
			var j=JSON.parse(d)
			textQuestion.setData(j.question ? j.question : "")
			chTrue.setData(j.correct ? j.correct : false)
		}*/
	}


	function saveData() {
		/*if (!objectiveUuid.length)
			return

		var d = {}

		d.question = textQuestion.text
		d.correct = chTrue.checked

		var x = JSON.stringify(d)

		mapEditor.run("objectiveModify", {uuid: objectiveUuid, data: { data: x }})*/
	}

}


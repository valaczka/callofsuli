import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QAccordion {
	id: control

	anchors.fill: parent

	property string objectiveUuid: ""
	property var moduleData: null

	MapEditorObjectiveEditHeader {
		id: header
		title: qsTr("Igaz/hamis")
	}


	QCollapsible {
		id: collapsible
		title: qsTr("Kérdés")

		QGridLayout {
			width: parent.width

			watchModification: false

			QGridLabel { field: textQuestion }

			QGridTextField {
				id: textQuestion
				fieldName: qsTr("Kérdés")

				onTextModified: saveData()
			}

			QGridCheckBox {
				id: chTrue
				text: qsTr("Helyes-e?")

				onToggled: saveData()
			}

		}
	}


	Connections {
		target: mapEditor

		function onObjectiveLoaded() {
			textQuestion.forceActiveFocus()
		}
	}


	function reloadData() {
		if (!moduleData)
			return

		var d = moduleData.objectiveData

		if (d) {
			var j=JSON.parse(d)
			textQuestion.setData(j.question ? j.question : "")
			chTrue.setData(j.correct ? j.correct : false)
		}
	}


	function saveData() {
		if (!objectiveUuid.length)
			return

		var d = {}

		d.question = textQuestion.text
		d.correct = chTrue.checked

		var x = JSON.stringify(d)

		mapEditor.run("objectiveModify", {uuid: objectiveUuid, data: { data: x }})
	}

}


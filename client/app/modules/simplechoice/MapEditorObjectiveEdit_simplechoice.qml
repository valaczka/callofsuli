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

	MapEditorObjectiveEditHeader {
		id: header
		title: qsTr("Egyszerű választás")
	}


	QCollapsible {
		title: qsTr("Kérdés")

		QGridLayout {
			width: parent.width

			watchModification: false

			QGridLabel { field: textQuestion }

			QGridTextField {
				id: textQuestion
				fieldName: qsTr("Kérdés")
				placeholderText: qsTr("Ez a kérdés fog megjelenni")

				onTextModified: saveData()
			}

			QGridLabel { field: textCorrectAnswer }

			QGridTextField {
				id: textCorrectAnswer
				fieldName: qsTr("Helyes válasz")
				placeholderText: qsTr("Ez lesz a helyes válasz")

				onTextModified: saveData()
			}

		}
	}

	QCollapsible {
		title: qsTr("Helytelen válaszok")

		QListTextFieldDelegate {
			id: answerView
			width: parent.width

			removeRole: "remove"
			addRole: "add"

			defaultPlaceholderText: qsTr("Ez helytelen válasz lesz")

			onRemoveRequest: {
				answerModel.remove(index)
				saveData()
			}

			onAddRequest: addItem()

			onItemAccepted: {
				if (index == answerModel.count-2)
					addItem()
				else
					answerView.forceFocus(index+1)

			}

			onItemModified: {
				answerModel.set(index, { text: text })
				saveData()
			}

			model: ListModel {
				id: answerModel
			}


			function addItem() {
				answerModel.insert(answerModel.count-1, {text: "", remove: true, add: false})
			}
		}
	}



	function reloadData() {
		if (!moduleData)
			return

		answerModel.clear()

		var d = moduleData.objectiveData

		if (d) {
			var j=JSON.parse(d)
			textQuestion.setData(j.question ? j.question : "")
			textCorrectAnswer.setData(j.correct ? j.correct : "")
			if (j.answers && j.answers.length) {
				for (var i=0; i<j.answers.length; i++) {
					answerModel.append({text: j.answers[i], add: false, remove:true})
				}
			}
		}

		answerModel.append({text: "", add: true, remove: false})
	}


	function saveData() {
		if (!objectiveUuid.length)
			return

		var d = {}
		var list = []

		for (var i=0; i<answerModel.count; i++) {
			var o = answerModel.get(i)
			if (!o.add)
				list.push(o.text)
		}

		d.question = textQuestion.text
		d.correct = textCorrectAnswer.text
		d.answers = list

		var x = JSON.stringify(d)

		mapEditor.run("objectiveModify", {uuid: objectiveUuid, data: { data: x }})
	}

}


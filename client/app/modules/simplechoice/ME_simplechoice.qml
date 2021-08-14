import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QCollapsible {
	id: collapsible
	title: qsTr("Egyszerű választás")

	property string moduleData: ""
	property string storageData: ""
	property string storageModule: ""
	property int storageCount: 0

	interactive: false

	QGridLayout {
		width: parent.width

		watchModification: false

		QGridLabel { field: textQuestion }

		QGridTextField {
			id: textQuestion
			fieldName: qsTr("Kérdés")
			sqlField: "question"
			placeholderText: qsTr("Ez a kérdés fog megjelenni")

			onTextModified: getData()
		}

		QGridLabel { field: textCorrectAnswer }

		QGridTextField {
			id: textCorrectAnswer
			fieldName: qsTr("Helyes válasz")
			sqlField: "correct"
			placeholderText: qsTr("Ez lesz a helyes válasz")

			onTextModified: getData()
		}

		QGridLabel {
			field: areaAnswers
		}

		QGridTextArea {
			id: areaAnswers
			fieldName: qsTr("Helytelen válaszok")
			placeholderText: qsTr("Lehetséges helytelen válaszok (soronként)")
			background: Item {
				implicitWidth: 50
				implicitHeight: CosStyle.baseHeight*2
			}

			onTextModified: getData()
		}


	}


	Component.onCompleted: {
		if (moduleData == "")
			return

		var d = JSON.parse(moduleData)

		JS.setSqlFields([textQuestion, textCorrectAnswer], d)
		areaAnswers.setData(d.answers.join("\n"))
	}


	function getData() {
		var d = JS.getSqlFields([textQuestion, textCorrectAnswer])
		d.answers = areaAnswers.text.split("\n")

		moduleData = JSON.stringify(d)
		return moduleData
	}

}





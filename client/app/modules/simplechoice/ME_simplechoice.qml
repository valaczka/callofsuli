import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QCollapsible {
	id: collapsible
	title: qsTr("Egyszerű választás")

	property var moduleData: null
	property var storageData: null
	property string storageModule: ""
	property int storageCount: 0

	interactive: false

	QGridLayout {
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
			minimumHeight: CosStyle.baseHeight*2

			onTextModified: getData()
		}


	}


	Component.onCompleted: {
		if (!moduleData)
			return

		JS.setSqlFields([textQuestion, textCorrectAnswer], moduleData)
		areaAnswers.setData(moduleData.answers.join("\n"))
	}


	function getData() {
		var d = JS.getSqlFields([textQuestion, textCorrectAnswer])
		d.answers = areaAnswers.text.split("\n")

		moduleData = d
		return moduleData
	}

}





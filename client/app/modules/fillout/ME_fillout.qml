import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QCollapsible {
	id: collapsible
	title: qsTr("Szövegkitöltés")

	property string moduleData: ""
	property string storageData: ""
	property string storageModule: ""
	property int storageCount: 0

	FilloutHighlighter {
		id: hl

		document: area.textDocument
		wordForeground: "black"
		wordBackground: CosStyle.colorWarningLight
	}


	interactive: false

	QGridLayout {
		width: parent.width

		watchModification: false

		QGridLabel { field: area }

		QGridTextArea {
			id: area
			fieldName: qsTr("Szöveg")
			sqlField: "text"
			placeholderText: qsTr("Ide kell írni a szöveget, amiből a hiányzó szavakat ki kell egészíteniük. A lehetséges pótolandó szavakat vagy kifejezéseket két százalékjel (%) közé kell tenni. (Pl: A %hiányzó% szó, vagy a %hiányzó kifejezések%.)\n Amennyiben %-jelet szeretnénk megjeleníteni ezt kell írni helyette: \\%")

			minimumHeight: CosStyle.baseHeight*3

			onTextModified: getData()
		}

		QGridLabel {
			field: areaAnswers
		}

		QGridTextArea {
			id: areaAnswers
			fieldName: qsTr("Helytelen válaszok")
			placeholderText: qsTr("Lehetséges egyéb helytelen válaszok (soronként)")

			minimumHeight: CosStyle.baseHeight*2

			onTextModified: getData()
		}

		QGridText { text: qsTr("Kiegészítendő helyek száma:") }

		QGridSpinBox {
			id: spinCount
			from: 1
			value: 3
			to: 99
			editable: true
			sqlField: "count"
		}

		QGridText { text: qsTr("Válaszlehetőségek száma:") }

		QGridSpinBox {
			id: spinOptions
			from: spinCount.value
			value: 5
			to: 99
			editable: true
			sqlField: "optionsCount"
		}

	}


	Component.onCompleted: {
		if (moduleData == "")
			return

		var d = JSON.parse(moduleData)

		JS.setSqlFields([area, areaAnswers, spinCount, spinOptions], d)
		areaAnswers.setData(d.options.join("\n"))
	}


	function getData() {
		var d = JS.getSqlFields([area, areaAnswers, spinCount, spinOptions])
		d.options = areaAnswers.text.split("\n")

		moduleData = JSON.stringify(d)
		return moduleData
	}

}


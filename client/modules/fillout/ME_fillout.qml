import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Item {
	id: control

	width: parent.width
	height: layout.height

	property var moduleData: ({})
	property var storageData: ({})
	property string storageModule: ""
	property int storageCount: 0

	signal modified()

	QGridLayout {
		id: layout


		watchModification: true
		onModifiedChanged: if (layout.modified)
							   control.modified()

		FilloutHighlighter {
			id: hl

			document: area.textDocument
			wordForeground: "black"
			wordBackground: CosStyle.colorWarningLight
		}


		QGridLabel { field: area }

		QGridTextArea {
			id: area
			fieldName: qsTr("Szöveg")
			sqlField: "text"
			placeholderText: qsTr("Ide kell írni a szöveget, amiből a hiányzó szavakat ki kell egészíteniük. A lehetséges pótolandó szavakat vagy kifejezéseket két százalékjel (%) közé kell tenni. (Pl: A %hiányzó% szó, vagy a %hiányzó kifejezések%.)\n Amennyiben %-jelet szeretnénk megjeleníteni ezt kell írni helyette: \\%")

			minimumHeight: CosStyle.baseHeight*3
		}

		QGridLabel {
			field: areaAnswers
		}

		QGridTextArea {
			id: areaAnswers
			fieldName: qsTr("Helytelen válaszok")
			placeholderText: qsTr("Lehetséges egyéb helytelen válaszok (soronként)")

			minimumHeight: CosStyle.baseHeight*2

		}

		QGridText {
			text: qsTr("Kiegészítendő helyek száma:")
			field: spinCount
		}

		QGridSpinBox {
			id: spinCount
			from: 1
			value: 3
			to: 99
			editable: true
			sqlField: "count"
		}

		QGridText {
			text: qsTr("Válaszlehetőségek száma:")
			field: spinOptions
		}

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
		if (!moduleData)
			return

		JS.setSqlFields([area, areaAnswers, spinCount, spinOptions], moduleData)
		areaAnswers.setData(moduleData.options.join("\n"))
	}


	function getData() {
		var d = JS.getSqlFields([area, areaAnswers, spinCount, spinOptions])
		d.options = areaAnswers.text.split("\n")

		moduleData = d
		return moduleData
	}

}

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

	spacing: 10

	onModifiedChanged: if (objectiveEditor) objectiveEditor.modified = true

	readonly property bool isBinding: storage && (storage.module == "binding" ||
												  storage.module == "mergebinding")
	readonly property bool isMergeBinding: storage && (storage.module == "mergebinding" ||
													   storage.module == "mergeblock")
	readonly property bool isBlock: storage && (storage.module == "block" ||
												storage.module == "mergeblock")


	QFormSectionSelector {
		id: _sectionSelector

		form: root
		field: "sections"
		visible: isMergeBinding
		storage: root.storage
	}

	QFormTextField {
		id: _question
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		field: "question"
		width: parent.width

		visible: !isBlock && !isBinding

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextField {
		id: _questionBlock
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		helperText: qsTr("A \%1 jelöli a generált elem helyét")
		field: "question"
		width: parent.width
		visible: isBlock || isBinding

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextField {
		id: _questionPrint
		title: qsTr("Nyomtatási segítség")
		placeholderText: qsTr("Így kerülnek nyomtatásra a válaszlehetőségek (pl. főcsoport: \%1 | alcsoport: \%2)")
		helperText: qsTr("A \%1 és \%2 jelölik a generált válaszlehetőségek helyét")
		field: "placeholder"
		width: parent.width
		visible: isBlock || isBinding

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextField {
		id: _correctAnswer
		title: qsTr("Helyes válasz")
		placeholderText: qsTr("Ez lesz az egyetlen helyes válasz")
		helperText: qsTr("A két válaszrészt '%1' választja el").arg(_separator.text.trim())
		field: "correct"
		width: parent.width
		visible: !isBinding && !isBlock

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextArea {
		id: _answers
		title: qsTr("Helytelen válaszok")
		placeholderText: qsTr("Lehetséges helytelen válaszok (soronként)")
		helperText: qsTr("A két válaszrészt '%1' választja el").arg(_separator.text.trim())
		width: parent.width
		visible: !isBinding && !isBlock

		field: "answers"
		getData: function() { return text.split("\n") }

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormTextField {
		id: _separator
		title: qsTr("Elválasztójel")
		placeholderText: qsTr("Ez a jel választja el a válaszok 1. és 2. részét")
		field: "separator"
		width: parent.width

		text: qsTr(",")

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormComboBox {
		id: _modeBinding
		text: qsTr("Kérdések készítése:")

		visible: isBinding

		combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 200*Qaterial.Style.pixelSizeRatio))

		field: "mode"

		valueRole: "value"
		textRole: "text"

		model: ListModel {
			ListElement {value: "left"; text: qsTr("Bal oldaliakhoz")}
			ListElement {value: "right"; text: qsTr("Jobb oldaliakhoz")}
		}

		combo.onActivated: if (objectiveEditor) objectiveEditor.previewRefresh()
	}




	QFormCheckButton {
		id: _checkMonospace
		field: "monospace"
		text: qsTr("Válaszlehetőségek monospace betűtípussal")
	}

	QFormSpinBox {
		id: _spinOptionsA
		field: "maxOptionsA"
		text: qsTr("1. válaszlehetőségek száma:")

		from: 2
		to: 7
		stepSize: 1
		value: 4

		spin.editable: true
	}

	QFormSpinBox {
		id: _spinOptionsB
		field: "maxOptionsB"
		text: qsTr("2. válaszlehetőségek száma:")

		from: 2
		to: 7
		stepSize: 1
		value: 4

		spin.editable: true
	}


	MapEditorSpinStorageCount {
		id: _countBinding
		visible: isBinding || isBlock
	}



	function loadData() {
		let _items = isBlock ?
				[_questionBlock, _spinOptionsA, _spinOptionsB, _separator, _checkMonospace, _questionPrint] :
				isBinding ?
					[_questionBlock, _spinOptionsA, _spinOptionsB, _separator, _checkMonospace, _questionPrint, _modeBinding] :
					[_question, _correctAnswer, _answers, _spinOptionsA, _spinOptionsB, _separator, _checkMonospace, _questionPrint]

		if (isMergeBinding)
			_items.push(_sectionSelector)

		_countBinding.value = objective.storageCount
		setItems(_items, objective.data)

		if (!isBinding && !isBlock && objective.data.answers !== undefined)
			_answers.fieldData = objective.data.answers.join("\n")

	}


	function saveData() {
		objective.data = previewData()
		objective.storageCount = _countBinding.value
	}



	function previewData() {
		let _items = isBlock ?
				[_questionBlock, _spinOptionsA, _spinOptionsB, _separator, _checkMonospace, _questionPrint] :
				isBinding ?
					[_questionBlock, _spinOptionsA, _spinOptionsB, _separator, _checkMonospace, _questionPrint, _modeBinding] :
					[_question, _correctAnswer, _answers, _spinOptionsA, _spinOptionsB, _separator, _checkMonospace, _questionPrint]

		if (isMergeBinding)
			_items.push(_sectionSelector)

		return getItems(_items)
	}

}




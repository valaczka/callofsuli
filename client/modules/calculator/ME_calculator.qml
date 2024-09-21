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

	readonly property bool isPlusminus: storage && storage.module == "plusminus"
	readonly property bool isNumbers: storage && storage.module == "numbers"


	QFormTextField {
		id: _question
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		helperText: isNumbers ? qsTr("A \%1 jelöli a generált elem helyét") : ""
		text: isNumbers ? "%1" : ""
		field: "question"
		width: parent.width
		visible: !isPlusminus

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormTextField {
		id: _answer
		title: qsTr("Numerikus válasz")
		placeholderText: qsTr("Helyes válasz (szám)")
		field: "answer"
		width: parent.width
		visible: !isPlusminus && !isNumbers

		getData: function() { return Number(text) }

		IntValidator {
			id: _validatorInt
			bottom: -999999
			top: 999999
		}

		DoubleValidator {
			id: _validatorDouble
			bottom: -999999
			top: 999999
			decimals: 4
			notation: DoubleValidator.StandardNotation
			locale: "en_US"
		}

		validator: _decimals.checked ? _validatorDouble : _validatorInt

		inputMethodHints: _decimals.checked ? Qt.ImhFormattedNumbersOnly : Qt.ImhDigitsOnly

		onEditingFinished:  if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormTextField {
		id: _suffix
		title: qsTr("Mértékegység")
		placeholderText: qsTr("A beviteli mező után feltüntetett mértékegység (opcionális)")
		field: "suffix"
		width: parent.width
		visible: !isPlusminus

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormCheckButton {
		id: _decimals
		text: qsTr("Tizedes számok engedélyezése")
		field: "decimals"

		visible: !isPlusminus

		onToggled: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormComboBox {
		id: _subtract
		text: qsTr("Kérdések készítése:")

		visible: isPlusminus

		combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 200*Qaterial.Style.pixelSizeRatio))

		field: "subtract"

		valueRole: "value"
		textRole: "text"

		model: [
			{value: false, text: qsTr("Összeadás")},
			{value: true, text: qsTr("Kivonás")},
		]

		combo.onActivated: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormComboBox {
		id: _range
		text: qsTr("Tartomány:")

		visible: isPlusminus

		combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 200*Qaterial.Style.pixelSizeRatio))

		field: "range"

		valueRole: "value"
		textRole: "text"

		model: [
			{value: 1, text: qsTr("0-10 között")},
			{value: 2, text: qsTr("0-20 között")},
			{value: 3, text: qsTr("0-50 között")},
			{value: 4, text: qsTr("0-100 között")}
		]

		combo.onActivated: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormComboBox {
		id: _negative
		text: qsTr("Negatív számok:")

		visible: isPlusminus

		combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 350*Qaterial.Style.pixelSizeRatio))

		field: "canNegative"

		valueRole: "value"
		textRole: "text"

		model: [
			{value: 0, text: qsTr("Semmi sem lehet negatív")},
			{value: 1, text: qsTr("Az eredmény lehet negatív")},
			{value: 2, text: qsTr("Az eredmény és a feladat is lehet negatív")},
		]


		combo.onActivated: if (objectiveEditor) objectiveEditor.previewRefresh()
	}




	MapEditorSpinStorageCount {
		id: _countBinding
		visible: isNumbers || isPlusminus
	}




	function loadData() {
		let _items = isPlusminus ? [_subtract, _range, _negative] :
								   isNumbers ? [_question, _suffix, _decimals] :
											   [_question, _answer, _suffix, _decimals]

		_countBinding.value = objective.storageCount
		setItems(_items, objective.data)
	}


	function saveData() {
		objective.data = previewData()
		objective.storageCount = _countBinding.value
	}



	function previewData() {
		let _items = isPlusminus ? [_subtract, _range, _negative] :
								   isNumbers ? [_question, _suffix, _decimals] :
											   [_question, _answer, _suffix, _decimals]

		return getItems(_items)
	}

}








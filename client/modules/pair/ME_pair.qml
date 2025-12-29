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

	readonly property bool isBinding: storage && (storage.module == "binding" || storage.module == "numbers" ||
												  storage.module == "mergebinding")
	readonly property bool isBlock: storage && (storage.module == "block" || storage.module == "mergeblock")
	readonly property bool isMergeBinding: storage && (storage.module == "mergebinding" || storage.module == "mergeblock")

	QFormTextField {
		id: _question
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		field: "question"
		width: parent.width

		text: qsTr("Rendezd össze a párokat!")

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormSection {
		width: parent.width
		text: qsTr("Párok")
		icon.source: Qaterial.Icons.cards

		visible: !isBinding && !isBlock
	}

	QFormSectionSelector {
		id: _sectionSelector

		form: root
		field: "sections"
		visible: isMergeBinding
		storage: root.storage
	}

	QFormBindingField {
		id: _binding
		width: parent.width

		defaultLeftData: ""
		defaultRightData: ""

		visible: !isBinding && !isBlock

		readOnly: false

		leftComponent: QFormBindingTextField {
			bindingField: _binding

			onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
		}

		rightComponent: QFormBindingTextField {
			bindingField: _binding

			onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
		}
	}

	QFormSpinBox {
		id: _spinCount
		field: "count"
		text: qsTr("Párok száma:")

		from: 2
		value: 4
		to: 99
		spin.editable: true

		spin.onValueModified: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormSpinBox {
		id: _spinOptions
		field: "optionsCount"
		text: qsTr("Válaszlehetőségek száma:")

		from: _spinCount.value
		value: 5
		to: 99
		spin.editable: true

		spin.onValueModified: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormComboBox {
		id: _modeOrder
		text: qsTr("Párok készítése:")

		combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 200*Qaterial.Style.pixelSizeRatio))

		field: "mode"

		valueRole: "value"
		textRole: "text"

		model: ListModel {
			ListElement {value: "first"; text: qsTr("Bal oldaliakhoz")}
			ListElement {value: "second"; text: qsTr("Jobb oldaliakhoz")}
			ListElement {value: "both"; text: qsTr("Véletlenszerű oldaliakhoz")}
			ListElement {value: "shuffle"; text: qsTr("Keverve")}
		}

		combo.onActivated: if (objectiveEditor) objectiveEditor.previewRefresh()
	}

	QFormCheckButton {
		id: _checkLinebreak
		field: "break"
		text: qsTr("Nyomtatásban a válaszlehetőségek több sorba")
	}

	MapEditorSpinStorageCount {
		id: _countBinding
		visible: isBinding || isBlock
	}




	function loadData() {
		let _items = isBlock ? [_question, _spinOptions, _modeOrder, _spinCount, _checkLinebreak] :
							   [_question, _spinOptions, _modeOrder, _spinCount, _checkLinebreak]

		if (isMergeBinding)
			_items.push(_sectionSelector)

		_countBinding.value = objective.storageCount
		setItems(_items, objective.data)

		if (!isBinding && !isBlock && objective.data.pairs !== undefined)
			_binding.loadFromList(objective.data.pairs)
		else
			_binding.loadFromList([])
	}


	function saveData() {
		objective.data = previewData()
		objective.storageCount = _countBinding.value
	}



	function previewData() {
		let _items = isBlock ? [_question, _spinOptions, _modeOrder, _spinCount, _checkLinebreak] :
							   [_question, _spinOptions, _modeOrder, _spinCount, _checkLinebreak]

		if (isMergeBinding)
			_items.push(_sectionSelector)

		let d = getItems(_items)

		if (!isBinding && !isBlock)
			d.pairs = _binding.saveToList()

		return d
	}
}



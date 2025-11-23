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

	readonly property bool isBinding: storage && storage.module == "binding"
	readonly property bool isBlock: storage && storage.module == "block"

	QFormSection {
		text: qsTr("Adatbank nélkül ez a feladattípus nem használható!")
		icon.source: Qaterial.Icons.alertCircle
		color: Qaterial.Colors.red500

		visible: !isBinding && !isBlock
	}

	QFormComboBox {
		id: _modeBinding
		text: qsTr("Kérdések készítése:")

		visible: isBinding

		combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 200*Qaterial.Style.pixelSizeRatio))

		field: "mode"

		valueRole: "value"
		textRole: "text"

		model: [
			{value: "left", text: qsTr("Bal oldaliakhoz")},
			{value: "right", text: qsTr("Jobb oldaliakhoz")}
		]

		combo.onActivated: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	QFormTextField {
		id: _questionBinding
		title: qsTr("Kérdés")
		placeholderText: qsTr("Ez a kérdés fog megjelenni")
		helperText: qsTr("A \%1 jelöli az elem helyét")
		field: "question"
		width: parent.width
		visible: isBinding || isBlock

		onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
	}


	MapEditorSpinStorageCount {
		id: _countBinding
		visible: isBinding || isBlock
	}



	function loadData() {
		let _items = isBinding ? [_modeBinding, _questionBinding] :
								 isBlock ? [_questionBinding] :
										   []


		_countBinding.value = objective.storageCount
		setItems(_items, objective.data)
	}


	function saveData() {
		objective.data = previewData()
		objective.storageCount = _countBinding.value
	}


	function previewData() {
		let _items = isBinding ? [_modeBinding, _questionBinding] :
								 isBlock ? [_questionBinding] :
										   []

		return getItems(_items)
	}
}






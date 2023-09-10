import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS


QFormColumn {
	id: root

	width: parent.width

	property Item objectiveEditor: null
	property MapEditorStorage storage: null
	property MapEditorObjective objective: null

	spacing: 10

	property alias readOnly: _binding.readOnly

	onModifiedChanged: if (objectiveEditor) objectiveEditor.modified = true

	QFormTextField {
		id: _title
		title: qsTr("Név")
		width: parent.width

		field: "name"

		enabled: !root.readOnly

		placeholderText: qsTr("Adatbank elnevezése")
		leadingIconSource: Qaterial.Icons.renameBox
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldClearButton {  }
		}
	}

	QFormSection {
		width: parent.width
		text: qsTr("Képek")
		icon.source: Qaterial.Icons.imageMultiple
	}

	QFormBindingField {
		id: _binding
		width: parent.width

		defaultLeftData: ""
		defaultRightData: -1
		split: 0.7


		leftComponent: QFormBindingTextField {
			bindingField: _binding

			onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()
		}

		rightComponent: MapEditorFormImage {
			bindingField: _binding
			editor: objectiveEditor.editor

			onModified: loadData(id)

		}

		function importFromDir() {
			if (!objectiveEditor.editor)
				return

			if (Qt.platform.os == "wasm") {
				Client.messageError(qsTr("Platform error"), qsTr("Belső hiba"))
			} else
				Qaterial.DialogManager.openFromComponent(_importDialog)
		}
	}

	Component {
		id: _importDialog

		QFileDialog {
			title: qsTr("Képek importálása")
			isDirectorySelect: true
			onDirectorySelected: {
				let list = objectiveEditor.editor.uploadImageDirectory(dir)

				if (list.length) {
					for (let i=0; i<list.length; ++i) {
						_binding._addItem(list[i].name, list[i].id)
					}

					root.modified = true
					Client.Utils.settingsSet("folder/mapEditor", dir.toString())
				}

				Client.snack(qsTr("%1 kép importálva").arg(list.length))
			}

			folder: Client.Utils.settingsGet("folder/mapEditor", "")
		}
	}


	QButton {
		id: _importButton
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("Importálás")
		icon.source: Qaterial.Icons.imageMultipleOutline
		outlined: true
		flat: true
		visible: !root.readOnly && Qt.platform.os != "wasm"
		foregroundColor: Qaterial.Colors.green400
		onClicked: {
			_binding.importFromDir()
		}
	}


	function saveData() {
		if (!storage)
			return

		storage.data = previewData()
	}

	function loadData() {
		if (storage && storage.data.images)
			_binding.loadFromList(storage.data.images)
		else
			_binding.loadFromList([])

		if (storage && storage.storageid <= 0)
			_binding.readOnly = false

		if (storage)
			setItems([_title], storage.data)
	}


	function previewData() {
		let d = getItems([_title])
		d.images = _binding.saveToList()

		return d
	}
}







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
		text: qsTr("Párok")
		icon.source: Qaterial.Icons.cards
	}

	QFormBindingField {
		id: _binding
		width: parent.width

		defaultLeftData: ""
		defaultRightData: ""

		leftComponent: QFormBindingTextField {
			bindingField: _binding

			onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()

			Keys.onPressed: event => {
								if (event.matches(StandardKey.Paste)) {
									if (pasteData())
									event.accepted = true
									return
								}

								if (event.matches(StandardKey.Copy)) {
									if (selectionStart != selectionEnd) {
										return
									}

									copyData()
									event.accepted = true
									return
								}
							}
		}

		rightComponent: QFormBindingTextField {
			bindingField: _binding

			onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()

			Keys.onPressed: event => {
								if (event.matches(StandardKey.Paste)) {
									if (pasteData())
									event.accepted = true
									return
								}

								if (event.matches(StandardKey.Copy)) {
									if (selectionStart != selectionEnd) {
										return
									}

									copyData()
									event.accepted = true
									return
								}
							}
		}
	}



	function pasteData() {
		let txt = Client.Utils.clipboardText()

		if (!txt || txt === undefined || txt === "") {
			Client.messageWarning(qsTr("A vágólap tartalma üres vagy érvénytelen"))
			return false
		}

		let d = {}

		try {
			d = JSON.parse(txt)
		} catch (error) {
			let found = false

			let rows = txt.split("\n")

			for (let i=0; i<rows.length; ++i) {
				let cols = rows[i].split("\t")

				if (cols.length < 2)
					continue

				_binding._addItem(cols[0], cols[1])
				found = true
			}


			if (found)
				root.modified = true

			return found
		}


		if (d.type === undefined || d.type !== "binding") {
			Client.snack(qsTr("Érvénytelen tartalom a vágólapon"))
			return false
		}

		for (let j=0; j<d.bindings.length; ++j) {
			_binding._addItem(d.bindings[j].first, d.bindings[j].second)
		}

		if (d.bindings.length>0)
			Client.snack(qsTr("%1 elem beillesztve").arg(d.bindings.length))

		root.modified = true

		return true
	}



	function copyData() {
		let d = previewData()
		d.type = "binding"

		Client.Utils.setClipboardText(JSON.stringify(d))

		Client.snack(qsTr("Tartalom a vágólapra másolva"))

		console.debug(JSON.stringify(d))
	}


	function saveData() {
		if (!storage)
			return

		storage.data = previewData()
	}

	function loadData() {
		if (storage && storage.data.bindings)
			_binding.loadFromList(storage.data.bindings)
		else
			_binding.loadFromList([])

		if (storage && storage.storageid <= 0)
			_binding.readOnly = false

		if (storage)
			setItems([_title], storage.data)
	}


	function previewData() {
		let d = getItems([_title])
		d.bindings = _binding.saveToList()

		return d
	}
}

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
		text: qsTr("Halmazok")
		icon.source: Qaterial.Icons.cards
	}

	QFormBindingField {
		id: _binding
		width: parent.width

		defaultLeftData: ""
		defaultRightData: ""

		leftComponent: QFormBindingTextField {
			bindingField: _binding

			title: qsTr("Név")
			onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()

			Keys.onPressed: event => {
								if (event.matches(StandardKey.Paste)) {
									if (pasteData())
									event.accepted = true
									return
								}

								if (event.matches(StandardKey.Copy)) {
									if (selectionStart !== selectionEnd) {
										return
									}

									copyData()
									event.accepted = true
									return
								}
							}
		}

		rightComponent: QFormBindingTextArea {
			bindingField: _binding

			onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()

			saveData: function(d) { return text.split("\n") }

			function loadData(d) {
				if (d && d.length)
					text = d.join("\n")
			}

			title: qsTr("Elemek")
			placeholderText: qsTr("A tömb elemei soronként")

			contentItem.Keys.onPressed: event => {
											if (event.matches(StandardKey.Paste)) {
												if (pasteData())
												event.accepted = true
												return
											}

											if (event.matches(StandardKey.Copy)) {
												if (contentItem.selectionStart !== contentItem.selectionEnd) {
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
		let d = {}

		try {
			d = JSON.parse(Client.Utils.clipboardText())
		} catch (error) {
			return false
		}

		if (d.type === undefined || d.type !== "blocks") {
			Client.snack(qsTr("Érvénytelen tartalom a vágólapon"))
			return false
		}

		for (let j=0; j<d.blocks.length; ++j) {
			_binding._addItem(d.blocks[j].first, d.blocks[j].second)
		}

		if (d.blocks.length>0) {
			Client.snack(qsTr("%1 elem beillesztve").arg(d.blocks.length))
			root.modified = true
		}

		return true
	}



	function copyData() {
		let d = previewData()
		d.type = "blocks"

		Client.Utils.setClipboardText(JSON.stringify(d))

		Client.snack(qsTr("Tartalom a vágólapra másolva"))
	}



	function saveData() {
		if (!storage)
			return

		storage.data = previewData()
	}

	function loadData() {
		if (storage && storage.data.blocks)
			_binding.loadFromList(storage.data.blocks)
		else
			_binding.loadFromList([])

		if (storage && storage.storageid <= 0)
			_binding.readOnly = false

		if (storage)
			setItems([_title], storage.data)
	}


	function previewData() {
		let d = getItems([_title])
		d.blocks = _binding.saveToList()

		return d
	}
}

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

	property bool readOnly: true

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

	Repeater {
		id: _repeater

		model: []

		delegate: Column {
			readonly property alias title: _sectionTitle.text
			readonly property alias binding: _binding

			width: parent.width

			bottomPadding: 15 * Qaterial.Style.pixelSizeRatio

			Qaterial.TextField {
				id: _sectionTitle
				width: parent.width

				enabled: !root.readOnly

				onTextEdited: root.modified = true
				text: modelData.name

				placeholderText: qsTr("Szakasz #%1 elnevezése").arg(index+1)
				leadingIconSource: Qaterial.Icons.cards
				trailingContent: Qaterial.TextFieldButtonContainer
				{
					Qaterial.TextFieldClearButton {  }

					Qaterial.TextFieldIconButton {
						visible: !root.readOnly

						icon.source: Qaterial.Icons.deleteForeverOutline
						icon.color: Qaterial.Colors.red400

						onClicked: {
							root.modified = true

							let d = _repeater.getData()
							d.splice(index, 1)
							_repeater.model = d
						}
					}
				}
			}


			QFormBindingField {
				id: _binding
				width: parent.width

				readOnly: root.readOnly
				form: root

				defaultLeftData: ""
				defaultRightData: ""

				leftComponent: QFormBindingTextField {
					bindingField: _binding

					onEditingFinished: if (objectiveEditor) objectiveEditor.previewRefresh()

					Keys.onPressed: event => {
										if (event.matches(StandardKey.Paste)) {
											if (pasteData(_binding))
											event.accepted = true
											return
										}

										if (event.matches(StandardKey.Copy)) {
											if (selectionStart != selectionEnd) {
												return
											}

											copyData(_binding)
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
											if (pasteData(_binding))
											event.accepted = true
											return
										}

										if (event.matches(StandardKey.Copy)) {
											if (selectionStart != selectionEnd) {
												return
											}

											copyData(_binding)
											event.accepted = true
											return
										}
									}
				}
			}

			function load() {
				_binding.defaultCount = 0
				_binding.loadFromList(modelData.bindings)
			}
		}

		onItemAdded: (index, item) => item.load()

		function getData() {
			let list = []

			for (let i=0; i<model.length; ++i) {
				let item = itemAt(i)

				if (i>=count || !item)
					continue

				list.push({
							  key: model[i].key,
							  name: item.title,
							  bindings: item.binding.saveToList()
						  })

			}

			return list
		}
	}



	Item {
		width: parent.width
		height: 10
	}

	QButton {
		id: _addButton
		anchors.left: parent.left
		text: qsTr("Új szakasz hozzáadása")
		icon.source: Qaterial.Icons.plus
		outlined: true
		flat: true
		visible: !root.readOnly
		foregroundColor: Qaterial.Colors.green400
		onClicked: {
			let d = _repeater.getData()

			d.push(createEmpty())

			_repeater.model = d

			root.modified = true
		}
	}




	function pasteData(binding) {
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

				binding._addItem(cols[0], cols[1])
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
			binding._addItem(d.bindings[j].first, d.bindings[j].second)
		}

		if (d.bindings.length>0)
			Client.snack(qsTr("%1 elem beillesztve").arg(d.bindings.length))

		root.modified = true

		return true
	}



	function copyData(binding) {
		let d = {}
		d.bindings = binding.saveToList()
		d.type = "binding"

		Client.Utils.setClipboardText(JSON.stringify(d))

		Client.snack(qsTr("Tartalom a vágólapra másolva"))
	}


	function saveData() {
		if (!storage)
			return

		storage.data = previewData()
	}


	function createEmpty() {
		let d = {
					 key: Client.Utils.createUuid(),
					 name: "",
					 bindings: [
						 { first: "", second: "" },
						 { first: "", second: "" },
						 { first: "", second: "" },
						 { first: "", second: "" },
						 { first: "", second: "" }
					 ]
				 }

		return d
	}

	function loadData() {
		if (storage && storage.data.sections)
			_repeater.model = storage.data.sections
		else {
			let d =	[]
			d.push(createEmpty())

			_repeater.model = d
		}


		if (storage && storage.storageid <= 0)
			root.readOnly = false

		if (storage)
			setItems([_title], storage.data)
	}


	function previewData() {
		let d = getItems([_title])
		d.sections = _repeater.getData()

		return d
	}
}

import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

QtObject
{
	id: root
	//onAccepted: _control.accepted(currentIndex)
	//onRejected: _control.rejected()

	property MapEditor editor: null


	property MapEditorChapter _chapter: null
	property string _objectiveModule: ""
	property string _storageModule: ""
	property MapEditorStorage _storage: null


	property Connections _connections: Connections {
		target: editor

		function onObjectiveDialogRequest(chapter) { loadDialog(chapter) }
	}

	property Component _delegate: Qaterial.ItemDelegate
	{
		text: modelData.text ? modelData.text : ""
		secondaryText: modelData.secondaryText ? modelData.secondaryText : ""
		icon.source: modelData.icon ? modelData.icon : ""
		iconColor: modelData.iconColor ? modelData.iconColor : Qaterial.Colors.cyan400
		width: ListView.view.width
		onClicked: ListView.view.select(index)
	}

	function loadDialog(chapter) {
		if (!editor)
			return

		_chapter = chapter
		_objectiveModule = ""
		_storageModule = ""
		_storage = null

		_loadObjectiveDialog()
	}



	function _loadObjectiveDialog() {
		if (!_chapter)
			return

		let list = editor.objectiveListModel()

		if (list.length === 0)
			return

		Qaterial.DialogManager.openListView(
					{
						onAccepted: function(index)
						{
							if (index < 0)
								return

							_loadStorageDialog(list[index].module)

						},
						title: qsTr("Új feladat"),
						model: list,
						delegate: _delegate
					})
	}




	function _loadStorageDialog(_module) {
		if (!_chapter)
			return

		_objectiveModule = _module

		let list = editor.storageListModel(_module)

		if (list.length === 0) {
			_loadEditor()
			return
		}

		let slist = []

		slist.push({
					   text: qsTr("Adatbank nélkül"),
					   icon: Qaterial.Icons.databaseOffOutline,
					   iconColor: Qaterial.Style.primaryTextColor(),
					   module: ""
				   })

		for (let i=0; i<list.length; ++i) {
			let s = list[i]
			s.iconColor = Qaterial.Colors.green400
			slist.push(s)
		}


		Qaterial.DialogManager.openListView(
					{
						onAccepted: function(index)
						{
							if (index < 0)
								return

							_loadStorageSelectorDialog(slist[index])

						},
						title: qsTr("Adatbank típus kiválasztása"),
						model: slist,
						delegate: _delegate
					})
	}



	function _loadStorageSelectorDialog(_module) {
		if (!_chapter || !_objectiveModule)
			return

		_storageModule = _module.module

		let list = editor.storageModel(_module.module)

		if (list.length === 0) {
			_loadEditor()
			return
		}

		let slist = []

		slist.push({
					   text: qsTr("Új létrehozása"),
					   icon: Qaterial.Icons.plus,
					   iconColor: Qaterial.Colors.green500,
					   storage: null
				   })

		for (let i=0; i<list.length; ++i) {
			let s = list[i]
			slist.push({
						   text: s.title,
						   secondaryText: s.details,
						   icon: "",
						   iconColor: Qaterial.Colors.cyan500,
						   storage: s.storage
					   })
		}


		Qaterial.DialogManager.openListView(
					{
						onAccepted: function(index)
						{
							if (index < 0)
								return

							let ml = slist[index]

							_storage = ml.storage

							_loadEditor()
						},
						title: qsTr("%1 kiválasztása").arg(_module.text),
						model: slist,
						delegate: _delegate
					})
	}




	function _loadEditor() {
		if (!_chapter || !_objectiveModule)
			return

		editor.objectiveLoadEditor(_chapter, _objectiveModule, _storageModule, _storage)
	}
}

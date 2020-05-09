import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Map map: null

	title: pageChapterEditor.objectiveId === -1 ? qsTr("Töltény") : qsTr("Fegyver")

	Label {
		id: noLabel
		opacity: pageChapterEditor.storageId === -1 && pageChapterEditor.objectiveId === -1
		visible: opacity != 0

		text: pageChapterEditor.objectiveId === -1 ? qsTr("Válassz töltényt") : qsTr("Válassz fegyvert")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}

	Loader {
		id: editorLoader
		anchors.fill: parent
		opacity: (pageChapterEditor.storageId !== -1 || pageChapterEditor.objectiveId !== -1) && editorLoader.status === Loader.Ready
		visible: opacity != 0
	}


	Connections {
		target: pageChapterEditor
		onReloadObjective:  get()
	}


	function populated() { get() }


	function get() {
		var d = {}
		if (pageChapterEditor.storageId !== -1) {
			d = map.storageGet(pageChapterEditor.storageId)
			var i = map.storageInfo(d.module)
			if (Object.keys(i).length) {
				var qml = "MOD"+i.type+"Editor.qml"
				if (editorLoader.source === qml)
					editorLoader.item.jsonData = d.data
				else
					editorLoader.setSource(qml, { jsonData: d.data, moduleLabel: i.label })
			} else {
				cosClient.sendMessageError(qsTr("Programhiba"), qsTr("Érvénytelen modul"), d.module)
			}
		} else if (pageChapterEditor.objectiveId !== -1) {
			d = map.objectiveGet(pageChapterEditor.objectiveId)

			i = map.objectiveInfo(d.module)
			if (Object.keys(i).length) {
				qml = "MOD"+i.type+"Editor.qml"
				if (editorLoader.source === qml) {
					editorLoader.item.level = d.level
					editorLoader.item.isSummary = d.isSummary
					editorLoader.item.storageData = d.storageData
					editorLoader.item.storageModule = d.storageModule
					editorLoader.item.jsonData = d.data
				} else
					editorLoader.setSource(qml, { moduleLabel: i.label, storageModule: d.storageModule, storageData: d.storageData,
											   jsonData: d.data, level: d.level, isSummary: d.isSummary })
			} else {
				cosClient.sendMessageError(qsTr("Programhiba"), qsTr("Érvénytelen modul"), d.module)
			}
		} else {
			editorLoader.source = ""
		}
	}
}

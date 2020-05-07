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
	property int storageId: -1
	property int objectiveId: -1

	title: objectiveId === -1 ? qsTr("Töltény") : qsTr("Fegyver")

	Label {
		id: noLabel
		opacity: storageId == -1 && objectiveId == -1
		visible: opacity != 0

		text: objectiveId === -1 ? qsTr("Válassz töltényt") : qsTr("Válassz fegyvert")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}

	ColumnLayout {
		id: lay
		anchors.fill: parent
		opacity: (storageId != -1 || objectiveId != -1) //&& editorLoader.status === Loader.Ready
		visible: opacity != 0

		RowLayout {
			Label {
				id: labelModule

				Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

				Layout.fillHeight: false
				Layout.fillWidth: true
			}

			QButton {
				id: buttonDelete
				text: objectiveId != -1 ? qsTr("Fegyver törlése") : qsTr("Töltény törlése")

				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

				Layout.fillHeight: false
				Layout.fillWidth: false


				icon.source: CosStyle.iconDelete
				backgroundColor: CosStyle.colorErrorDarker
				borderColor: CosStyle.colorErrorDark
				textColor: CosStyle.colorErrorLight

				onClicked: {
					var d = JS.dialogCreateQml("YesNo")
					d.item.title = objectiveId != -1 ? qsTr("Biztosan törlöd a fegyvert?") : qsTr("Biztosan törlöd a töltényt?")
					d.item.text = labelModule.text
					d.accepted.connect(function () {
						if (objectiveId != -1) {
							map.undoLogBegin(qsTr("Fegyver törlése"))
							if (map.objectiveRemove(objectiveId)) {
								objectiveId = -1
								storageId = -1
								get()
							}
							map.undoLogEnd()
						} else if (storageId != -1) {
							map.undoLogBegin(qsTr("Töltény törlése"))
							if (map.storageRemove(storageId)) {
								objectiveId = -1
								storageId = -1
								get()
							}
							map.undoLogEnd()
						}
					})
					d.open()
				}

			}
		}


		Loader {
			id: editorLoader


			Behavior on opacity { NumberAnimation { duration: 125 } }
		}
	}


	Connections {
		target: editorLoader.item
		onSave: if (storageId !== -1) {
					map.undoLogBegin(qsTr("Töltény módosítása"))
					map.storageDataSet(storageId, jsondata)
					map.undoLogEnd()
				} else if (objectiveId !== -1) {
					map.undoLogBegin(qsTr("Fegyver módosítása"))
					map.objectiveDataSet(objectiveId, jsondata)
					map.undoLogEnd()
				}

	}



	Connections {
		target: pageChapterEditor
		onStorageSelected: {
			storageId = id
			objectiveId = -1
			get()
		}

		onObjectiveSelected: {
			storageId = -1
			objectiveId = id
			get()
		}
	}


	Connections {
		target: map
		onUndone: get()
	}

	function populated() { get() }


	function get() {
		var d = {}
		if (storageId != -1) {
			d = map.storageGet(storageId)
			var i = map.storageInfo(d.module)
			labelModule.text = i.label
		} else if (objectiveId != -1) {
			d = map.objectiveGet(objectiveId)
			i = map.objectiveInfo(d.module)
			labelModule.text = i.label
		}
	}
}

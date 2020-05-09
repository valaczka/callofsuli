import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QAccordion {
	id: control

	property var jsonData: null
	property var storageData: null
	property string storageModule: ""
	property alias level: spinLevel.value
	property alias isSummary: checkSummary.checked
	property alias moduleLabel: labelModule.text


	QCollapsible {
		title: qsTr("Általános")

		Row {
			id: row1
			spacing: 5

			width: parent.width

			Label {
				id: labelModule
				anchors.verticalCenter: parent.verticalCenter

				width: row1.width-row1.spacing-buttonDelete.width
			}

			QButton {
				id: buttonDelete
				text: pageChapterEditor.objectiveId !== -1 ? qsTr("Fegyver törlése") : qsTr("Töltény törlése")

				anchors.verticalCenter: parent.verticalCenter

				icon.source: CosStyle.iconDelete
				themeColors: CosStyle.buttonThemeDelete

				onClicked: {
					var d = JS.dialogCreateQml("YesNo")
					d.item.title = pageChapterEditor.objectiveId !== -1 ? qsTr("Biztosan törlöd a fegyvert?") : qsTr("Biztosan törlöd a töltényt?")
					d.item.text = moduleLabel
					d.accepted.connect(function () {
						if (pageChapterEditor.objectiveId !== -1) {
							var i = pageChapterEditor.objectiveId
							pageChapterEditor.objectiveId = -1
							map.undoLogBegin(qsTr("Fegyver törlése"))
							map.objectiveRemove(i)
							map.undoLogEnd()
						} else if (pageChapterEditor.storageId !== -1) {
							i = pageChapterEditor.storageId
							pageChapterEditor.storageId = -1
							map.undoLogBegin(qsTr("Töltény törlése"))
							map.storageRemove(i)
							map.undoLogEnd()
						}
					})
					d.open()
				}
			}
		}
	}

	QCollapsible {
		title: qsTr("Fegyver beállításai")
		visible: pageChapterEditor.objectiveId !== -1

		Row {
			Label {
				anchors.verticalCenter: parent.verticalCenter
				text: qsTr("Szint:")
			}

			QSpinBox {
				id: spinLevel
				anchors.verticalCenter: parent.verticalCenter
				ToolTip.text: qsTr("Szint")
				from: 1
				to: 99

				onValueModified: saveJson()
			}

			QCheckBox {
				id: checkSummary
				text: qsTr("Összegzéshez használ")

				onToggled: saveJson()
			}
		}
	}




	function saveJson() {
		if (pageChapterEditor.storageId !== -1) {
			map.undoLogBegin(qsTr("Töltény módosítása"))
			map.storageUpdate(pageChapterEditor.storageId, {}, saveJsonData(), pageChapterEditor.chapterid)
			map.undoLogEnd()
		} else if (pageChapterEditor.objectiveId !== -1) {
			map.undoLogBegin(qsTr("Fegyver módosítása"))
			map.objectiveUpdate(pageChapterEditor.objectiveId, {level: level, isSummary: isSummary}, saveJsonData(), pageChapterEditor.chapterid)
			map.undoLogEnd()
		}
	}
}

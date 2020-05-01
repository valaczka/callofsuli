import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Map map: null
	property int storageId: -1
	property int objectiveId: -1

	title: objectiveId === -1 ? qsTr("Lőszer") : qsTr("Fegyver")

	Label {
		id: noLabel
		opacity: storageId == -1 && objectiveId == -1
		visible: opacity != 0

		text: objectiveId === -1 ? qsTr("Válassz lőszert") : qsTr("Válassz fegyvert")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}


	QAccordion {
		anchors.fill: parent
		opacity: storageId != -1 || objectiveId != -1
		visible: opacity != 0

		Behavior on opacity { NumberAnimation { duration: 125 } }

		QCollapsible {
			title: qsTr("Általános")

			Column {
				width: parent.width

				QTextField {
					id: chapterName
					width: parent.width

					//onTextModified:  if (chapterId != -1) map.chapterUpdate(chapterId, { "name": chapterName.text }, parentMissionId, parentSummaryId)
				}

				QButton {
					text: objectiveId === -1 ? qsTr("Lőszer törlése") : qsTr("Fegyver törlése")

					icon.source: CosStyle.iconDelete
					backgroundColor: CosStyle.colorErrorDarker
					borderColor: CosStyle.colorErrorDark

					onClicked: {
						var d = JS.dialogCreateQml("YesNo")
						d.item.title = objectiveId === -1 ? qsTr("Biztosan törtlöd a lőszert?") : qsTr("Biztosan törlöd a fegyvert?")
						d.item.text = chapterName.text
						d.accepted.connect(function () {

						})
						d.open()
					}

				}
			}
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

	Component.onCompleted: get()


	function get() {

	}
}

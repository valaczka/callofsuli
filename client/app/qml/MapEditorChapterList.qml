import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

ListView {
	id: control
	anchors.fill: parent

	implicitHeight: 300
	implicitWidth: 650

	clip: true
	focus: true
	activeFocusOnTab: true
	boundsBehavior: Flickable.StopAtBounds
	orientation: ListView.Horizontal
	snapMode: width > implicitWidth ? ListView.SnapToItem : ListView.SnapOneItem

	highlightFollowsCurrentItem: false

	ScrollBar.horizontal: ScrollBar { }

	//Keys.onLeftPressed: decrementCurrentIndex()
	//Keys.onRightPressed: incrementCurrentIndex()


	model: SortFilterProxyModel {
		sourceModel: mapEditor.modelChapterList

		sorters: RoleSorter {
			roleName: "chapter"
		}
	}

	delegate: QSwipeContainer {
		id: item

		required property string name
		required property int chapter
		required property int missionCount
		required property int objectiveCount

		verticalPadding: 5
		horizontalPadding: 5

		width: Math.min(control.width, control.implicitWidth)
		height: control.height

		//title: qsTr("Level %1").arg(level)
		icon: CosStyle.iconTrophy

		borderColor: chapterItem.backgroundColor

		QAccordion {
			MapEditorChapter {
				id: chapterItem
				collapsed: false
				interactive: false
				level: -1
				name: item.name
				chapter: item.chapter
				missionCount: item.missionCount
				objectiveCount: item.objectiveCount
			}
		}
	}

	footer: Item {
		width: Math.min(control.implicitWidth, control.width)
		height: control.height

		Column {
			anchors.centerIn: parent

			QToolButtonBig {
				anchors.horizontalCenter: parent.horizontalCenter
				icon.source: CosStyle.iconAdd
				text: qsTr("Hozzáadás")

				onClicked: {
					var d = JS.dialogCreateQml("TextField", {
												   title: qsTr("Új szakasz"),
												   text: qsTr("Az új szakasz neve")
											   })

					d.accepted.connect(function(data) {
						if (data.length)
							mapEditor.chapterAdd({name: data})
					})
					d.open()
				}
			}

			QToolButtonBig {
				anchors.horizontalCenter: parent.horizontalCenter
				icon.source: CosStyle.iconAdd
				text: qsTr("Importálás")

				onClicked: {
					var d = JS.dialogCreateQml("File", {})
					d.item.isSave = false
					d.item.filters = ["*.xlsx", "*.xls"]

					d.accepted.connect(function(data){
						mapEditor.chapterImport({filename: data})
					})

					d.open()
				}
			}

		}
	}


	function layoutBack() {
		return false
	}
}

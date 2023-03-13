import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2

Qaterial.Page
{
	id: control

	implicitWidth: 200
	implicitHeight: 200

	background: Rectangle { color: "transparent" }

	property alias view: view

	signal classSelected(int classid)

	QListView {
		id: view

		currentIndex: 0
		anchors.fill: parent
		autoSelectChange: true

		refreshProgressVisible: msgHandler.pending
		refreshEnabled: true
		onRefreshRequest: msgHandler.sendRequestFunc(WebSocketMessage.ClassGeneral, "classList")

		model: SortFilterProxyModel {
			sourceModel: classList

			sorters: [
				FilterSorter {
					RangeFilter {
						roleName: "classid"
						maximumValue: -1
					}
					priority: 1
					sortOrder: Qt.AscendingOrder
				},
				StringSorter {
					roleName: "name"
					sortOrder: Qt.AscendingOrder
				}
			]
		}


		delegate: QItemDelegate {
			property ClassObject classObject: model.qtObject
			selectableObject: classObject

			highlighted: ListView.isCurrentItem
			//highlightedIcon: server ? server.autoConnect : false
			iconSource: Qaterial.Icons.desktopClassic
			text: classObject ? classObject.name : ""

			onClicked: if (!view.selectEnabled)
						   classSelected(classObject.classid)
		}

		Qaterial.Menu {
			id: contextMenu
			QMenuItem { action: view.actionSelectAll }
			QMenuItem { action: view.actionSelectNone }
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionClassAdd }
			QMenuItem { action: actionClassRename }
			QMenuItem { action: actionClassRemove }

		}

		onRightClickOrPressAndHold: {
			if (index != -1)
				currentIndex = index
			contextMenu.popup(mouseX, mouseY)
		}
	}

	QFabButton {
		visible: view.visible
		action: actionClassAdd
	}

}
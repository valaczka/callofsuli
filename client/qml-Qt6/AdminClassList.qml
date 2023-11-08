import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel

Qaterial.Page
{
	id: control

	implicitWidth: 200
	implicitHeight: 200

	background: null

	property alias view: view

	signal classSelected(int classid, string classname)

	QListView {
		id: view

		height: parent.height
		width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
		anchors.horizontalCenter: parent.horizontalCenter

		currentIndex: -1
		autoSelectChange: true

		refreshProgressVisible: Client.httpConnection.pending
		refreshEnabled: true
		onRefreshRequest: Client.reloadCache("classList")

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

		header: Column {
			QItemDelegate {
				text: qsTr("Minden felhasználó")
				iconSource: Qaterial.Icons.accountGroup

				width: view.width

				onClicked: classSelected(-1, "")
			}

			QItemDelegate {
				text: qsTr("Osztály nélküli felhasználók")
				iconSource: Qaterial.Icons.accountSupervisorOutline

				width: view.width

				onClicked: classSelected(-2, "")
			}
		}


		delegate: QItemDelegate {
			property ClassObject classObject: model.qtObject
			selectableObject: classObject

			highlighted: ListView.isCurrentItem
			//highlightedIcon: server ? server.autoConnect : false
			iconSource: Qaterial.Icons.accountMultiple
			text: classObject ? classObject.name : ""

			onClicked: if (!view.selectEnabled)
						   classSelected(classObject.classid, classObject.name)
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

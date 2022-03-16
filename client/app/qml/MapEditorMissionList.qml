import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabContainer {
	id: control

	title: qsTr("Küldetések")
	icon: CosStyle.iconComputer

	property int contextAction: (MapEditorAction.ActionTypeMissionList | MapEditorAction.ActionTypeMission)
	property int actionContextType: -1
	property var actionContextId: null

	signal forceMissionOpen(string missionuuid)

	QObjectListDelegateView {
		id: missionList
		anchors.fill: parent

		visible: mapEditor.editor.missions.count

		selectorSet: mapEditor.editor.missions.selectedCount

		model: SortFilterProxyModel {
			sourceModel: mapEditor.editor.missions

			sorters: StringSorter {
				roleName: "name"
			}
		}

		delegate: MapEditorMission {
			id: missionItem

			required property int index

			collapsed: true
			selectorSet: missionList.selectorSet

			onLongClicked: missionList.onDelegateLongClicked(index)
			onSelectToggled: missionList.onDelegateClicked(index, withShift)
			self: missionList.modelObject(index)

			onSelfChanged: if (!self) {
							   delete missionItem
						   }

			onMissionRemove: {
				if (mapEditor.editor.missions.selectedCount > 0) {
					mapEditor.missionRemoveList(mapEditor.editor.missions.getSelected())
				} else {
					mapEditor.missionRemove(self)
				}
			}

			/*Connections {
				target: control
				function onforceMissionOpen(chid) {
					if (!missionItem.self)
						return

					if (chid === missionItem.self.uuid)
						missionItem.collapsed = false
					else
						missionItem.collapsed = true
				}
			}*/
		}


		header: QTabHeader {
			tabContainer: control
			isPlaceholder: true
		}


		footer: QToolButtonFooter {
			width: missionList.width
			anchors.horizontalCenter: parent.horizontalCenter
			action: actionMissionNew
		}
	}

	QToolButtonBig {
		anchors.centerIn: parent
		visible: !mapEditor.editor.missions.count
		action: actionMissionNew
		color: CosStyle.colorOKLighter
	}

	Action {
		id: actionMissionNew
		icon.source: CosStyle.iconAdd
		text: qsTr("Új küldetés")

		onTriggered: {
			var d = JS.dialogCreateQml("TextField", {
										   title: qsTr("Új küldetés"),
										   text: qsTr("Az új küldetés neve")
									   })

			d.accepted.connect(function(data) {
				/*if (data.length)
					mapEditor.chapterAdd({name: data})*/
			})
			d.open()
		}
	}

	onPopulated: {
		missionList.forceActiveFocus()

		loadContextId(actionContextType, actionContextId)
		actionContextType = -1
		actionContextId = null
	}


	function loadContextId(type, id) {
		if (type === MapEditorAction.ActionTypeMissionList)
			forceMissionOpen("")
		else if (type === MapEditorAction.ActionTypeMission)
			forceMissionOpen(id)
	}


	backCallbackFunction: function () {
		/*if (mapEditor.chapterModelUnselectObjectives(mapEditor.editor.missions))
			return true*/

		if (mapEditor.editor.missions.selectedCount) {
			mapEditor.editor.missions.unselectAll()
			return true
		}

		return false
	}
}

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

	property ListModel availableTerrainModel: ListModel {}

	SortFilterProxyModel {
		id: terrainProxyModel

		sourceModel: availableTerrainModel

		filters: ValueFilter {
			roleName: "level"
			value: 1
		}

		sorters: StringSorter {
			roleName: "readableName"
		}
	}


	QObjectListDelegateView {
		id: missionList
		anchors.fill: parent

		visible: mapEditor.editor.missions.count

		selectorSet: mapEditor.editor.missions.selectedCount

		property GameMapEditorMission _openRequest: null

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

			onSelfChanged: if (self) {
							   if (missionList._openRequest == self) {
								   collapsed = false
								   missionList._openRequest = null
							   }
						   } else {
							   delete missionItem
						   }

			onMissionRemove: {
				if (mapEditor.editor.missions.selectedCount > 0) {
					mapEditor.missionRemoveList(mapEditor.editor.missions.getSelected())
				} else {
					mapEditor.missionRemove(self)
				}
			}

			Connections {
				target: mapEditor

				function onMissionOpenRequest(mis) {
					missionItem.collapsed = !(mis === self)
				}
			}
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

		Connections {
			target: mapEditor

			function onMissionOpenRequest(mis) {
				missionList._openRequest = mis
			}
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
				var dd = JS.dialogCreateQml("List", {
											   icon: CosStyle.iconLockAdd,
											   title: qsTr("Harcmező kiválasztása"),
											   selectorSet: false,
											   modelTitleRole: "readableName",
											   modelImageRole: "thumbnail",
											   delegateHeight: CosStyle.twoLineHeight*1.5,
											   model: terrainProxyModel
										   })


				dd.accepted.connect(function(data2) {
					if (!data2)
						return

					mapEditor.missionAdd({name: data}, data2.terrain)
				})
				dd.open()
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

	}


	Component.onCompleted: {
		var l = mapEditor.availableTerrains

		for (var i=0; i<l.length; i++) {
			availableTerrainModel.append(l[i])
		}
	}

	backCallbackFunction: function () {
		if (mapEditor.editor.missions.selectedCount) {
			mapEditor.editor.missions.unselectAll()
			return true
		}

		return false
	}
}

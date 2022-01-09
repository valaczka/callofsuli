import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
	id: control

	title: qsTr("Pályák")
	icon: CosStyle.iconPlanet

	property alias list: list
	property MapListObject _mapWaitForDownload: null

	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: studentMaps.modelMapList
		sorters: [
			RoleSorter { roleName: "active"; sortOrder: Qt.DescendingOrder; priority: 1 },
			StringSorter { roleName: "name"; priority: 0 }
		]

		proxyRoles: [
			SwitchRole {
				name: "textColor"
				filters: [
					ValueFilter {
						roleName: "active"
						value: true
						SwitchRole.value: CosStyle.colorOK
					}
				]
				defaultValue: CosStyle.colorAccentDarker
			},
			SwitchRole {
				name: "fontWeight"
				filters: ExpressionFilter {
					expression: model.active
					SwitchRole.value: Font.Medium
				}
				defaultValue: Font.Light
			}
		]
	}


	QIconEmpty {
		visible: userProxyModel.count == 0
		anchors.centerIn: parent
		textWidth: parent.width*0.75
		text: qsTr("Sajnos egyetlen pálya sem tartozik ehhez a csoporthoz")
		tabContainer: control
	}


	QObjectListView {
		id: list

		anchors.fill: parent

		model: userProxyModel
		modelTitleRole: "name"
		modelTitleColorRole: "textColor"
		modelTitleWeightRole: "fontWeight"
		pixelSizeTitle: CosStyle.pixelSize*1.3

		header: QTabHeader {
			tabContainer: control
			isPlaceholder: true
		}

		autoSelectorChange: true

		refreshEnabled: true

		delegateHeight: CosStyle.twoLineHeight*1.5

		leftComponent: QFontImage {
			width: visible ? list.delegateHeight : 0
			height: width*0.8
			size: height*0.8

			icon: if (model && model.downloaded) {
					  if (model.active)
						  CosStyle.iconPlanet
					  else
						  CosStyle.iconVisible
				  } else
					  CosStyle.iconDownloadCloud

			visible: model

			color: model && model.downloaded ? model.textColor : CosStyle.colorWarningLighter
		}

		rightComponent: QLabel {
			anchors.verticalCenter: parent.verticalCenter
			font.pixelSize: CosStyle.pixelSize*0.9
			font.weight: Font.DemiBold
			color: CosStyle.colorWarningLighter
			visible: model && (!model.downloaded || !model.active)
			text: model && !model.downloaded ? qsTr("még nincs letöltve") : qsTr("csak megtekinthető")
		}

		onRefreshRequest: studentMaps.send("mapListGet", { groupid: studentMaps.selectedGroupId } )

		onClicked: {
			_mapWaitForDownload = null
			var o = list.modelObject(index)

			if (o.downloaded) {
				studentMaps.mapLoad(o)
			} else {
				_mapWaitForDownload = o
				list.currentIndex = index
				actionDownload.trigger()
			}
		}

		onRightClicked: contextMenu.popup()

		onLongPressed: {
			if (selectorSet) {
				contextMenu.popup()
				return
			}
		}



		QMenu {
			id: contextMenu

			MenuItem { action: actionDownload }
		}


		//onKeyInsertPressed: actionMapNew.trigger()
		//onKeyF2Pressed: actionRename.trigger()
		/*onKeyDeletePressed: actionRemove.trigger()
		onKeyF4Pressed: actionObjectiveNew.trigger()*/
	}


	Action {
		id: actionDownload
		text: qsTr("Letöltés")
		icon.source: CosStyle.iconDownload
		enabled: !studentMaps.isBusy && (list.currentIndex !== -1 || studentMaps.modelMapList.selectedCount)
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var more = studentMaps.modelMapList.selectedCount

			if (more > 0)
				studentMaps.mapDownload(studentMaps.modelMapList.getSelected())
			else
				studentMaps.mapDownload(list.modelObject(list.currentIndex))
		}
	}


	Connections {
		target: studentMaps

		function onMapDownloadFinished(list) {
			if (list.includes(_mapWaitForDownload)) {
				studentMaps.mapLoad(_mapWaitForDownload)
			}
			_mapWaitForDownload = null
		}
	}

	onPopulated: {
		if (studentMaps.selectedGroupId > -1)
			studentMaps.send("mapListGet", {groupid: studentMaps.selectedGroupId})
	}

}




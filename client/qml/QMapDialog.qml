import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.ModalDialog
{
	id: control

	property TeacherMapHandler handler: null
	property var disabledUuids: []


	horizontalPadding: 0
	standardButtons: itemNoMap.visible ? DialogButtonBox.Close : DialogButtonBox.Cancel

	dialogImplicitWidth: 600

	signal mapSelected(string uuid, TeacherMap map)


	// Private properties

	property string _selectedUuid: ""
	property TeacherMap _selectedMap: null


	ListModel {
		id: _model
	}


	contentItem: ColumnLayout
	{
		id: col

		spacing: 5

		Item {
			id: itemNoMap
			Layout.fillWidth: true
			Layout.fillHeight: true

			visible: !handler || !handler.mapList || handler.mapList.count === 0

			implicitHeight: Math.min(control.parent.height-150, 350 * Qaterial.Style.pixelSizeRatio)

			Qaterial.IconLabelWithCaption {
				anchors.centerIn: parent
				icon.source: Qaterial.Icons.briefcaseOffOutline
				icon.color: Qaterial.Colors.orange500
				textColor: Qaterial.Colors.orange500
				text: qsTr("Nincsenek pályák")
				caption: qsTr("A szerveren még egyetlen pálya sem található")
			}
		}


		QMapPathSelector {
			id: _selector
			Layout.fillWidth: true
			Layout.preferredHeight: implicitHeight
			visible: !itemNoMap.visible
		}

		Qaterial.HorizontalLineSeparator {
			Layout.fillWidth: true
			visible: !itemNoMap.visible
		}

		ListView {
			id: view
			Layout.fillWidth: true
			Layout.fillHeight: true

			implicitHeight: 800
			implicitWidth: 200

			clip: true

			visible: !itemNoMap.visible

			model: SortFilterProxyModel {
				sourceModel: _model

				filters: [
					ValueFilter {
						roleName: "parent"
						value: _selector.currentTagId
						enabled: _selector.currentTagId >= 0
					},
					ValueFilter {
						roleName: "type"
						value: 4					// Minden pálya
						enabled: _selector.currentTagId < 0
					}

				]

				sorters: [
					RoleSorter {
						roleName: "type"
						priority: 2
						sortOrder: Qt.DescendingOrder
					},
					StringSorter {
						roleName: "name"
						priority: 1
					}
				]
			}


			delegate: Qaterial.ItemDelegate {
				id: item
				width: view.width

				required property int id
				required property string uuid
				required property string name
				required property int type
				required property TeacherMap map

				readonly property bool isTag: type > 1 && type != 4

				text: name
				icon.source: type == 3 ? Qaterial.Icons.tagMultiple : isTag ? Qaterial.Icons.tag : Qaterial.Icons.briefcaseOutline
				textColor: isTag ? Qaterial.Style.accentColor : Qaterial.Style.primaryTextColor()

				MouseArea {
					anchors.fill: parent
					acceptedButtons: Qt.LeftButton

					onClicked: {
						if (item.isTag) {
							_selector.addToStack(item.id, item.name)
						} else {
							_selectedUuid = item.uuid
							_selectedMap = item.map
							control.accept()
						}
					}
				}
			}
		}
	}



	function loadModel() {
		_model.clear()

		if (!handler || !handler.mapList || !handler.tagList)
			return

		if (handler.mapList.count === 0)
			return


		for (let i=0; i<handler.mapList.length; ++i) {
			let m = handler.mapList.get(i)

			if (disabledUuids.includes(m.uuid))
				continue

			// Minden pálya módban csak egyszer szerepeljen
			_model.append({
							  type: 4, id: 0, uuid: m.uuid, parent: -1, name: m.name, map: m
						  })

			if (m.tags.length === 0) {
				_model.append({
								  type: 1, id: 0, uuid: m.uuid, parent: 0, name: m.name, map: m
							  })
			} else {

				for (let j=0; j<m.tags.length; ++j) {
					_model.append({
									  type: 1, id: 0, uuid: m.uuid, parent: m.tags[j], name: m.name, map: m
								  })
				}

			}

		}


		let mdef = handler.mapList.get(0)			// null érték esetén a role problémás, ezrt adunk egy map-et

		_model.append({
						  type: 3, id: -2, uuid: "", parent: 0, name: qsTr("[Minden pálya]"), map: mdef
					  })

		for (let i=0; i<handler.tagList.count; ++i) {
			let t = handler.tagList.get(i)

			_model.append({
							  type: 2, id: t.tagId, uuid: "", parent: t.parentId, name: t.name, map: mdef
						  })
		}
	}


	onHandlerChanged: {
		loadModel()
		_selector.resetStack()
	}

	Component.onCompleted: {
		open()
	}


	onOpened: {
		view.forceActiveFocus()
	}

	onAccepted: {
		if (_selectedUuid == "")
			return

		mapSelected(_selectedUuid, _selectedMap)
	}

}


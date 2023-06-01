import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

Item {
	id: root

	property MapEditor editor: null

	SortFilterProxyModel {
		id: _model

		sourceModel: editor && editor.map ? editor.map.storageList : null

		sorters: RoleSorter {
			roleName: "storageid"
		}
	}


	QScrollable {
		anchors.fill: parent

		QListView {
			id: _inventoryView

			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			height: contentHeight
			boundsBehavior: Flickable.StopAtBounds

			autoSelectChange: false

			model: _model

			delegate: MapEditorStorageItem {
				storage: model.qtObject
				width: ListView.view.width

				onClicked: {
					Client.stackPushPage("MapEditorStorageEditor.qml", {
											 storage: storage
										 })
				}
			}
		}
	}

	Qaterial.Banner
	{
		anchors.top: parent.top
		width: parent.width
		drawSeparator: true
		text: qsTr("Még egyetlen adatbankot sem tartalmaz ez a pálya. Hozz létre egyet!")
		iconSource: Qaterial.Icons.database
		fillIcon: false
		outlinedIcon: true
		highlightedIcon: true

		action1: qsTr("Létrehozás")

		onAction1Clicked: _actionAdd.trigger()

		enabled: editor && editor.map && editor.map.storageList.length === 0
		visible: editor && editor.map && editor.map.storageList.length === 0
	}

	QFabButton {
		action: _actionAdd
	}

	Action {
		id: _actionAdd
		icon.source: Qaterial.Icons.databasePlus
		onTriggered: {
			let list = editor.storageListAllModel()

			if (list.length === 0)
				return

			Qaterial.DialogManager.openListView(
						{
							onAccepted: function(index)
							{
								if (index < 0)
									return

								editor.storageLoadEditor(list[index].module)

							},
							title: qsTr("Új adatbank létrehozása"),
							model: list,
							delegate: _delegate
						})
		}
	}

	Component {
		id: _delegate

		Qaterial.ItemDelegate
		{
			text: modelData.text ? modelData.text : ""
			secondaryText: modelData.secondaryText ? modelData.secondaryText : ""
			icon.source: modelData.icon ? modelData.icon : ""
			iconColor: modelData.iconColor ? modelData.iconColor : Qaterial.Colors.green400
			width: ListView.view.width
			onClicked: ListView.view.select(index)
		}
	}

}

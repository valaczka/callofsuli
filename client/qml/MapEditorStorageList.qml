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

			autoSelectChange: true

			model: _model

			delegate: MapEditorStorageItem {
				storage: model.qtObject
				width: ListView.view.width
			}

			/*footer: Qaterial.ItemDelegate {
				width: ListView.view.width
				textColor: Qaterial.Colors.blue700
				iconColor: textColor
				action: actionInventoryAdd
			}*/
		}
	}

	Qaterial.Banner
	{
		anchors.top: parent.top
		width: parent.width
		drawSeparator: true
		text: qsTr("Még egyetlen adatbázist sem tartalmaz ez a pálya. Hozz létre egyet!")
		iconSource: Qaterial.Icons.desktopClassic
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
		icon.source: Qaterial.Icons.plus
		onTriggered: {
			Qaterial.DialogManager.showTextFieldDialog({
														   textTitle: qsTr("Feladatcsoport neve"),
														   title: qsTr("Új feladatcsoport létrehozása"),
														   standardButtons: Dialog.Cancel | Dialog.Ok,
														   onAccepted: function(_text, _noerror) {
															   if (_noerror && _text.length) {
																   //var m = editor.chapterAdd(_text)
																   //loadMission(m)
															   }
														   }
													   })
		}
	}

	function loadMission(m) {
		Client.stackPushPage("MapEditorMission.qml", {
								 editor: root.editor,
								 mission: m
							 })
	}

}

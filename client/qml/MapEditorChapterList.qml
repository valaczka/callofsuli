import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

Item {
	id: root

	property MapEditor editor: null
	property int filterStorageId: -1


	onFilterStorageIdChanged: {
		// Enélkül nem frissíti rendesen

		_expressionFilter.enabled = false
		if (filterStorageId != -1)
			_expressionFilter.enabled = true
	}

	SortFilterProxyModel {
		id: _model

		sourceModel: editor && editor.map ? editor.map.chapterList : null

		sorters: StringSorter {
			roleName: "name"
		}

		filters: ExpressionFilter {
			id: _expressionFilter
			expression: usedStorageList.includes(root.filterStorageId)
			enabled: false
		}
	}

	QScrollable {
		anchors.fill: parent

		Repeater {
			model: _model

			delegate: MapEditorChapterItem {
				width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
				anchors.horizontalCenter: parent.horizontalCenter
				id: _chapter
				chapter: model.qtObject
				separatorVisible: index !== _model.count-1 || expanded
				actionAddVisible: true
				chapterDeleteAction: true
			}
		}

	}

	Qaterial.Banner
	{
		anchors.top: parent.top
		width: parent.width
		drawSeparator: true
		text: qsTr("Még egyetlen feladatot sem tartalmaz ez a pálya. Hozz létre egyet!")
		iconSource: Qaterial.Icons.folder
		fillIcon: false
		outlinedIcon: true
		highlightedIcon: true

		action1: qsTr("Létrehozás")

		onAction1Clicked: _actionAdd.trigger()

		enabled: editor && editor.map && editor.map.chapterList.length === 0
		visible: editor && editor.map && editor.map.chapterList.length === 0
	}

	QFabButton {
		action: _actionAdd
	}

	Action {
		id: _actionAdd
		icon.source: Qaterial.Icons.folderPlus
		onTriggered: {
			Qaterial.DialogManager.showTextFieldDialog({
														   textTitle: qsTr("Feladatcsoport neve"),
														   title: qsTr("Új feladatcsoport létrehozása"),
														   standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
														   onAccepted: function(_text, _noerror) {
															   if (_noerror && _text.length) {
																   editor.chapterAdd(_text)
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

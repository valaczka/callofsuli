import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: root

	stackPopFunction: function() {
		/*if (_campaignList.view.selectEnabled) {
			_campaignList.view.unselectAll()
			return false
		}*/

		if (swipeView.currentIndex > 0) {
			swipeView.decrementCurrentIndex()
			return false
		}

		return true
	}

	title: qsTr("Pályaszerkesztő")
	subtitle: _editor.displayName

	MapEditor {
		id: _editor
	}

	appBar.backButtonVisible: true
	appBar.rightComponent: MapEditorToolbarComponent {
		editor: _editor
		Shortcut {
			sequence: "Ctrl+S"
			onActivated: _editor.save()
		}

		Shortcut {
			sequence: "Ctrl+Z"
			onActivated: if (_editor.undoStack.canUndo)
							 _editor.undoStack.undo()
		}

		Shortcut {
			sequence: "Ctrl+Y"
			onActivated: if (_editor.undoStack.canRedo)
							 _editor.undoStack.redo()
		}


		Qaterial.AppBarButton
		{
			icon.source: Qaterial.Icons.dotsVertical

			/*onClicked: swipeView.currentIndex == 0 ? menuClass.open() : menuCampaign.open()

		QMenu {
			id: menuClass

			QMenuItem { action: actionGroupRename }
			QMenuItem { action: actionGroupRemove }
		}

		QMenu {
			id: menuCampaign

			QMenuItem { action: _campaignList.actionCampaignAdd }
		}*/
		}
	}


	Qaterial.SwipeView
	{
		id: swipeView
		anchors.fill: parent
		currentIndex: tabBar.currentIndex

		MapEditorMissionList {
			editor: _editor
		}


		MapEditorChapterList {
			editor: _editor
		}


		MapEditorStorageList {
			editor: _editor
		}
	}

	footer: QTabBar {
		id: tabBar
		currentIndex: swipeView.currentIndex

		Component.onCompleted: {
			model.append({ text: qsTr("Küldetések"), source: Qaterial.Icons.trophyBroken, color: "pink" })
			model.append({ text: qsTr("Feladatcsoportok"), source: Qaterial.Icons.account, color: "green" })
			model.append({ text: qsTr("Adatbázisok"), source: Qaterial.Icons.trophyBroken, color: "pink" })
			/*model.append({ text: qsTr("Dolgozatok"), source: Qaterial.Icons.trophyBroken, color: "pink" })*/
		}
	}



	MapEditorObjectiveDialog {
		editor: _editor
	}




}

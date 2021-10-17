import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QBasePage {
	id: control

	defaultTitle: ""

	//mainToolBarComponent: QToolButton { action: actionSave }

	/*toolBarMenu: QMenu {
		MenuItem {
			text: qsTr("Pályák")
			icon.source: CosStyle.iconBooks
		}
		MenuItem {
			text: qsTr("Fejezetek")
			icon.source: CosStyle.iconAdd
		}
		MenuItem {
			text: qsTr("Storages")
			icon.source: CosStyle.iconBooks
		}
	}*/

	/*activity: ServerSettings {
		id: serverSettings
	}*/




	QStackComponent {
		id: stackComponent
		anchors.fill: parent
		basePage: control

		//requiredWidth: 500

		//headerContent: QLabel {	}

		initialItem: QSimpleContainer {
			id: panel

			title: ""
			icon: CosStyle.iconSetup

			/*menuComponent: QToolButton {
					id: menuButton
					action: actionA
					display: AbstractButton.IconOnly
				}*/

			/*property var contextMenuFunc: function (m) {
					m.addAction(actionA)
				}*/
		}


	}


	function windowClose() {
		return false
	}

	function pageStackBack() {
		if (stackComponent.layoutBack())
			return true

		return false
	}
}



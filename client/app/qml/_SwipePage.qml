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



	QSwipeComponent {
		id: swComponent
		anchors.fill: parent

		//headerContent: QLabel {	}

		content: [
			QSwipeContainer {
				id: container1
				reparented: swComponent.swipeMode
				reparentedParent: placeholder1
				title: ""
				icon: CosStyle.iconSetup
				QAccordion {
					QCollapsible {
						title: ""
					}
				}
			},

			QSwipeContainer {
				id: container2
				reparented: swComponent.swipeMode
				reparentedParent: placeholder2
				title: ""
				icon: CosStyle.iconPreferences
				QAccordion {
					QCollapsible {
						title: ""
					}
				}
			}
		]

		swipeContent: [
			Item { id: placeholder1 },
			Item { id: placeholder2 }
		]

		tabBarContent: [
			QSwipeButton { swipeContainer: container1 },
			QSwipeButton { swipeContainer: container2 }
		]

	}


	function windowClose() {
		return false
	}

	function pageStackBack() {
		if (swComponent.layoutBack())
			return true

		return false
	}
}


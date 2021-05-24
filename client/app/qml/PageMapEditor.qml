import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QBasePage {
	id: control

	defaultTitle: qsTr("Pályaszerkesztő")

	//mainToolBarComponent: QToolButton { action: actionSave }

	activity: MapEditor {
		id: mapEditor
	}

	toolBarMenu: QMenu {

		MenuItem {
			text: qsTr("Pályák")
			icon.source: CosStyle.iconBooks
			//onClicked: editorLoader.sourceComponent = componentOpenSave
		}
		MenuItem {
			text: qsTr("Fejezetek")
			icon.source: CosStyle.iconAdd
			onClicked: editorLoader.sourceComponent = componentMissions
		}
		MenuItem {
			text: qsTr("Storages")
			icon.source: CosStyle.iconBooks
			onClicked: editorLoader.sourceComponent = undefined
		}
	}



	Loader {
		id: editorLoader
		anchors.fill: parent
	}


	Column {
		visible: editorLoader.status != Loader.Ready

		anchors.centerIn: parent
		spacing: 5
		QToolButtonBig {
			anchors.horizontalCenter: parent.horizontalCenter
			text: qsTr("Új")
			icon.source: CosStyle.iconAdd
		}
		QToolButtonBig {
			anchors.horizontalCenter: parent.horizontalCenter
			text: qsTr("Megnyitás")
			icon.source: CosStyle.iconAdjust
		}
	}





	Component {
		id: componentMissions
		QSwipeComponent {
			id: swComponent
			anchors.fill: parent

			//headerContent: QLabel {	}

			content: [
				QSwipeContainer {
					id: container1
					reparented: swComponent.swipeMode
					reparentedParent: placeholder1
					title: "a"
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
					title: "b"
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
	}


	function windowClose() {
		return false
	}

	function pageStackBack() {
		if (editorLoader.status == Loader.Ready && editorLoader.item.layoutBack())
			return true

		return false
	}
}


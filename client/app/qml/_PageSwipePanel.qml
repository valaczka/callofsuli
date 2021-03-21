import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QPage {
	id: control

	mainToolBar.title: ""

	stackTopOffset: 0

	panelComponents: [
		Component {
			QPageSwipePanel {
				id: panel

				title: ""

				headerContent: QLabel {

				}

				QSwipeContainer {
					id: container1
					reparented: stackMode
					reparentedParent: placeholder1
					title: ""
					icon: CosStyle.iconSetup
					QCollapsible {
						title: ""
					}
				}

				QSwipeContainer {
					id: container2
					reparented: stackMode
					reparentedParent: placeholder2
					title: ""
					icon: CosStyle.iconPreferences
					QCollapsible {
						title: ""
					}

				}

				swipeContent: [
					Item { id: placeholder1 },
					Item { id: placeholder2 }
				]

				tabBarContent: [
					QSwipeButton { swipeContainer: container1 },
					QSwipeButton { swipeContainer: container2 }
				]

				onPopulated: container1.forceActiveFocus()
			}
		}
	]



	function windowClose() {
		return false
	}

	function pageStackBack() {
		return false
	}
}


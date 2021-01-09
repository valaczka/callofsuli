import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: page

	defaultTitle: qsTr("")
	defaultSubTitle: ""


	mainToolBarComponent: QToolButton {
			action: actionSave
		}


	activity: AbstractActivity {

	}


	property list<Component> listComponents: [
		Component { QPagePanel {
				panelVisible: true
				layoutFillWidth: true
				Connections {
					target: page
					function onPageActivated() {
						list.forceActiveFocus()
					}
				}
			} }
	]

	panelComponents: listComponents


	mainMenuFunc: function (m) {
		m.addAction(actionSave)
	}



	onPageActivated: {

	}



	function windowClose() {
		return false
	}


	function pageStackBack() {
		return false
	}

}

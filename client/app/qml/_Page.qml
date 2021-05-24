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

	//mainToolBar.titleItem.acceptedButtons: Qt.LeftButton
	//mainToolBar.titleItem.mouseArea.onClicked: console.debug("CLICK")


	activity: AbstractActivity {

	}


	panelComponents: [
		Component { QPagePanel {  } }
	]


	mainMenuFunc: function (m) {
		m.addAction(actionSave)
	}

	onStackModeChanged: {

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

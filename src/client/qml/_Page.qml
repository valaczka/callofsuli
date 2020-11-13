import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: page

	//requiredPanelWidth: 900

	title: ""
	subtitle: ""

	mainToolBarComponent: QToolBusyIndicator { running: true }


	//mainMenuFunc: function(m) {}
	//contextMenuFunc: function(m) {}

	panelComponents: [
		Component { QPagePanel {
				panelVisible: true
				layoutFillWidth: true
			} }
	]

	onPageActivated: {
	}


	function windowClose() {
		return true
	}

	function pageStackBack() {
		return false
	}

}

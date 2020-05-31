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

	mainToolBarComponent: QToolBusyIndicator { running: true }

	pageContextMenu: QMenu {
		MenuItem {
		}
	}


	onlyPanel: QPagePanel {
		id: panel

		title: page.title
		maximumWidth: 600

		//onPanelActivated:
		//onPopulated:

		Connections {
			target: page
			//onPageActivated:
		}

	}

	onPageActivated: {
		panels = [
					{ url: "", params: { }, fillWidth: false},
					{ url: "", params: { }, fillWidth: true}
				]
	}


	function windowClose() {
		return true
	}

	function pageStackBack() {
		return false
	}

}

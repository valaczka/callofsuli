import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: page

	header: QToolBar {
		id: toolbar

		title: "PAGE"

		backButton.visible: true
		backButton.onClicked: mainStack.back()

		/*Row {
			QToolBusyIndicator { running: .isBusy }
			QMenuButton {
				MenuItem {
					text:
				}
			}
		}*/
	}

	Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/img/villa.png"
	}



	/* CONTENT */


	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.resetTitle()
			/* LOAD */
	}

	StackView.onDeactivated: {
			/* UNLOAD */
	}

	function windowClose() {
		return true
	}

	function stackBack() {
		if (mainStack.depth > page.StackView.index+1) {
			if (!mainStack.get(page.StackView.index+1).stackBack()) {
				if (mainStack.depth > page.StackView.index+1) {
					mainStack.pop(page)
				}
			}
			return true
		}

		/* BACK */

		return false
	}
}

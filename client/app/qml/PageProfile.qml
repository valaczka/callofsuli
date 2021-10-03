import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QBasePage {
	id: control

	defaultTitle: qsTr("Profil")

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

	mainToolBarComponent: Row {
		QToolButton {
			action: actionSave
			display: AbstractButton.IconOnly
		}

		UserButton {
			userDetails: userData
			userNameVisible: control.width>800
		}
	}

	UserDetails {
		id: userData
	}

	activity: Profile {
		id: profile
	}



	QSwipeComponent {
		id: swComponent
		anchors.fill: parent

		//headerContent: QLabel {	}

		content: [
			ProfileDetails {
				id: container1
				reparented: swComponent.swipeMode
				reparentedParent: placeholder1

				flickable.onAccepted: actionSave.trigger()
			}
		]

		swipeContent: [
			Item { id: placeholder1 }
			//Item { id: placeholder2 }
		]

		tabBarContent: [
			QSwipeButton { swipeContainer: container1 }
			//QSwipeButton { swipeContainer: container2 }
		]

	}


	onPageActivated: profile.send("userGet", {})


	Action {
		id: actionSave
		icon.source: CosStyle.iconSave
		text: qsTr("Mentés")
		enabled: container1.flickable.modified && container1.flickable.acceptable
		shortcut: "Ctrl+S"

		onTriggered: container1.save()
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


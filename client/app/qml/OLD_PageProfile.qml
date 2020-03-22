import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
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
			action: container1.modificationEnabled ? actionSave : actionEdit
			display: AbstractButton.IconOnly
		}

		/*UserButton {
			userDetails: userData
			userNameVisible: control.width>800
		}*/
	}

	/*UserDetails {
		id: userData
	}*/

	activity: Profile {
		id: profile

		onUserPasswordChange: {
			if (jsonData.error && jsonData.error.length) {
				cosClient.sendMessageWarning(qsTr("Jelszó változtatás"), qsTr("A jelszó változtatás sikertelen"), jsonData.error)
			} else {
				cosClient.sendMessageInfo(qsTr("Jelszó változtatás"), qsTr("A jelszót sikeresen megváltozott"))
			}
		}
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

				Layout.fillWidth: false
				Layout.preferredWidth: 1200
				Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

				onAccepted: actionSave.trigger()
			}
		]

		swipeContent: [
			Item { id: placeholder1
				property string title: container1.title
			}
			//Item { id: placeholder2 }
		]

		tabBarContent: [
			QSwipeButton { swipeContainer: container1 }
			//QSwipeButton { swipeContainer: container2 }
		]

	}


	onPageActivated: profile.send("userGet", {
									  withTrophy: true,
									  withRanklog: true
								  })


	Action {
		id: actionSave
		icon.source: CosStyle.iconSave
		text: qsTr("Mentés")
		enabled: container1.modificationEnabled && container1.modified && container1.acceptable
		shortcut: "Ctrl+S"

		onTriggered: container1.save()
	}

	Action {
		id: actionEdit
		icon.source: CosStyle.iconEdit
		text: qsTr("Szerkesztés")
		shortcut: "F4"

		onTriggered: container1.modificationEnabled = true
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


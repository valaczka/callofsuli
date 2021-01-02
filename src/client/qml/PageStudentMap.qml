import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: page

	defaultTitle: qsTr("Pályák")

	mainToolBarComponent: UserButton {
		userDetails: userData
		userNameVisible: page.width>800
	}

	UserDetails {
		id: userData
	}

	activity: StudentMaps {
		id: studentMaps

		onMapDownloadRequest: {
			var d = JS.dialogCreateQml("YesNo", {
										   title: qsTr("Letöltés"),
										   text: qsTr("A szerver %1 adatot akar küldeni. Elindítod a letöltést?").arg(formattedDataSize)
									   })
			d.accepted.connect(function() {
				var dd = JS.dialogCreateQml("Progress", { title: qsTr("Letöltés"), downloader: studentMaps.downloader })
				dd.closePolicy = Popup.NoAutoClose
				dd.open()
			})

			d.open()
		}


		onGameMapLoaded: {
			JS.createPage("MapMissions", {
							  studentMaps: studentMaps,
							  defaultTitle: mapName
						  })
		}

		onGameMapUnloaded: {
			if (page.StackView.view)
				mainStack.pop(page)
		}

		onGamePlayReady: {
			var o = JS.createPage("Game", {
									  gameMatch: gameMatch,
									  deleteGameMatch: true
								  })
		}

	}


	panelComponents: [
		Component { StudentMapList {
				panelVisible: true
				Connections {
					target: page
					function onPageActivated() {
						list.forceActiveFocus()
					}
				}
			} }
	]




	/*mainMenuFunc: function (m) {
			m.addAction(actionSave)
		}*/



	onPageActivated: {
		studentMaps.send("mapListGet")
	}




	function windowClose() {
		return false
	}


	function pageStackBack() {
		return false
	}

}

import QtQuick 2.15
import QtQuick.Controls 2.15
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
							  defaultSubTitle: mapName
						  })
		}

		onGameMapUnloaded: {
			if (page.StackView.view)
				mainStack.pop(page)
		}

		onGamePlayReady: {
			var o = JS.createPage("Game", {
									  gameMatch: gameMatch,
									  studentMaps: studentMaps,
									  deleteGameMatch: true
								  })

			isGameRunning = o ? true : false
		}

		onGameFinishDialogReady: {
			JS.dialogMessage("success", qsTr("Game over"), qsTr("MISSION COMPLETED\nMegszerezve %1 XP\nTeljesítve: %2x\nStreak: %3/%4").arg(data.xp).arg(data.solved).arg(data.currentStreak).arg(data.maxStreak))
		}


		function selectGroup(id) {
			selectedGroupId = id
			if (id !== -1 && stackMode) {
				addStackPanel(panelMapList)
			}
		}
	}


	panelComponents: [
		Component { StudentGroupList {  } },
		Component { StudentMapList { } }
	]


	Component {
		id: panelMapList
		StudentMapList { }
	}



	/*mainMenuFunc: function (m) {
			m.addAction(actionSave)
		}*/



	onPageActivated: {
		studentMaps.send("groupListGet")
	}




	function windowClose() {
		return false
	}


	function pageStackBack() {
		return false
	}

}

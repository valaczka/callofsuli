import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS




QBasePage {
	id: page

	defaultTitle: ""
	defaultSubTitle: ""

	property alias groupId: studentMaps.selectedGroupId

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
							  defaultSubTitle: mapName,
							  mapUuid: mapUuid
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
			var d = JS.dialogCreateQml("MissionCompleted", { gameData: data })
			d.accepted.connect(function(m) {
				if (studentMaps.gamePage) {
					cosClient.sendMessageError(qsTr("Belső hiba"), qsTr("Már folyamatban van egy játék!"))
				} else {
					studentMaps.playGame({
											 uuid: m.missionid,
											 level: m.level,
											 deathmatch: m.deathmatch
										 })
				}
			})
			d.open()
		}

	}


	QSwipeComponent {
		id: swComponent
		anchors.fill: parent

		content: [
			StudentMapList {
				id: container1
				reparented: swComponent.swipeMode
				reparentedParent: placeholder1
			},
			StudentMemberList {
				id: container2
				reparented: swComponent.swipeMode
				reparentedParent: placeholder2
			}
		]

		swipeContent: [
			Item { id: placeholder1 },
			Item { id: placeholder2 }
		]

		tabBarContent: [
			QSwipeButton { swipeContainer: container1 },
			QSwipeButton { swipeContainer: container2 }
		]

	}


	onPageActivated: {
		if (studentMaps.selectedGroupId > -1) {
			studentMaps.send("mapListGet", {groupid: studentMaps.selectedGroupId})
			studentMaps.send("userListGet", {groupid: studentMaps.selectedGroupId})
		} else
			studentMaps.modelMapList.clear()

		container1.list.forceActiveFocus()
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


import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QBasePage {
	id: page

	defaultTitle: qsTr("Pályák kezelése")
	defaultSubTitle: ""

	mainToolBarComponent: UserButton {
		userDetails: userData
		userNameVisible: page.width>800
	}

	mainMenuFunc: function (m) {
		m.addAction(actionUpload)
		m.addSeparator()
		m.addAction(actionMapEditor)
	}

	UserDetails {
		id: userData
	}


	activity: TeacherMaps {
		id: teacherMaps


		onSelectedMapIdChanged: {
			if (selectedMapId != "")
				swComponent.swipeToPage(1)
		}

		onMapDownloadRequest: {
			if (teacherMaps.downloader.fullSize > cosClient.getSetting("autoDownloadBelow", 500000)) {
				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Letöltés"),
											   text: qsTr("A szerver %1 adatot akar küldeni. Elindítod a letöltést?").arg(formattedDataSize)
										   })
				d.accepted.connect(function() {
					var dd = JS.dialogCreateQml("Progress", { title: qsTr("Letöltés"), downloader: teacherMaps.downloader })
					dd.closePolicy = Popup.NoAutoClose
					dd.open()
				})

				d.open()
			} else {
				var dd = JS.dialogCreateQml("Progress", { title: qsTr("Letöltés"), downloader: teacherMaps.downloader })
				dd.closePolicy = Popup.NoAutoClose
				dd.open()
			}
		}

		onMapAdd: {
			if (jsonData.error !== undefined) {
				cosClient.sendMessageWarning(qsTr("Hiba"), qsTr("Sikertelen feltöltés:\n%1").arg(jsonData.error))
			} else {
				send("mapListGet");
			}
		}

		onMapModify: {
			if (jsonData.error !== undefined) {
				cosClient.sendMessageWarning(qsTr("Hiba"), qsTr("Sikertelen módosítás:\n%1").arg(jsonData.error))
			} else {
				send("mapListGet");
			}
		}

		onMapRemove: {
			if (jsonData.error !== undefined) {
				cosClient.sendMessageWarning(qsTr("Hiba"), qsTr("Sikertelen törlés:\n%1").arg(jsonData.error))
			} else {
				send("mapListGet");
			}
		}
	}


	QSwipeComponent {
		id: swComponent
		anchors.fill: parent

		basePage: page

		content: [
			TeacherMapList {
				id: container1
				reparented: swComponent.swipeMode
				reparentedParent: placeholder1
				buttonUpload.action: actionUpload
			},
			TeacherMapInfo {
				id: container2
				reparented: swComponent.swipeMode
				reparentedParent: placeholder2
			}
		]

		swipeContent: [
			Item {
				id: placeholder1
			},
			Item {
				id: placeholder2
			}
		]

		tabBarContent: [
			QSwipeButton { swipeContainer: container1 },
			QSwipeButton { swipeContainer: container2 }
		]

	}


	Action {
		id: actionUpload
		text: qsTr("Feltöltés")
		icon.source: CosStyle.iconUpload
		onTriggered: {
			var d = JS.dialogCreateQml("File", {
										   isSave: false,
										   folder: cosClient.getSetting("mapFolder", ""),
										   title: qsTr("Feltöltés")
									   })
			d.accepted.connect(function(data){
				teacherMaps.mapUpload(data)
				cosClient.setSetting("mapFolder", d.item.modelFolder)
			})

			d.open()
		}
	}


	Action {
		id: actionMapEditor
		text: qsTr("Pályaszerkesztő")
		icon.source: CosStyle.iconEdit
		onTriggered: {
			JS.createPage("MapEditor", { })
		}
	}


	onPageActivated: {
		teacherMaps.send("mapListGet", {})

		container1.list.forceActiveFocus()
	}



	function pageStackBack() {
		if (swComponent.layoutBack()) {
			return true
		} else if (teacherMaps.modelMapList.selectedCount > 0) {
			teacherMaps.modelMapList.unselectAll()
			return true
		}

		return false
	}

}


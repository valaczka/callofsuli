import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabPage {
	id: control

	property alias groupId: studentMaps.selectedGroupId

	property Profile profile: null

	buttonColor: CosStyle.colorPrimary
	buttonBackgroundColor: Qt.darker("#006400")

	buttonModel: ListModel {
		id: modelGuest

		ListElement {
			title: qsTr("Pályák")
			icon: "image://font/School/\uf19d"
			iconColor: "lime"
			func: function() { replaceContent(cmpStudentMapList) }
			checked: true
		}
		ListElement {
			title: qsTr("Résztvevők")
			icon: "image://font/School/\uf154"
			iconColor: "orchid"
			func: function() { replaceContent(cmpStudentMemberList) }
		}
		ListElement {
			title: qsTr("Eredményeim")
			icon: "image://font/AcademicI/\uf15d"
			iconColor: "tomato"
			func: function() { replaceContent(cmpStudentGroupScore) }
		}
	}


	activity: StudentMaps {
		id: studentMaps

		onMapDownloadRequest: {
			if (studentMaps.downloader.fullSize > cosClient.getSetting("autoDownloadBelow", 500000)) {
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
			} else {
				var dd = JS.dialogCreateQml("Progress", { title: qsTr("Letöltés"), downloader: studentMaps.downloader })
				dd.closePolicy = Popup.NoAutoClose
				dd.open()
			}
		}


		onGameMapLoaded: {
			JS.createPage("Map", {
							  studentMaps: studentMaps,
							  title: map.name,
							  mapUuid: map.uuid,
							  readOnly: !map.active
						  })
		}

		onGameMapUnloaded: {
			if (control.StackView.view)
				mainStack.pop(control)
		}

		Component.onCompleted: init(false)

	}

	Component {
		id: cmpStudentMapList
		StudentMapList {  }
	}

	Component {
		id: cmpStudentMemberList
		StudentMemberList { }
	}

	Component {
		id: cmpStudentGroupScore
		StudentGroupScore { }
	}

}


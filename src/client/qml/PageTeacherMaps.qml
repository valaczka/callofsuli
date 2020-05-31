import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: pageTeacherMap

	title: qsTr("Pályák")

	mainToolBarComponent: QToolBusyIndicator { running: teacher.isBusy }

	signal mapSelected(var id, var name)

	Teacher {
		id: teacher
		client: cosClient

		onMapCreated: listReload()

		onMapUpdated: {
			if (mapData.error) {
				client.sendMessageError("ADATBÁZIS", "HIBA", mapData.error)
			}
			listReload()
		}

		onMapRemoved: {
			if (mapData.error) {
				client.sendMessageError("ADATBÁZIS", "HIBA", mapData.error)
			}
			listReload()
		}

		onMapDataReceived: {
			var o = JS.createPage("MapEditor", {
									  mapId: jsonData["id"],
									  mapName: jsonData["name"],
									  mapBinaryFormat: true
								  })
			o.pagePopulated.connect(function() {
				/*o.map.loadFromFile("AAA.cosm")
					o.map.mapOriginalFile = "AAA.cosm"
					o.mapName = "AAA.cosm"*/

				o.map.loadFromJson(mapData)

				o.map.mapSaved.connect(pageTeacherMap.onMapSaved)

				teacher.isBusyChanged.connect(o.setBusy)

				o.Component.onDestruction.connect(function() {
					teacher.isBusyChanged.disconnect(o.setBusy)
				})
			})
		}
	}



	onMapSelected: if (id !== -1)
					   swipeToPage(1)

	onPageActivated: listReload()

	panels: [
		{ url: "TeacherMapList.qml", params: { teacher: teacher }, fillWidth: false},
		{ url: "TeacherMapMenu.qml", params: { teacher: teacher }, fillWidth: true}
		//{ url: "AdminUserData.qml", params: { adminUsers: adminUsers }, fillWidth: true}
	]

	function windowClose() {
		return true
	}

	function pageStackBack() {
		return false
	}


	function listReload() {
		if (teacher)
			teacher.send({"class": "teacherMaps", "func": "getAllMap"})
		mapSelected(-1, "")
	}

	function onMapSaved(data, uuid, mapid) {
		teacher.send({"class": "teacherMaps", "func": "updateMapData", "id": mapid }, data)
	}
}

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: page

	TeacherMaps {
		id: teacherMaps
		client: cosClient

		onMapListLoaded: setModel(list)
		onMapCreated: listReload()
		onMapUpdated: {
			if (data.error) {
				client.sendMessageError("ADATBÁZIS", "HIBA", data.error)
			}
		}

		onMapReceived: {
			var o = JS.createPage("MapEditor",
								  {
									  mapId: jsonData["id"],
									  mapName: jsonData["name"],
									  mapBinaryFormat: true,
								  },
								  page)
			o.map.loadFromJson(mapData)

			o.map.mapSaved.connect(page.onMapSaved)

			teacherMaps.isBusyChanged.connect(o.setBusy)

			o.Component.onDestruction.connect(function() {
				teacherMaps.isBusyChanged.disconnect(o.setBusy)
			})
		}
	}

	header: QToolBar {
		id: toolbar

		backButton.visible: true
		backButton.onClicked: mainStack.back()

		Row {
			QToolBusyIndicator { running: teacherMaps.isBusy }
			QMenuButton {
				MenuItem {
					text: qsTr("Új pálya")
					onClicked:  {
						var d = JS.dialogCreateQml("TextField", {title: qsTr("Új pálya neve")})
						d.accepted.connect(function(data) {
							teacherMaps.send({"class": "teacherMaps", "func": "createMap", "name": data })
						})
						d.open()
					}
				}
			}
		}
	}


	Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/img/villa.png"
	}



	QPagePanel {
		id: p

		maximumWidth: 800

		anchors.fill: parent

		blurSource: bgImage

		QListItemDelegate {
			id: listMaps
			anchors.fill: parent

			isObjectModel: true

			modelTitleRole: "name"
			modelSubtitleRole: "labelSubtitle"

			onClicked: teacherMaps.send({"class": "teacherMaps", "func": "getMap", "id": model.get(index).id })

			onLongPressed: {
				listRigthMenu.modelIndex = index
				listRigthMenu.popup()
			}

			onRightClicked: {
				listRigthMenu.modelIndex = index
				listRigthMenu.popup()
			}

			Keys.onPressed: {
				if (event.key === Qt.Key_Insert) {
					editServer(-1)
				} else if (event.key === Qt.Key_F4 && listMenu.currentIndex !== -1) {
					editServer(listMenu.model.get(listMenu.currentIndex).id)
				} else if (event.key === Qt.Key_Delete && listMenu.currentIndex !== -1) {
					/*var d = JS.dialogCreate(dlgDelete)
					d.open()*/
				}
			}
		}

		QMenu {
			id: listRigthMenu

			property int modelIndex: -1


			MenuItem {
				text: qsTr("Szerkesztés")
				//onClicked: editServer(listMenu.model.get(listRigthMenu.modelIndex).id)
			}

			MenuItem {
				text: qsTr("Törlés")


			}

			MenuSeparator {}

			MenuItem {
				text: qsTr("Új pálya")
			}
		}

	}






	function onMapSaved(data, uuid, mapid) {
		teacherMaps.send({"class": "teacherMaps", "func": "updateMap", "id": mapid }, data)
	}


	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.title = qsTr("Pályák")
		listReload()
	}

	StackView.onDeactivated: {
		/* UNLOAD */
	}


	function listReload() {
		teacherMaps.send({"class": "teacherMaps", "func": "getAllMap"})
	}


	function setModel(list) {
		listMaps.model.clear()
		for (var i=0; i<list.length; i++) {
			var o = list[i]
			o.labelSubtitle = o.timeCreated+" "+o.timeModified+" v"+o.version
			o.toolTip = "Objectives "+o.objectives
			listMaps.model.append(o)
		}

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

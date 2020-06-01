import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: pageMaps

	property bool _isFirst: true

	requiredPanelWidth: 900

	title: qsTr("Pályák")
	subtitle: cosClient.serverName


	signal loadMap(var uuid, var md5, var maptitle)

	Student {
		id: student
		client: cosClient

		onMapRepositoryOpenError: {
			var d = JS.dialogMessageError(qsTr("Adatbázis megnyitás"), qsTr("Nem sikerült megnyitni az adatbázist!"), databaseFile)
			d.onClosedAndDestroyed.connect(function() {
				mainStack.back()
			})
		}

		onMapRepositoryOpened: listReload()
	}

	mainToolBarComponent: UserButton {
		userDetails: userData
	}

	onlyPanel: QPagePanel {
		id: panel

		title: pageMaps.title
		maximumWidth: 600

		ListModel {
			id: baseMapModel
		}

		SortFilterProxyModel {
			id: mapProxyModel
			sourceModel: baseMapModel
			sorters: [
				StringSorter { roleName: "groupname" },
				StringSorter { roleName: "mapname" }
			]
		}


		QListItemDelegate {
			id: listMaps
			anchors.fill: parent

			model: mapProxyModel
			isProxyModel: true
			modelTitleRole: "mapname"

			refreshEnabled: true
			onRefreshRequest: listReload()

			section.property: "groupname"
			section.criteria: ViewSection.FullString
			section.labelPositioning: ViewSection.InlineLabels | ViewSection.CurrentLabelAtStart
			section.delegate: Rectangle {
				width: listMaps.width
				height: childrenRect.height
				color: CosStyle.colorAccentDarker

				QLabel {
					text: section
					color: "black"
					font.capitalization: Font.AllUppercase
					font.weight: Font.DemiBold
					padding: 2
				}
			}

			onClicked: {
				var m = listMaps.model.get(index)
				pageMaps.loadMap(m.uuid, m.md5, m.mapname)
			}
		}

		onPanelActivated: listReload()

		Connections {
			target: pageMaps
			onPageActivated: {
				listReload()
				listMaps.forceActiveFocus()
			}
		}

		Connections {
			target: student

			onMapListLoaded: {
				JS.setModel(baseMapModel, list)
			}
		}

	}

	UserDetails {
		id: userData
	}


	onLoadMap: {
		var o = JS.createPage("StudentMap", {
								  student: student,
								  title: maptitle,
								  mapUuid: uuid,
								  mapMd5: md5
							  })
	}

	onPageActivated: {
		if (_isFirst) {
			student.mapRepositoryOpen()
			_isFirst = false
		}
	}


	function listReload() {
		if (student && student.repositoryReady)
			student.send({"class": "student", "func": "getMaps"})
	}


	function windowClose() {
		return true
	}

	function pageStackBack() {
		return false
	}
}



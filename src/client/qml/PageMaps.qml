import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: pageMaps

	requiredPanelWidth: 900

	title: qsTr("Pályák")
	subtitle: cosClient.serverName

	Student {
		id: student
		client: cosClient
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



	function listReload() {
		if (student)
			student.send({"class": "student", "func": "getMaps"})
	}

	function windowClose() {
		return true
	}

	function pageStackBack() {
		return false
	}
}



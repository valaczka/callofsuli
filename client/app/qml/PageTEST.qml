import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS
import SortFilterProxyModel 0.2


QTabPage {
	id: control

	title: qsTr("Test page title %1").arg(serverSettings.testModel.selectedCount)
	backFunction: null

	property int num: 13

	activity: ServerSettings {
		id: serverSettings
	}

	Component {
		id: cmp1
		QTabContainer {
			title:  "Test"


			menu: QMenu {
				MenuItem {
					text: "2"
					onClicked: {
						serverSettings.updateJsonArray(num, num++)
						console.debug(serverSettings.testModel.toJsonArray())
					}
				}
			}

			QListView {
				id: view
				width: parent.width
				model: SortFilterProxyModel {
					sourceModel: serverSettings.testModel
					sorters: [
						StringSorter { roleName: "param3"; priority: 0 }
					]
				}

				delegate: Rectangle {
					width: view.width
					height: 50
					color: "black"
					QLabel {
						text: "%1 - %2 | %3".arg(param1).arg(param3).arg(selected)
					}

					QButton {
						anchors.right: parent.right
						anchors.verticalCenter: parent.verticalCenter
						text: 'X'
						onClicked: {
							var d = serverSettings.testModel.object(view.model.mapToSource(index))
							//var d = serverSettings.testModel.object(index)
							//console.debug("D", d, d.param1, d.param3, d.selected)
							//d.destroy()
							//d.param3 = "XXX"
							d.param3 = "x"+d.param3
							selected = !selected
						}
					}
				}
			}

		}
	}

	Component.onCompleted: pushContent(cmp1)
}

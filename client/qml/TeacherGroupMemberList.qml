import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "./JScript.js" as JS

Item
{
	id: control

	property TeacherGroup group: null
	property alias view: view

	QScrollable {
		anchors.fill: parent

		QListView {
			id: view

			currentIndex: -1
			autoSelectChange: true

			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter


			refreshProgressVisible: Client.webSocket.pending
			refreshEnabled: true
			onRefreshRequest: group.reload()

			model: SortFilterProxyModel {
				sourceModel: group ? group.memberList : null

				sorters: [
					StringSorter {
						roleName: "fullName"
						sortOrder: Qt.AscendingOrder
					}
				]
			}

			delegate: QItemDelegate {
				property User user: model.qtObject
				selectableObject: user

				highlighted: ListView.isCurrentItem
				iconSource: Qaterial.Icons.account

				text: user ? user.fullName: ""

				/*onClicked: if (!view.selectEnabled)
						   Client.stackPushPage("AdminUserEdit.qml", {
													user: user
												})*/
			}
		}
	}

	Qaterial.Banner
	{
		anchors.top: parent.top
		width: parent.width
		drawSeparator: true
		text: qsTr("Még egyetlen résztvevő sincsen felvéve. Adj hozzá a csoporthoz egy osztályt vagy felhasználót.")
		iconSource: Qaterial.Icons.account
		fillIcon: false
		outlinedIcon: true
		highlightedIcon: true

		action1: qsTr("Hozzáadás")

		onAction1Clicked: actionUserEdit.trigger()

		enabled: group
		visible: group && !group.memberList.length
	}

	QFabButton {
		visible: view.visible
		action: actionUserEdit
	}

	Action {
		id: actionUserEdit
		icon.source: Qaterial.Icons.pen
		text: qsTr("Törlés")
		onTriggered: {
			Client.stackPushPage("PageTeacherGroupEdit.qml", {
									 group: control.group
								 })

		}
	}



}

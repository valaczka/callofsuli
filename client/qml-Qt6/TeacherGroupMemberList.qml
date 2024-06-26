import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel
import "./JScript.js" as JS

Item
{
	id: control

	property TeacherGroup group: null
	property alias view: view

	QScrollable {
		anchors.fill: parent

		refreshEnabled: true
		onRefreshRequest: group.reload()

		topPadding: 0
		leftPadding: 0
		bottomPadding: 0
		rightPadding: 0

		QListView {
			id: view

			currentIndex: -1
			autoSelectChange: true

			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsBehavior: Flickable.StopAtBounds

			refreshProgressVisible: Client.httpConnection.pending

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
		iconSource: Qaterial.Icons.accountOutline
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
		text: qsTr("Résztvevők")
		onTriggered: {
			Client.stackPushPage("PageTeacherGroupEdit.qml", {
									 group: control.group
								 })

		}
	}



}

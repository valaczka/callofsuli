import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: control

	title: qsTr("Ranglista")
	subtitle: Client.server ? Client.server.serverName : ""

	appBar.backButtonVisible: true

	QScrollable {
		anchors.fill: parent
		horizontalPadding: 0
		topPadding: 0
		bottomPadding: 0

		refreshEnabled: false


		QListView {
			id: view

			currentIndex: -1
			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsBehavior: Flickable.StopAtBounds

			model: Client.server ? Client.server.rankList : null

			delegate: Qaterial.LoaderItemDelegate {
				highlighted: ListView.isCurrentItem
				text: modelData.name
				secondaryText: modelData.sublevel > 0 ? qsTr("level %1").arg(modelData.sublevel) : ""

				width: ListView.view.width

				leftSourceComponent: UserImage {
					user: User {
						rank: modelData
					}
				}

				rightSourceComponent: Qaterial.LabelHeadline5 {
					text: modelData.xp >= 0 ? qsTr("%1 XP").arg(Number(modelData.xp).toLocaleString()) : ""
					color: Qaterial.Style.iconColor()
				}
			}
		}
	}
}

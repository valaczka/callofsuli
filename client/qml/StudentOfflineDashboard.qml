import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel
import "JScript.js" as JS


QItemGradient {
	id: root

	property OfflineClientEngine engine: null
	property StudentMapHandler studentMapHandler: engine ? engine.studentMapHandler : null
	property User user: engine ? engine.user : null
	property CampaignList campaignList: engine ? engine.campaignList : null
	property bool freeplay: engine ? engine.freeplay : false

	property date _referenceDate: new Date()


	appBar.rightComponent: Row {
		Qaterial.AppBarButton {
			icon.source: Qaterial.Icons.cogOutline
			onClicked: Client.stackPushPage("PageStudentSettings.qml")
		}
	}

	QScrollable {
		anchors.fill: parent
		spacing: 15
		contentCentered: true
		topPadding: Math.max(verticalPadding, Client.safeMarginTop, root.paddingTop)

		refreshEnabled: false

		Qaterial.LabelHeadline3 {
			anchors.horizontalCenter: parent.horizontalCenter
			width: Math.min(parent.width-100, Qaterial.Style.maxContainerSize)
			//topPadding: root.paddingTop
			horizontalAlignment: Qt.AlignHCenter
			text: user ? user.fullNickName : ""
			wrapMode: Text.Wrap
			maximumLineCount: 2
			elide: Text.ElideRight
		}

		Qaterial.LabelHeadline6 {
			anchors.horizontalCenter: parent.horizontalCenter
			text: qsTr("Offline mód")
		}



		QDashboardGrid {
			id: _grid
			anchors.horizontalCenter: parent.horizontalCenter

			Repeater {
				model: _campaignList

				delegate: QDashboardButton {
					id: _btn
					property Campaign campaign: model && model.qtObject ? model.qtObject : null
					text: campaign ? campaign.readableName : ""

					icon.source: Qaterial.Icons.trophyBroken

					QTagList {
						anchors.right: parent.right
						anchors.top: parent.top
						anchors.rightMargin: Qaterial.Style.card.horizontalPadding
						anchors.topMargin: Qaterial.Style.card.horizontalPadding

						readonly property int msecLeft: _btn.campaign && _btn.campaign.endTime.getTime() ?
															_btn.campaign.endTime - _referenceDate.getTime():
															0

						visible: msecLeft > 0 && msecLeft < 8 * 24 * 60 * 60 * 1000

						readonly property string stateString: {
							let d = Math.floor(msecLeft / (24*60*60*1000))

							if (d > 5)
								return qsTr(">5 nap")
							else if (d > 0)
								return qsTr("%1 nap").arg(d)
							else if (msecLeft > 60*60*1000) {
								let h = Math.floor(msecLeft / (60*60*1000))
								return qsTr("%1 óra").arg(h)
							} else {
								return qsTr("<1 óra")
							}
						}

						readonly property color stateColor: {
							if (msecLeft > 4 * 24 * 60 * 60 * 1000)
								return Qaterial.Colors.green600
							else if (msecLeft > 2 * 24 * 60 * 60 * 1000)
								return Qaterial.Colors.orange800
							else
								return Qaterial.Colors.red500
						}

						z: 99

						model: [
							{
								"text": stateString,
								"color": stateColor,
								"textColor": Qaterial.Colors.white
							}

						]
					}

					onClicked: {
						Client.stackPushPage("PageStudentCampaign.qml", {
												 user: root.user,
												 campaign: _btn.campaign,
												 studentMapHandler: root.studentMapHandler,
												 title: qsTr("Offline mód")
											 })
					}
				}

			}


			QDashboardButton {
				text: qsTr("Szabad játék")
				visible: root.freeplay
				icon.source: Qaterial.Icons.controller
				highlighted: false
				outlined: true
				flat: true

				textColor: Qaterial.Colors.green500

				onClicked: Client.stackPushPage("PageStudentFreePlay.qml", {
													studentMapHandler: studentMapHandler,
													title: qsTr("Offline mód")
												})
			}

		}

	}

	SortFilterProxyModel {
		id: _campaignList
		sourceModel: campaignList

		/*filters: ValueFilter {
			roleName: "finished"
			value: false
		}*/

		sorters: StringSorter {
			roleName: "readableName"
			sortOrder: Qt.AscendingOrder
		}
	}




	StackView.onDeactivating: {
		//Client.contextHelper.unsetContext(ContextHelperData.ContextStudentDasboard)
	}

	StackView.onActivated: {
		//Client.contextHelper.setCurrentContext(ContextHelperData.ContextStudentDasboard)
	}

	StackView.onDeactivated: {

	}
}



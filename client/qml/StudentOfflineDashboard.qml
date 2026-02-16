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


		Qaterial.LabelBody1 {
			visible: engine && !engine.allPermitValid
			anchors.horizontalCenter: parent.horizontalCenter
			width: Math.min(parent.width-100, Qaterial.Style.maxContainerSize)
			horizontalAlignment: Qt.AlignHCenter
			wrapMode: Text.Wrap

			text: qsTr("Az adatbázis egy része zárolva van, szinkronizálj a feloldáshoz")
			color: Qaterial.Style.errorColor
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

					readonly property bool isValid: campaign && engine && engine.checkCampaignValid(campaign)

					icon.source: isValid ?
									 Qaterial.Icons.trophyBroken :
									 Qaterial.Icons.lockAlertOutline

					bgColor: isValid ? Qaterial.Style.buttonColor : Qaterial.Colors.red500

					DashboardTimingTag {
						anchors.right: parent.right
						anchors.rightMargin: Qaterial.Style.card.horizontalPadding
						anchors.top: parent.top
						anchors.topMargin: Qaterial.Style.card.horizontalPadding
						z: 99

						campaign: _btn.campaign
						referenceDate: _referenceDate
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

				readonly property bool isValid: engine && engine.checkCampaignValid(null)

				icon.source: isValid ?
								 Qaterial.Icons.controller :
								 Qaterial.Icons.lockAlertOutline

				highlighted: false
				outlined: true
				flat: true

				textColor: isValid ? Qaterial.Colors.green500 : Qaterial.Colors.red500

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



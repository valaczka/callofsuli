import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QBasePage {
	id: control

	defaultTitle: qsTr("Rangsor")

	implicitWidth: 1000

	mainToolBarComponent: Row {
		QToolButton {
			action: actionRankList
			ToolTip.text: qsTr("Ranglista")
		}

		UserButton {
			userDetails: userData
			userNameVisible: control.width>800
		}
	}

	UserDetails {
		id: userData
	}


	activity: Scores {
		id: scores

		property VariantMapModel modelRankList: newModel([
															 "rankid",
															 "rankimage",
															 "rankname",
															 "ranklevel",
															 "xp"
														 ])



		onGetUserScore: {
			details.loadUserScore(jsonData)
		}

		Component.onCompleted: {
			modelRankList.setVariantList(cosClient.rankList, "rankid")
		}
	}

	QSwipeComponent {
		id: swComponent
		anchors.fill: parent

		content: [
			ScoreList {
				id: container1
				reparented: swComponent.swipeMode
				reparentedParent: placeholder1

				list.onRefreshRequest: scores.send("getAllUser")

				onUserSelected: {
					scores.send("getUserScore", {username: username})
					swComponent.swipeToPage(1)
				}
			},

			QSwipeContainer {
				id: container2
				reparented: swComponent.swipeMode
				reparentedParent: placeholder2
				title: qsTr("Eredmények")
				icon: CosStyle.iconXPgraph
				ScoreDetails {
					id: details
					anchors.fill: parent
				}
			}
		]

		swipeContent: [
			Item { id: placeholder1 },
			Item { id: placeholder2 }
		]

		tabBarContent: [
			QSwipeButton { swipeContainer: container1 },
			QSwipeButton { swipeContainer: container2 }
		]

	}

	onPageActivated: {
		scores.send("getAllUser")
		container1.list.forceActiveFocus()
	}



	SortFilterProxyModel {
		id: rankProxyModel

		sourceModel: scores.modelRankList

		sorters: [
			RoleSorter { roleName: "rankid" }
		]
		proxyRoles: [
			ExpressionRole {
				name: "details"
				expression: model.ranklevel ? "level "+model.ranklevel : ""
			},
			ExpressionRole {
				name: "readableXP"
				expression: (model.xp || model.ranklevel) ? Number(model.xp).toLocaleString() +" XP" : ""
			},
			ExpressionRole {
				name: "imgSource"
				expression: cosClient.rankImageSource(model.rankid, model.ranklevel, model.rankimage)
			}
		]
	}



	Action {
		id: actionRankList
		icon.source: CosStyle.iconRank
		onTriggered: {
			var d = JS.dialogCreateQml("List", {
										   roles: ["rankid", "rankname", "details", "imgSource"],
										   icon: actionRankList.icon.source,
										   title: qsTr("Ranglista"),
										   selectorSet: false,
										   modelTitleRole: "rankname",
										   modelImageRole: "imgSource",
										   modelSubtitleRole: "details",
										   modelRightTextRole: "readableXP",
										   delegateHeight: CosStyle.twoLineHeight,
									   })

			d.item.list.model = rankProxyModel

			d.open()
		}
	}

	function windowClose() {
		return false
	}

	function pageStackBack() {
		if (swComponent.layoutBack())
			return true

		return false
	}
}


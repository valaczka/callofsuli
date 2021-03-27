import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QSwipeContainer {
	id: panel

	title: qsTr("Résztvevők")
	icon: CosStyle.iconUsers


	SortFilterProxyModel {
		id: scoreProxyModel
		sourceModel: studentMaps.modelUserList

		sorters: [
			RoleSorter { roleName: "rankid"; sortOrder: Qt.DescendingOrder; priority: 2 },
			RoleSorter { roleName: "sumxp"; sortOrder: Qt.DescendingOrder; priority: 1 }
		]

		proxyRoles: [
			ExpressionRole {
				name: "name"
				expression: model.nickname.length ? model.nickname : model.firstname+" "+model.lastname
			},
			SwitchRole {
				name: "background"
				filters: ValueFilter {
					roleName: "username"
					value: cosClient.userName
					SwitchRole.value: JS.setColorAlpha(CosStyle.colorWarningDark, 0.4)
				}
				defaultValue: "transparent"
			},
			SwitchRole {
				name: "titlecolor"
				filters: ValueFilter {
					roleName: "username"
					value: cosClient.userName
					SwitchRole.value: CosStyle.colorAccentLight
				}
				defaultValue: CosStyle.colorAccentLighter
			}

		]
	}


	QVariantMapProxyView {
		id: scoreList
		anchors.fill: parent

		refreshEnabled: true
		delegateHeight: CosStyle.twoLineHeight
		//numbered: true

		leftComponent: Image {
			source: model ? cosClient.rankImageSource(model.rankid, model.ranklevel, model.rankimage) : ""
			width: scoreList.delegateHeight+10
			height: scoreList.delegateHeight*0.7
			fillMode: Image.PreserveAspectFit
		}

		rightComponent: Column {
			QLabel {
				text: model.sumxp+" XP"
				font.weight: Font.Normal
				font.pixelSize: CosStyle.pixelSize
				color: CosStyle.colorAccent
				leftPadding: 5
			}
			QLabel {
				text: model.t1+" "+model.t2+" "+model.t3+" "+model.d1+" "+model.d2+" "+model.d3
				font.weight: Font.DemiBold
				font.pixelSize: CosStyle.pixelSize*0.8
			}
		}

		model: scoreProxyModel
		modelTitleRole: "name"
		modelBackgroundRole: "background"
		modelTitleColorRole: "titlecolor"
		colorSubtitle: CosStyle.colorAccentDark

		highlightCurrentItem: false

		onRefreshRequest: studentMaps.send("userListGet", { groupid: studentMaps.selectedGroupId })
	}



}




import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QSwipeContainer {
	id: panel

	title: qsTr("Rangsor")
	icon: CosStyle.iconTrophy

	property alias list: scoreList


	signal userSelected(string username)

	SortFilterProxyModel {
		id: scoreProxyModel
		sourceModel: scores.scoreModel
		filters: [
			AllOf {
				ValueFilter {
					roleName: "isTeacher"
					value: false
				}
				ValueFilter {
					roleName: "isAdmin"
					value: false
				}
			}
		]

		sorters: [
			RoleSorter { roleName: "rankid"; sortOrder: Qt.DescendingOrder; priority: 2 },
			RoleSorter { roleName: "xp"; sortOrder: Qt.DescendingOrder; priority: 1 }
		]

		proxyRoles: [
			ExpressionRole {
				name: "details"
				expression: model.rankname+(model.ranklevel > 0 ? " (lvl "+model.ranklevel+")": "")
			},
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
		numbered: true

		leftComponent: Image {
			source: model ? cosClient.rankImageSource(model.rankid, model.ranklevel, model.rankimage) : ""
			width: scoreList.delegateHeight+10
			height: scoreList.delegateHeight*0.7
			fillMode: Image.PreserveAspectFit
		}

		rightComponent: QLabel {
			text: Number(model.xp).toLocaleString()+" XP"
			font.weight: Font.Normal
			font.pixelSize: scoreList.delegateHeight*0.5
			color: CosStyle.colorAccent
			leftPadding: 5
		}

		model: scoreProxyModel
		modelTitleRole: "name"
		modelSubtitleRole: "details"
		modelBackgroundRole: "background"
		modelTitleColorRole: "titlecolor"
		colorSubtitle: CosStyle.colorAccentDark

		highlightCurrentItem: false

		onClicked: {
			userSelected(scoreList.model.get(index).username)
		}
	}

}

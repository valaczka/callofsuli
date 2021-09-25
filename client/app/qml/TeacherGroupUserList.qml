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
	icon: CosStyle.iconGroup

	property alias list: userList

	QVariantMapProxyView {
		id: userList
		anchors.fill: parent

		refreshEnabled: true
		delegateHeight: CosStyle.twoLineHeight
		//numbered: true

		leftComponent: Image {
			source: model ? cosClient.rankImageSource(model.rankid, model.ranklevel, model.rankimage) : ""
			width: userList.delegateHeight+10
			height: userList.delegateHeight*0.8
			fillMode: Image.PreserveAspectFit
		}

		model: SortFilterProxyModel {
			id: scoreProxyModel
			sourceModel: teacherGroups.modelUserList

			sorters: [
				StringSorter { roleName: "name" }
			]

			proxyRoles: [
				ExpressionRole {
					name: "name"
					expression: model.firstname+" "+model.lastname
				},
				SwitchRole {
					name: "titlecolor"
					filters: ValueFilter {
						roleName: "activeClient"
						value: true
						SwitchRole.value: CosStyle.colorOK
					}
					defaultValue: CosStyle.colorPrimaryLighter
				}
			]
		}

		modelTitleRole: "name"
		modelSubtitleRole: "nickname"
		modelTitleColorRole: "titlecolor"
		modelSubtitleColorRole: "titlecolor"

		highlightCurrentItem: false

		onRefreshRequest: teacherGroups.send("groupGet", { id: teacherGroups.selectedGroupId })
	}



}




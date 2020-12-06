import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	maximumWidth: 600

	//title: qsTr("Szerverek")
	//icon: CosStyle.iconUsers

	/*contextMenuFunc: function (m) {
		m.addAction(actionServerNew)
	}*/



	/*SortFilterProxyModel {
		id: userProxyModel
		sourceModel: servers.serversModel
		filters: [
			RegExpFilter {
				enabled: mainSearch.text.length
				roleName: "name"
				pattern: mainSearch.text
				caseSensitivity: Qt.CaseInsensitive
				syntax: RegExpFilter.FixedString
			}
		]
		sorters: [
			StringSorter { roleName: "name" }
		]
		proxyRoles: ExpressionRole {
			name: "details"
			expression: model.host+":"+model.port+(model.username.length ? " - "+model.username : "")
		}
	}*/


	/*QPagePanelSearch {
		id: toolbar

		//listView: list

		//labelCountText: servers.serversModel.selectedCount

		//onSelectAll: servers.serversModel.selectAllToggle()

	}


	QToolButtonBig {
		anchors.centerIn: parent
		visible: !serverModel.count
		action: actionServerNew
	}*/



	Action {
		id: actionServerNew
		text: qsTr("Új szerver")
		onTriggered: {
		}
	}




	onPanelActivated: {
	}
}




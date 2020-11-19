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


	QPageHeader {
		id: header

		//isSelectorMode: serverList.selectorSet

		//labelCountText: servers.serversModel.selectedCount

		/*mainItem: QTextField {
			id: mainSearch
			width: parent.width

			lineVisible: false
			clearAlwaysVisible: true

			placeholderText: qsTr("Keresés...")
		}*/

		//onSelectAll: servers.serversModel.selectAllToggle()
	}

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




	Action {
		id: actionServerNew
		text: qsTr("Új szerver")
		onTriggered: {
		}
	}




	onPanelActivated: {
	}
}




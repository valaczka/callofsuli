import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: page

	property StudentMaps studentMaps: null

	mainToolBarComponent: UserButton {
		userDetails: userData
		userNameVisible: page.width>800
	}

	UserDetails {
		id: userData
	}

	property list<Component> cmps: [
		Component { MapMissionList {
				panelVisible: true
				Connections {
					target: page
					function onPageActivated() {
						list.forceActiveFocus()
					}
				}
			} },
		Component { MapMissionInfo {
				panelVisible: true
			} }
	]




	/*mainMenuFunc: function (m) {
			m.addAction(actionSave)
		}*/



	onPageActivated: {
		if (!panelComponents.length)
			panelComponents = cmps
		if (studentMaps)
			studentMaps.getMissionList()
	}






	function windowClose() {
		return false
	}


	function pageStackBack() {
		return false
	}

}

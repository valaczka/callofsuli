import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: page

	defaultTitle: qsTr("Pályák kezelése")

	activity: TeacherMaps {
		id: teacherMaps

		property VariantMapModel modelMapList: newModel([
															"rowid",
															"uuid",
															"name",
															"version",
															"draft"
														])

		onMapListLoaded: {
			modelMapList.unselectAll()
			modelMapList.setVariantList(list, "rowid");
		}
	}


	panelComponents: [
		Component { TeacherMapList {
				panelVisible: true
			} }
	]




	/*mainMenuFunc: function (m) {
		m.addAction(actionSave)
	}*/



	onPageActivated: {
		teacherMaps.send(CosMessage.ClassTeacherMap, "mapListGet")
	}



	function windowClose() {
		return false
	}


	function pageStackBack() {
		return false
	}

}

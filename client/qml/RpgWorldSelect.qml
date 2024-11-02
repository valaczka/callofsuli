import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


QPage {
	id: root

	property alias world: _map.world

	title: _map.selectedLand ? (_map.selectedLand.name != "" ?
									_map.selectedLand.name.split("\n").join(" ") :
									qsTr("[névtelen terület]"))
							 : ""


	appBar.backButtonVisible: true
	appBar.background: null

	appBar.rightComponent: Qaterial.AppBarButton
	{
		action: _actionSelect
		backgroundColor: enabled ? Qaterial.Colors.green500 : "transparent"
	}

	RpgUserWorldMap {
		id: _map
		anchors.fill: parent
		onLandSelected: (land) => {
							_map.selectedLand = land
							_actionSelect.trigger()
						}
	}

	Action {
		id: _actionSelect
		icon.source: Qaterial.Icons.checkBold
		enabled: _map.selectedLand && (
					 _map.selectedLand.landState == RpgWorldLandData.LandSelectable ||
					 _map.selectedLand.landState == RpgWorldLandData.LandAchieved)

		onTriggered: {
			world.selectLand(_map.selectedLand)
			Client.stackPop()
		}
	}

}



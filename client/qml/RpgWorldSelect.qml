import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


QPage {
	id: root

	property RpgUserWorld world: null
	property RpgUserWorldMap _map: null

	title: _map && _map.selectedLand ? (_map.selectedLand.name != "" ?
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

	Connections {
		target: _map

		function onLandSelected(land) {
			_map.selectedLand = land
			_actionSelect.trigger()
		}
	}

	/*RpgUserWorldMap {
		id: _map
		anchors.fill: parent
	}*/

	Action {
		id: _actionSelect
		icon.source: Qaterial.Icons.checkBold
		enabled: _map && _map.selectedLand && (
					 _map.selectedLand.landState == RpgWorldLandData.LandSelectable ||
					 _map.selectedLand.landState == RpgWorldLandData.LandAchieved)

		onTriggered: {
			world.selectLand(_map.selectedLand)
			Client.stackPop()
		}
	}


	function loadMap() {
		if (_map)
			return

		_map = world.getCachedMapItem()

		if (!_map)
			return

		if (!_map._loaded)
			Qaterial.DialogManager.openBusyIndicator({
														 text: qsTr("Betöltés...")
													 })

		_map.parent = root
		_map.updateSelectedLand()
		_map.visible = true
		_map.enabled = true
	}

	onWorldChanged: loadMap()

	Component.onCompleted: loadMap()


	StackView.onRemoved: {
		_map.visible = false
		_map.enabled = false
		_map.parent = null
	}

	StackView.onActivated: {
		Client.contextHelper.setCurrentContext(ContextHelperData.ContextStudentSelectWorld)
	}

	StackView.onDeactivating: {
		Client.contextHelper.unsetContext(ContextHelperData.ContextStudentSelectWorld)
	}
}



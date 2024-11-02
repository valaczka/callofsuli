import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

ConquestLandImpl {
	id: root

	implicitWidth: _imgMap.source.toString() != "" ? _imgMap.width : 100
	implicitHeight: _imgMap.source.toString() != "" ? _imgMap.height : 100

	readonly property ConquestGameAdjacencySetup _setup: landData && landData.game && landData.game instanceof ConquestGameAdjacencySetup ?
															 landData.game :
															 null

	readonly property bool _setupPicked: _setup && landData && _setup.currentAdjacency.includes(landData.landId)

	readonly property bool _pickable: (_setup && landData && landData.landId != _setup.currentLandId) ||
									  (landData && landData.game &&
									   landData.game.currentTurn.canPick.includes(landData.landId) &&
									   landData.game.currentTurn.player === landData.game.playerId)

	readonly property bool _picked: landData && landData.game &&
									landData.game.currentTurn.pickedId === landData.landId

	ownColor: Qaterial.Style.iconColor()

	signal imageLoaded()

	SequentialAnimation {
		running: !_setup
		loops: Animation.Infinite

		ColorAnimation {
			target: root
			property: "ownColor"
			to: Qaterial.Style.accentColor
			duration: 225
		}

		PauseAnimation {
			duration: 125
		}

		ColorAnimation {
			target: root
			property: "ownColor"
			to: Qaterial.Style.iconColor()
			duration: 225
		}
	}

	Image {
		id: _imgMap
		fillMode: Image.PreserveAspectFit
		visible: false
		source: landData ? landData.imgMap : ""
		width: sourceSize.width
		height: sourceSize.height
		asynchronous: true
		onStatusChanged: if (_imgMap.status == Image.Ready && _imgBorder.status == Image.Ready) imageLoaded()
	}

	ColorOverlay {
		anchors.fill: _imgMap
		source: _imgMap
		color: _setupPicked ? Qaterial.Colors.green500 :
							  landData && _setup && _setup.currentLandId == landData.landId ?
								  Qaterial.Colors.amber400 :
								  root.baseColor
		opacity: 0.6
		visible: active || _setupPicked || landData && _setup && _setup.currentLandId == landData.landId
	}


	Image {
		id: _imgBorder
		width: _imgMap.width
		height: _imgMap.height
		fillMode: Image.PreserveAspectFit
		visible: false
		source: landData ? landData.imgBorder : ""
		asynchronous: true
		onStatusChanged: if (_imgMap.status == Image.Ready && _imgBorder.status == Image.Ready) imageLoaded()
	}

	ColorOverlay {
		id: _overlayBorder
		anchors.fill: _imgBorder
		source: _imgBorder
		color: _setupPicked ? Qaterial.Colors.green500 :
							  landData && _setup && _setup.currentLandId == landData.landId ?
								  Qaterial.Colors.amber400 :
								  _pickable || _picked ? root.ownColor : root.baseColor
		visible: false
	}


	ColorOverlay {
		id: _hover
		anchors.fill: _imgMap
		source: _imgMap
		color: Qaterial.Colors.white
		opacity: _pickable && _mouse.containsMouse ? 0.5 : 0.0
		visible: _imgMap.source.toString() != ""

		Behavior on opacity {
			NumberAnimation { duration: 250; easing.type: Easing.InQuad }
		}
	}

	OpacityMask {
		anchors.fill: _overlayBorder
		source: _overlayBorder
		maskSource: _imgMap
		visible: active || _pickable || _picked || _setupPicked || landData && _setup && _setup.currentLandId == landData.landId
	}

	Qaterial.LabelHeadline4 {
		anchors.centerIn: _imgMap
		visible: _setup && landData
		text: landData ? landData.landId : ""
		color: Qaterial.Colors.amber400
	}

	MaskedMouseArea {
		id: _mouse
		anchors.fill: _imgMap
		maskSource: _imgMap.source
		enabled: _pickable

		onClicked: {
			if (!landData || !landData.game)
				return

			if (_setup) {
				_setup.adjacencyToggle(landData.landId)
				return
			}

			landData.game.sendWebSocketMessage({
												   cmd: "pick",
												   engine: landData.game.engineId,
												   id: landData.landId
											   })
		}
	}
}

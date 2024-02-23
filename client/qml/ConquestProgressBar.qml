import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Qaterial.ProgressBar {
	id: root

	property ConquestGame game: null

	readonly property bool _active: {
		if (!game)
			return false

		if (game.currentTurn.subStageStart <= 0 || game.currentTurn.subStageEnd <= 0)
			return false

		if (game.currentStage == ConquestTurn.StagePick && game.currentTurn.subStage === ConquestTurn.SubStageUserAnswer)
			return true

		if ((game.currentTurn.player === game.playerId || game.isAttacked) &&
				(game.currentTurn.subStage === ConquestTurn.SubStageUserSelect ||
				 game.currentTurn.subStage === ConquestTurn.SubStageUserAnswer))
			return true

		return false
	}

	visible: _active

	color: Qaterial.Style.iconColor()
	from: game ? game.currentTurn.subStageStart : 0
	to: game ? game.currentTurn.subStageEnd : 0

	Behavior on value {
		NumberAnimation {
			duration: game ? game.tickTimerInterval : 100
			easing.type: Easing.Linear
		}
	}

	Timer {
		running: root._active
		interval: game ? game.tickTimerInterval : 0
		repeat: true
		triggeredOnStart: true
		onTriggered: {
			root.value = Math.max(root.from + root.to - game.currentTick(), root.from)
		}
	}
}

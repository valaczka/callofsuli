import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli


Item {
	id: root

	// [{target: <Item>, title: <string>, text: <string>, [id: <int>], [after: <int>]},...]
	property bool active: false
	property var list: []
	property string page: "untitled"


	// internal
	property int _step: 0
	property int _lastId: -1
	property int _lastSeen: -1
	property bool _success: true
	readonly property int _version: Client.Utils.versionCode()

	readonly property string _settingsPrefix: "notification/tour_"+page+"_"


	SpotlightCoachMark {
		id: _coach

		onDismissed: close()
		onClosed: next()
	}


	Timer {
		id: _timer
		interval: 1000
		repeat: false
		onTriggered: next()
	}

	function start() {
		active = true
	}


	function next() {
		while (true) {
			if (_step >= list.length) {
				if (_lastId > -1) {
					save()
				}

				active = false
				_step = 0
				_lastId = -1
				_lastSeen = -1
				_success = true

				return
			}

			const d = list[_step]

			let id = 0
			let minId = -1

			if (d.id !== undefined)
				id = d.id

			if (d.after !== undefined)
				minId = d.after

			let seen = Client.Utils.settingsGet(_settingsPrefix+id, 0)

			if (_lastId != -1 && id != _lastId)
				save()

			_lastId = id


			if (minId > -1 && _lastSeen < minId) {
				_success = false
			} else if (seen > 0) {
				_lastSeen = Math.max(_lastSeen, id)
			} else {
				if (load(d))
					break
				else
					_success = false
			}

			++_step
		}
	}

	function load(d) {
		if (_step >= list.length)
			return false

		if (!d.target || !d.target.visible)
			return false

		_coach.title = d.title
		_coach.text = d.text
		_coach.target = d.target			// -> updateGeometry()
		_coach.open()

		++_step

		return true
	}



	function save() {
		if (_lastId < 0)
			return

		if (_success)
			Client.Utils.settingsSet(_settingsPrefix+_lastId, _version)

		_success = true
	}

	onActiveChanged: {
		if (!active)
			return

		if (list.length == 0)
			return

		_timer.stop()
		_timer.start()
	}

}

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
	property int maxStep: 2


	// internal
	property int _step: 0
	property int _realStep: 0
	property int _lastId: -1
	property int _lastSeen: -1
	property bool _success: true

	readonly property string _settingsPrefix: "notification/tour_"+page


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
				_realStep = 0
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


			let lst = loadList()

			let seen = lst.includes(id)

			let finish = false

			if (_lastId != -1 && id != _lastId) {
				save()

				if (_realStep >= maxStep)
					finish = true
			}

			_lastId = id


			if (minId > -1 && _lastSeen < minId) {
				_success = false
			} else if (seen) {
				_lastSeen = Math.max(_lastSeen, id)
			} else {
				if (finish)
					break

				if (load(d))
					break
				else
					_success = false
			}

			++_step
		}
	}


	function loadList() {
		let l = Client.Utils.settingsGet(_settingsPrefix, "")

		if (l === "")
			return []

		return l.split(",").map(s => Number(s.trim()))
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
		++_realStep

		return true
	}



	function save() {
		if (_lastId < 0)
			return

		if (_success) {
			let lst = loadList()

			if (!lst.includes(_lastId))
				lst.push(_lastId)
			Client.Utils.settingsSet(_settingsPrefix, lst.join(","))
		}

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

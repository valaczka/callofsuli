import QtQuick 2.15
import CallOfSuli 1.0

QtObject {
	id: root

	enum State {
		Invalid,
		LoadingLive,
		LoadingStatic,
		Live,
		Static,
		Error
	}

	property int state: QLiveStream.Invalid

	property EventStream eventStream: null
	property alias interval: _timer.interval

	property int api: WebSocket.ApiInvalid
	property string path: ""
	property var data: ({})

	property int maximumTries: 5

	property var reloadCallback: null


	property int _tries: 0


	property Timer _timer: Timer {
		id: _timer
		interval: 5000
		triggeredOnStart: true
		repeat: true
		running: root.state === QLiveStream.LoadingStatic || root.state === QLiveStream.Static
		onTriggered: {
			if (reloadCallback) {
				reloadCallback()
			}
			root.state = QLiveStream.Static
		}
	}

	property Connections _connections: Connections {
		target: eventStream

		function onConnected() {
			console.debug("EventStream connected")
			root.state = QLiveStream.Live
		}

		function onDisconnected() {
			console.debug("EventStream disconnected")

			if (root.state !== QLiveStream.LoadingLive) {
				root.state = QLiveStream.LoadingLive
				root.eventStream = null
				_try()
			}
		}
	}


	function load() {
		reload()
	}



	function reload() {
		if (eventStream) {
			eventStream.destroy()
			eventStream = null
		}

		_tries = 0

		if (root.state !== QLiveStream.LoadingLive) {
			root.state = QLiveStream.LoadingLive
			_try()
		}
	}


	function _try() {
		if (Qt.platform.os == "wasm" || Qt.platform.os == "ios") {
			console.debug("Platform doesn't support EventStream, switch to static")
			root.state = QLiveStream.LoadingStatic
			return
		}

		if (state === QLiveStream.Static) {
			console.warn("Static state, can't establish EventStream")
			return
		}

		if (state === QLiveStream.Live || eventStream) {
			console.warn("Already established EventStream")
			return
		}

		if (_tries >= maximumTries) {
			console.debug("Maximum tries reached, switch to static")
			root.state = QLiveStream.LoadingStatic
			return
		}

		_tries++

		eventStream = Client.webSocket.getEventStream(api, path, data)

		if (!eventStream) {
			console.warn("Error occurred")
			root.state = QLiveStream.Error
			return
		}
	}

	Component.onDestruction: {
		if (eventStream) {
			eventStream.destroy()
			eventStream = null
		}
	}

}

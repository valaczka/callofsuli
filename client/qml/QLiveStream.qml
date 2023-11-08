import QtQuick 2.15
import CallOfSuli 1.0

QtObject {
	id: root

	enum State {
		Invalid,
		LoadingLive,
		LoadingStatic,
		Connected,
		Live,
		Static,
		Error
	}

	property int state: QLiveStream.Invalid

	property EventStream eventStream: null
	property alias interval: _timer.interval

	property int api: HttpConnection.ApiInvalid
	property string path: ""
	property var data: ({})

	property int maximumTries: 2

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


	property Timer _helloTimer: Timer {
		id: _helloTimer
		interval: 5000
		triggeredOnStart: false
		running: false
		onTriggered: {
			console.warn("EventStream hello message not received")

			if (eventStream) {
				eventStream.destroy()
				eventStream = null
			}

			if (root.state !== QLiveStream.LoadingLive) {
				root.state = QLiveStream.LoadingLive
			}
			_try()
		}
	}

	property Connections _connections: Connections {
		target: eventStream

		function onConnected() {
			console.debug("EventStream connected")
			root.state = QLiveStream.Connected
			_helloTimer.start()
		}

		function onDisconnected() {
			console.debug("EventStream disconnected")
			_helloTimer.stop()

			if (root.state !== QLiveStream.LoadingLive) {
				root.state = QLiveStream.LoadingLive
				root.eventStream = null
				_try()
			}
		}

		function onEventHelloReceived() {
			if (root.state !== QLiveStream.Connected)
				console.warn("EventStream state error")

			console.debug("EventStream live")
			root.state = QLiveStream.Live
			_helloTimer.stop()
		}
	}


	function load() {
		reload()
	}



	function reload() {
		if (root.state === QLiveStream.LoadingStatic || root.state === QLiveStream.Static) {
			if (reloadCallback)
				reloadCallback()

			root.state = QLiveStream.Static
			return
		}

		_helloTimer.stop()

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
		if (Qt.platform.os == "wasm") {
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

		eventStream = Client.httpConnection.getEventStream(api, path, data)

		if (!eventStream) {
			console.warn("Error occurred")
			root.state = QLiveStream.Error
			return
		}

		_helloTimer.start()
	}

	Component.onDestruction: {
		if (eventStream) {
			eventStream.destroy()
			eventStream = null
		}
	}

}

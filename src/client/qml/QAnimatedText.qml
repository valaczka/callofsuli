import QtQuick 2.12
import QtQuick.Controls 2.12

QLabel {
	id: txt

	property string str: ""
	property int durationRewind: 450
	property int durationRewindShort: 125
	property int durationForward: 750

	property int _length: 0

	readonly property bool running: anim.running


	onStrChanged: {
		resetStr()
	}

	on_LengthChanged: {
		if (_length === str.length)
			txt.text = str
		else
		{
			var i
			var t = ""
			var d = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890áéíöóőüúű"

			for (i=0; i<_length; i++) {
				var p = Math.floor(Math.random()*d.length)
				t += d.substring(p, p+1)
			}
			txt.text = t
		}
	}

	SequentialAnimation {
		id: anim

		PropertyAnimation {
			id: animRew
			duration: 1
			easing.type: Easing.InQuad
			target: txt
			property: "_length"
			from: 0
			to: 0
		}

		PropertyAnimation {
			id: animFwd
			duration: 0
			easing.type: Easing.InOutSine
			target: txt
			property: "_length"
			from: 0
			to: 0
		}
	}

	function resetStr() {
		animRew.duration = txt.text.length > 5 ? durationRewind :
												 txt.text.length ? durationRewindShort : 0
		animRew.from = txt.text.length

		animFwd.to = str.length
		animFwd.duration = Math.min(str.length * 75, durationForward)
		anim.start()
	}

}

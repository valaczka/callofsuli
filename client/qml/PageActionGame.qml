import QtQuick 2.7
import CallOfSuli 1.0

PageActionGameBase {
	id: control

	Component.onCompleted: {
		Client.playSound("qrc:/sound/voiceover/prepare_yourself.mp3", Sound.VoiceOver)
	}

}

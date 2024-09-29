AGENT_SIGN_FILE = $$PWD/../../../agent_sign.dat
AGENT_SIGN_QRC = $$OUT_PWD/agent_sign.qrc

exists($$AGENT_SIGN_FILE): {
	sign_lines = "<!DOCTYPE RCC><RCC version=\"1.0\">"
	sign_lines += "<qresource prefix=\"/sign\">"
	sign_lines += "<file alias=\"$$basename(AGENT_SIGN_FILE)\">$${AGENT_SIGN_FILE}</file>"
	sign_lines += "</qresource></RCC>"

	write_file($$AGENT_SIGN_QRC, sign_lines)

	RESOURCES += $$AGENT_SIGN_QRC
}


SQLITE = sqlite3

SOURCES = $(shell find -mindepth 2)

define ADD_FILE
$(SQLITE) $@ "INSERT INTO resource(folder, file, content) VALUES('$(shell echo $(1) | cut -d / -f 2)', '$(shell echo $(1) | cut -d / -f 3)', readfile('$(1)'));";
echo "add resource: $(1)";
endef

all: default.db

distclean: clean

clean:
	$(RM) default.db


default.db:
	@$(SQLITE) $@ "CREATE TABLE resource(folder TEXT NOT NULL, file TEXT NOT NULL, content BLOB);"
	@$(foreach f, $(SOURCES), $(call ADD_FILE,$(f)))

FORCE:

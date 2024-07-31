#
# Various helper targets for deployment and debugging
# Source setuptarget.sh to use them!
#

#==============================================================================#
# Deployment targets

# Deploy bin to target:
.PHONY: deploy
deploy: build
ifdef TARGET_IP
	@sshpass -p $(TARGET_PWD) scp $(PROGS) $(TARGET_USR)@$(TARGET_IP):$(TARGET_DIR)
else
	$(error Please source setuptarget.sh first)
endif

# Deploy web:
.PHONY: deployweb
deployweb: web
ifdef TARGET_IP
	@sshpass -p $(TARGET_PWD) ssh $(TARGET_USR)@$(TARGET_IP) 'rm -rf $(TARGET_DIR)/html/*'
	@sshpass -p $(TARGET_PWD) scp -r ./html/* $(TARGET_USR)@$(TARGET_IP):$(TARGET_DIR)/html
else
	$(error Please source setuptarget.sh first)
endif

# Deploy sound server to target:
.PHONY: deploysnd
deploysnd: sndserv
ifdef TARGET_IP
	@sshpass -p $(TARGET_PWD) scp ./sndserv/sndserver $(TARGET_USR)@$(TARGET_IP):$(TARGET_DIR)
else
	$(error Please source setuptarget.sh first)
endif

# Deploy shell profile to target:
.PHONY: deployprofile
deployprofile:
ifdef TARGET_IP
	@sshpass -p $(TARGET_PWD) scp ./scripts/profile $(TARGET_USR)@$(TARGET_IP):/$(TARGET_USR)/.profile
	@sshpass -p $(TARGET_PWD) ssh -t $(TARGET_USR)@$(TARGET_IP) 'sed -i "s/xxxxxx/$(PROGS)/g" /$(TARGET_USR)/.profile'
else
	$(error Please source setuptarget.sh first)
endif

# Deploy gdbserver to target:
.PHONY: deploygdb
deploygdb:
ifdef TARGET_IP
ifeq ($(APPTYPE), armv7hf)
	@sshpass -p $(TARGET_PWD) scp ./gdb/gdbserver_armv7 $(TARGET_USR)@$(TARGET_IP):/tmp/gdbserver
else ifeq ($(APPTYPE), aarch64)
	@sshpass -p $(TARGET_PWD) scp ./gdb/gdbserver_aarch64 $(TARGET_USR)@$(TARGET_IP):/tmp/gdbserver
else
	@echo "Error: Unsupported APPTYPE"
	@exit 1
endif
else
	$(error Please source setuptarget.sh first)
endif

#==============================================================================#
# Miscellaneous targets

# Logon to ACAP dir:
.PHONY: logon
logon:
ifdef TARGET_IP
	@sshpass -p $(TARGET_PWD) ssh -t $(TARGET_USR)@$(TARGET_IP) "cd $(TARGET_DIR) && sh"
else
	$(error Please source setuptarget.sh first)
endif

# Kill the app:
.PHONY: kill
kill:
ifdef TARGET_IP
	@sshpass -p $(TARGET_PWD) ssh $(TARGET_USR)@$(TARGET_IP) 'kill -KILL $$(pidof $(PROGS))'
else
	$(error Please source setuptarget.sh first)
endif

# Check target embedded SDK info:
.PHONY: checksdk
checksdk:
ifdef TARGET_IP
	@curl --anyauth --noproxy "*" -u $(TARGET_USR):$(TARGET_PWD) \
	'$(TARGET_IP)/axis-cgi/admin/param.cgi?action=list&group=Properties.EmbeddedDevelopment'
else
	$(error Please source setuptarget.sh first)
endif

# Open apps web (Linux):
.PHONY: openweb
openweb:
ifdef TARGET_IP
	@xdg-open http://$(TARGET_IP)/camera/index.html#/apps > /dev/null 2>&1
else
	$(error Please source setuptarget.sh first)
endif

# Trace logs on target:
.PHONY: log
log:
ifdef TARGET_IP
	@./scripts/log.py
else
	$(error Please source setuptarget.sh first)
endif

# Build ACAP release for all arch and put in a build dir:
.PHONY: release
release:
	@./scripts/make_acap_release.sh

#==============================================================================#
# Docker helpers

# Remove all stopped containers (not images):
.PHONY: dockerprune
dockerprune: checkdocker
	@docker container prune

# List all Docker images:
.PHONY: dockerlist
dockerlist: checkdocker
	@docker image list $(DOCKER_X32_IMG)
	@echo
	@docker image list $(DOCKER_X64_IMG)

# Run current Docker image:
.PHONY: dockerrun
dockerrun: checkdocker
ifeq ($(APPTYPE), armv7hf)
	@$(DOCKER_CMD) $(DOCKER_X32_IMG)
else ifeq ($(APPTYPE), aarch64)
	@$(DOCKER_CMD) $(DOCKER_X64_IMG)
else
	@echo "Error: Unsupported APPTYPE"
	@exit 1
endif

#==============================================================================#
# Code helpers

# Static code analysis using cppcheck:
.PHONY: cppcheck
cppcheck:
	@$(ECHO) "${PURPLE}*** Static code analysis${NC}"
	@cppcheck $(shell find . -name "*.[ch]") \
		--verbose --enable=all -DDEBUG=1 \
		--suppress=missingIncludeSystem \
		--suppress=unknownMacro \
		--suppress=unusedFunction

# Format code using clang-format:
.PHONY: indent
indent:
	@$(ECHO) "${PURPLE}*** Formatting code${NC}"
	@clang-format $(shell find . -name "*.[ch]") \
		-style=file -i -fallback-style=none

#==============================================================================#

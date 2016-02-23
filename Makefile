BROKER_DIR = broker
CLIENT_DIR = client

.PHONY: all clean

all:
	$(MAKE) -C $(BROKER_DIR)
	$(MAKE) -C $(CLIENT_DIR)

clean:
	$(MAKE) -C $(BROKER_DIR) clean
	$(MAKE) -C $(CLIENT_DIR) clean

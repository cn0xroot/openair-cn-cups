# imagem para rodar os binários
IMAGE_NAME=openair-cn-cups

# container name
CONTAINER_NAME=openair-cn-cups

# tag da imagem
IMAGE_TAG=latest

# diretório do projeto específico que contém o CMakeLists.txt
PROJECT_DIR=$(CURDIR)

# diretório de trabalho no container
WORK_CONTAINER_DIR=$(PROJECT_DIR)

# diretório de testes
TEST_DIR=$(PROJECT_DIR)/build/Debug

NUM_THREADS=8

# Interface to be configured with veth pair.
DEVICE_IN=enp0s20f0u9
DEVICE_OUT=veth0

.PHONY: help

help:
	@echo "Usage: make [comando]"
	@echo "Gerencia o container de build."
	@echo
	@echo "Comandos:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "- \033[36m%-30s\033[0m %s\n", $$1, $$2}'

# Função para realizar diversos tipos de build locais
# O primeiro parâmetro é o tipo do build
define build
	docker run -it --workdir $(WORK_CONTAINER_DIR) -v $(PROJECT_DIR):$(PROJECT_DIR)  \
	$(IMAGE_NAME):$(IMAGE_TAG) /bin/bash -c \
	'cd $(PROJECT_DIR)/build/scripts && \
	./build_spgwu -c -V -b Debug -j'
endef

docker-debug: ## Build in Debug mode
	docker-build
	$(call build)

docker-clean: ## Clean generated artifects
	docker run -it -v  $(PROJECT_DIR):$(PROJECT_DIR)   \
	$(IMAGE_CROSS) /bin/bash -c \
	'cd $(PROJECT_DIR)/build/Debug && make clean && \
	cd $(PROJECT_DIR)/build/Release && make clean'

docker-bash: ## Run container in bash mode
	docker run -it --workdir $(WORK_CONTAINER_DIR) -v $(PROJECT_DIR):$(PROJECT_DIR)   \
	$(IMAGE_NAME) --name=$(CONTAINER_NAME) /bin/bash
    
docker-login: ## Login in openair-cn-cups container
	docker exec -it openair-cn-cups  /bin/bash -c 'cd /workspaces/openair-cn-cups && /bin/bash'

docker-create-network: ## Create macvlan with subnet 192.168.15.0 using enp0s20f0u1 interface
	docker network create -d macvlan --subnet=192.168.15.0/24 --gateway=192.168.15.1 -o parent=enp0s20f0u8 macvlan-enp0s20f0u8

docker-setup-network: ## Connect maclan on coutainer openair-cn-cup and create spgwu interfaces
	docker network connect macvlan-enp0s20f0u8 openair-cn-cups

docker-config-spgwu-iface: ## Create and configure spgwu interefaces 
	$(PROJECT_DIR)/configs/config-spgwu-interface.sh 

docker-run-spwgu: ## Run container and run spgwu
	docker run -it --workdir $(WORK_CONTAINER_DIR) -v $(PROJECT_DIR):$(PROJECT_DIR)   \
	$(IMAGE_NAME) /bin/bash -c '$(PROJECT_DIR)/build/spgw_u/build/spgwu -c ./etc/spgw_u-dev.conf'
 
docker-build: ## Build docker image
	docker build --build-arg UID=$$(id -u) --build-arg GID=$$(id -g) -t $(IMAGE_NAME) docker/.

run-spgwu: ## Run spgwu 
	$(PROJECT_DIR)/build/spgw_u/build/spgwu -c ./etc/spgw_u-dev.conf -o

kill-spgwu: ## Kill spgwu
	echo "TODO" 

# TODO navarrothiago - include from upf-bpf, avoiding hardcoded. 
config-veth-pair: ## Config veth pair. It must be run before <run-*> targets
	sudo ./build/ext/upf-bpf/tests/scripts/config_veth_pair 

setup: docker-config-spgwu-iface config-veth-pair ## Install upf-bpf dependencies
	cd build/ext/upf-bpf/ && \
	make setup && \
	make install && \
	cd ../../../

clean-upf-bpf: 
	cd build/ext/upf-bpf/ && \
	make clean-all 

force-xdp-deload: ## Kill all and force deload XDP programs
	sudo ip link set dev $(DEVICE_IN) xdpgeneric off
	sudo ip link set dev $(DEVICE_OUT) xdpgeneric off

build-standalone-test: ## Build standalone test
	cmake -Btest/standalone/build -Htest/standalone/ && \
	cmake --build test/standalone/build --target spgwu_standalone_test

setup-interfaces: docker-config-spgwu-iface config-veth-pair ## Setup interfaces only
FROM ubuntu:18.04

ARG GIT_TAG=2019.w47
ARG UID=1000
ARG GID=1000
ARG UNAME=oai-spgwu

WORKDIR /tmp/$UNAME

RUN apt-get update && \
      apt-get -y install sudo git

# TODO: Instead of clone, we should download only de dependencies.
RUN git clone --depth=1 --branch=$GIT_TAG https://github.com/OPENAIRINTERFACE/openair-cn-cups.git

RUN git clone --depth=1 --branch=master https://github.com/gabime/spdlog.git openair-cn-cups/build/ext/spdlog

RUN apt-get update && \
      apt-get -y install cmake 

RUN apt-get update && \
      apt-get -y install gcc g++ 

RUN apt-get update && \
      apt-get -y install libconfig++

RUN cd openair-cn-cups/build/scripts && \
    ./build_spgwu -I -f

RUN apt-get update && \
      apt-get -y install iproute2

RUN apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 93C4A3FD7BB9C367 && \
      apt-get update && \
      apt-get -y install ansible

RUN apt-get update && \
      apt-get -y install iptables


# Creates openair-cn-cups user. The password is the username. 
#RUN useradd -m openair-cn-cups && echo "openair-cn-cups:openair-cn-cups" | chpasswd && adduser openair-cn-cups sudo

# The user and group `$UNAME` is created.
# The user `$UNAME` is added to the both the `$UNAME` and `sudo` group.
# The `uid` and `gid` is set to the value of `999`.
# The home directory is set to `/home/$UNAME`. 
# The shell is set to `/bin/bash`.
# The `sed` command does inline updates to the `/etc/sudoers` file to allow `$UNAME` and `root` users passwordless access to the `sudo` group.
# The `sed` command disables the `#includedir` directive that would allow any files in subdirectories to override these inline updates.
# Ref: https://superuser.com/questions/1296533/sudo-inside-docker-on-fedora-ask-for-password-on-host-it-doesnt-ask-password/1491777#1491777
RUN echo "GUI=${GID} and UID=${UID}"

RUN groupadd -g ${GID} $UNAME && useradd -u ${UID} -g $UNAME -G sudo -m -s /bin/bash $UNAME && \
    sed -i /etc/sudoers -re 's/^%sudo.*/%sudo ALL=(ALL:ALL) NOPASSWD: ALL/g' && \
    sed -i /etc/sudoers -re 's/^root.*/root ALL=(ALL:ALL) NOPASSWD: ALL/g' && \
    sed -i /etc/sudoers -re 's/^#includedir.*/## **Removed the include directive** ##"/g' && \
    echo "$UNAME ALL=(ALL:ALL) NOPASSWD: ALL" >> /etc/sudoers && \
    echo "Customized the sudoers file for passwordless access to the $UNAME user!" && \
    echo "$UNAME user:";  su - $UNAME -c id

USER root
#USER $UNAME

CMD /bin/bash

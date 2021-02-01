FROM debian:buster
RUN apt-get update && apt-get install -y \
    build-essential \
    googletest \
    cmake \
    && rm -rf /var/lib/apt/lists/*


RUN cd /usr/src/googletest;  mkdir build; cd build; cmake ..; make; make install


COPY . /init
COPY entrypoint.sh /entrypoint.sh
WORKDIR /workspace

VOLUME [ "/workspace" "/submission"]

ENTRYPOINT [ "/entrypoint.sh" ]
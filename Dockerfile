FROM ubuntu:16.04

RUN \
	apt-get update -y && \
	apt-get install -y wget && \
	apt-get install -y gcc-4.7 g++-4.7 gcc-4.7-base && \
apt-get install -y build-essential python-pip && \
apt-get install -y software-properties-common && \
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.7 100 && \
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.7 100

# Check out and build xercesc
ENV X_VERSION 2_8_0
WORKDIR /code
RUN wget http://archive.apache.org/dist/xerces/c/2/sources/xerces-c-src_${X_VERSION}.tar.gz
RUN tar -zxf xerces-c-src_${X_VERSION}.tar.gz
ENV XERCESCROOT=/code/xerces-c-src_${X_VERSION}
WORKDIR /code/xerces-c-src_${X_VERSION}/src/xercesc
RUN ./runConfigure -p linux -b 64
RUN make
RUN make install
ENV LD_LIBRARY_PATH /usr/local/lib:/$(LD_LIBRARY_PATH)
CMD ["/bin/bash"]

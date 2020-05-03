FROM debian:stretch

RUN apt update
RUN apt install -y git clang cmake make gcc g++ libmariadbclient-dev libssl1.0-dev libbz2-dev libreadline-dev libncurses-dev libboost-all-dev mariadb-server p7zip default-libmysqlclient-dev

RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang 100

# copy the sources from the host machine to the Docker container
ADD .git /trinitycore/.git
ADD dep /trinitycore/dep
ADD src /trinitycore/src
ADD cmake /trinitycore/cmake
ADD contrib /trinitycore/contrib
ADD PreLoad.cmake /trinitycore/PreLoad.cmake
ADD CMakeLists.txt /trinitycore/CMakeLists.txt
ADD revision_data.h.in.cmake /trinitycore/revision_data.h.in.cmake

ENTRYPOINT  cd trinitycore/build && \
            # run cmake
            cmake ../ -DCMAKE_INSTALL_PREFIX=/trinity-server -DTOOLS=1 && \
            # calculate the optimal number of threads
            MTHREADS=`grep -c ^processor /proc/cpuinfo`; MTHREADS=$(($MTHREADS + 2)) && \
            # run compilation
            make -j $MTHREADS && \
            make install -j $MTHREADS && \
            # copy the binary files "authserver" and "worldserver" files back to the host
            # - the directories "/binworldserver" and "/binauthserver" are meant to be bound to the host directories
            # - see docker-build to view the bindings
            cp /trinity-server/bin/worldserver /binworldserver && \
            cp /trinity-server/bin/authserver /binauthserver && \
	    cp /trinity-server/bin/mapextractor /binlib && \
	    cp /trinity-server/bin/vmap4assembler /binlib && \
	    cp /trinity-server/bin/vmap4extractor /binlib && \
	    cp /trinity-server/bin/mmaps_generator /binlib

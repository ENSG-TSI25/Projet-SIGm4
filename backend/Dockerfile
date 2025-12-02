FROM ubuntu:22.04



RUN apt-get update && apt-get install -y \
    g++ cmake make \
    gdal-bin libgdal-dev \
    libpq-dev libpqxx-dev \
    pkg-config \
    git \
    curl \
    sqlite3 libsqlite3-dev \
    libtiff-dev \
    libcurl4-openssl-dev \
    liblzma-dev \
    && apt-get clean
    
RUN mkdir -p /app/io/input /app/io/output

WORKDIR /opt

RUN git clone https://github.com/OSGeo/PROJ.git && \
    cd PROJ && \
    git checkout 9.5.1 && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release \
             -DBUILD_SHARED_LIBS=ON \
             -DBUILD_PROJSYNC=ON \
             -DENABLE_IPO=ON \
             -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make -j$(nproc) && \
    make install && \
    ldconfig

# Build de ton application
WORKDIR /app
COPY . .

RUN mkdir build \
    && cd build \
    && cmake .. \
    && make -j4 \
    && cp backend_app /app/backend_app

CMD ["/app/backend_app"]


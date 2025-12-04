FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ cmake make git \
    pkg-config \
    gdal-bin libgdal-dev \
    libpq-dev libpqxx-dev \
    libproj-dev \
    pkg-config \
    wget unzip \
    && apt-get clean
    
WORKDIR /app

COPY . .

RUN rm -rf build CMakeCache.txt CMakeFiles \
    && mkdir build \
    && cd build \
    && cmake .. -DBUILD_TESTS=ON \
    && make -j4 \
    && ctest --verbose \
    && cp backend_app /app/backend_app \
    && cp libsigm4_backend.a /app/libsigm4_backend.a

CMD ["/bin/sh", "-c", "/app/backend_app; tail -f /dev/null"]
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ cmake make git \
    pkg-config \
    gdal-bin libgdal-dev \
    libpq-dev libpqxx-dev \
    libproj-dev \
    && apt-get clean
    
    
WORKDIR /app

COPY . .

RUN mkdir build \
    && cd build \
    && cmake .. -DBUILD_TESTS=ON \
    && make -j4 \
    && cp backend_app /app/backend_app

CMD ["/app/backend_app"]

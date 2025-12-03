FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ cmake make \
    gdal-bin libgdal-dev \
    libpq-dev libpqxx-dev \
    libproj-dev \
    pkg-config \
    wget unzip \
    && apt-get clean

WORKDIR /app

COPY . .

RUN mkdir build \
    && cd build \
    && cmake .. \
    && make -j4 \
    && ctest --verbose \
    && cp backend_app /app/backend_app
    
CMD ["tail", "-f", "/dev/null"]

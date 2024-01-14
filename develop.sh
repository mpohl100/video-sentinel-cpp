#!/bin/bash

# Build the Docker image
docker build -t develop-image $(pwd)/develop

# Run the Docker container interactively with the /project directory mounted
docker run -it -v $(pwd)/project:/project develop-image
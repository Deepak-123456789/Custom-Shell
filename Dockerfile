FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# Install required packages
RUN apt update && apt install -y \
    build-essential \
    gcc \
    libreadline-dev \
    curl \
    nodejs \
    npm

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Compile the C shell
RUN gcc -o myshell shell_backend.c -lreadline

# Install Node dependencies
RUN npm install

# Set environment variable for Render
ENV PORT 10000

EXPOSE 10000

# Run the backend server
CMD ["node", "server.js"]

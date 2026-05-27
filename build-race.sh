#!/bin/bash
# Build script for Programming Challenge 1 - Particle Race Simulation

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Building Particle Race Simulation ===${NC}"
echo ""

# Create build directory
mkdir -p build-race
cd build-race

# Generate build files with CMake
echo -e "${BLUE}Running CMake...${NC}"
cmake .. 

if [ $? -ne 0 ]; then
    echo "CMake failed!"
    exit 1
fi

# Build the race_simulation target
echo -e "${BLUE}Building race_simulation...${NC}"
cmake --build . --target race_simulation

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo -e "${GREEN}Build successful!${NC}"
echo ""

# Run the simulation
echo -e "${BLUE}Running simulation...${NC}"
./race_simulation

echo ""
echo -e "${GREEN}Simulation completed!${NC}"

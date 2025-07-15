#compute

#version 430 core

layout (local_size_x = 1) in;

layout(std430, binding = 0) buffer Data {
    uint numbers[]; // numbers[0] is the accumulator
};

void main() {
    uint id = gl_GlobalInvocationID.x;

    if (id != 0) {
        atomicAdd(numbers[0], numbers[id]);
    }
}
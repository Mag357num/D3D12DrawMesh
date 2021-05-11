#pragma once
#include <cstdint>
#include <glm.hpp>
#include <ext/vector_float3.hpp>
#include <ext/matrix_float4x4.hpp>
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using FVector2 = glm::vec2;
using FVector = glm::vec3;
using FVector4 = glm::vec4;
using FMatrix = glm::mat4;
using FFloat4X4 = glm::mat4;

struct FRotator // order same with ue4
{
	float Roll;
	float Pitch;
	float Yaw;
};

struct FTransform
{
	FVector Translation;
	FRotator Rotation;
	FVector Scale;
};


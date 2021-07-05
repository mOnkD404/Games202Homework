#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <fstream>
#include <random>
#include "vec.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

const int resolution = 128;

Vec2f Hammersley(uint32_t i, uint32_t N) { // 0-1
    uint32_t bits = (i << 16u) | (i >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * 2.3283064365386963e-10;
    return {float(i) / float(N), rdi};
}

Vec3f ImportanceSampleGGX(Vec2f Xi, Vec3f N, Vec3f T, float roughness) {
    float a = roughness * roughness;

    //TODO: in spherical space - Bonus 1
    float theta = atan(a * sqrt(Xi.x) / sqrt(1.0 - Xi.x));
    float phi = 2.0 * M_PI * Xi.y;


    //TODO: from spherical space to cartesian space - Bonus 1
    Vec3f wi = Vec3f(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

    //TODO: tangent coordinates - Bonus 1
    Vec3f B = cross(N, T);
    // N是z轴，不需要变换了

    //TODO: transform H to tangent space - Bonus 1
    
    return normalize(wi);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    // TODO: To calculate Schlick G1 here - Bonus 1
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;

    float G = NdotV / (NdotV * (1.0 - k) + k);
    
    return G;
}

float GeometrySmith(float roughness, float NoV, float NoL) {
    float ggx2 = GeometrySchlickGGX(NoV, roughness);
    float ggx1 = GeometrySchlickGGX(NoL, roughness);

    return ggx1 * ggx2;
}

float DistributionGGX(Vec3f N, Vec3f H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = std::max(dot(N, H), 0.0f);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / std::max(denom, 0.0001f);
}

Vec3f IntegrateBRDF(Vec3f V, float roughness) {

    const int sample_count = 1024;
    Vec3f N = Vec3f(0.0, 0.0, 1.0);
    Vec3f T = Vec3f(0.0, 1.0, 0.0);
    Vec3f Res = Vec3f(0.0,0.0,0.0);
    Vec3f R0 = Vec3f(0.97, 0.98, 0.98); 
    float ResSum1 = 0.0;
    float ResSum2 = 0.0;
    for (int i = 0; i < sample_count; i++) {
        Vec2f Xi = Hammersley(i, sample_count);
        Vec3f H = ImportanceSampleGGX(Xi, N, T, roughness);
        Vec3f L = normalize(H * 2.0f * dot(V, H) - V);

        if (L.z < 0.0) continue;

        float NoL = std::max(L.z, 0.0f);
        float NoH = std::max(H.z, 0.0f);
        float VoH = std::max(dot(V, H), 0.0f);
        float NoV = std::max(dot(N, V), 0.0f);
        
        // TODO: To calculate (fr * ni) / p_o here - Bonus 1
        float LoH = std::max(dot(L, H), 0.0f);
        float G = GeometrySmith(roughness, NoV, NoL);
        float Dm = DistributionGGX(N, H, roughness);
        float pdfm = NoH * Dm;
        float pdfi = pdfm / (4.0 * LoH);

        // Split Sum - Bonus 2
        float Fr_F = G * Dm / ( 4.0 * NoV * NoL); 
        float CosTerm = pow(1.0 - dot(V, H), 5.0);
        float Sum1 = Fr_F * (1.0 - CosTerm) * NoL / pdfi;
        float Sum2 = Fr_F * CosTerm * NoL / pdfi;
        ResSum1 += Sum1;
        ResSum2 += Sum2;
    }
    Res = R0 * ResSum1 + Vec3f(ResSum2);

    return Res / sample_count;
}

int main() {
    uint8_t data[resolution * resolution * 3];
    float step = 1.0 / resolution;
    for (int i = 0; i < resolution; i++) {
        for (int j = 0; j < resolution; j++) {
            float roughness = step * (static_cast<float>(i) + 0.5f);
            float NdotV = step * (static_cast<float>(j) + 0.5f);
            Vec3f V = Vec3f(std::sqrt(1.f - NdotV * NdotV), 0.f, NdotV);

            Vec3f irr = IntegrateBRDF(V, roughness);

            data[(i * resolution + j) * 3 + 0] = uint8_t(irr.x * 255.0);
            data[(i * resolution + j) * 3 + 1] = uint8_t(irr.y * 255.0);
            data[(i * resolution + j) * 3 + 2] = uint8_t(irr.z * 255.0);
        }
    }
    stbi_flip_vertically_on_write(true);
    stbi_write_png("GGX_E_LUT.png", resolution, resolution, 3, data, resolution * 3);
    
    std::cout << "Finished precomputed!" << std::endl;
    return 0;
}
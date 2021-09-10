#include "denoiser.h"

Denoiser::Denoiser() : m_useTemportal(false) {}

void Denoiser::Reprojection(const FrameInfo &frameInfo) {
    int height = m_accColor.m_height;
    int width = m_accColor.m_width;
    Matrix4x4 preWorldToScreen =
        m_preFrameInfo.m_matrix[m_preFrameInfo.m_matrix.size() - 1];
    Matrix4x4 preWorldToCamera =
        m_preFrameInfo.m_matrix[m_preFrameInfo.m_matrix.size() - 2];

#pragma omp parallel for
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // TODO: Reproject
            m_valid(x, y) = false;
            m_misc(x, y) = Float3(0.f);

            int id = round(frameInfo.m_id(x, y));
            // no object
            if (id == -1)
                continue;

            Float3 worldCoord = frameInfo.m_position(x, y);
            Matrix4x4 curObjectToWorld = frameInfo.m_matrix[id];
            Float3 objectCoord = Inverse(curObjectToWorld)(worldCoord, Float3::Point);

            Float3 preScreenCoord = preWorldToScreen(
                m_preFrameInfo.m_matrix[id](objectCoord, Float3::Point), Float3::Point);

            int prex = preScreenCoord.x;
            int prey = preScreenCoord.y;
            // in screen
            if (prex < 0 || prex >= width || prey < 0 || prey >= height)
                continue;
            // object id
            int preObjectId = m_preFrameInfo.m_id(prex, prey);
            if (preObjectId != id)
                continue;

            m_misc(x, y) = m_accColor(prex, prey);
            m_valid(x, y) = true;
        }
    }
    std::swap(m_misc, m_accColor);
}

void Denoiser::TemporalAccumulation(const Buffer2D<Float3> &curFilteredColor) {
    int height = m_accColor.m_height;
    int width = m_accColor.m_width;
    int kernelRadius = 3;
#pragma omp parallel for
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // TODO: Temporal clamp
            Float3 color = m_accColor(x, y);
            // TODO: Exponential moving average
            float alpha = 1.0f;
            if (m_valid(x, y)) {
                Float3 avg;
                int count = 0;
                for (int clampy = 0; clampy < kernelRadius * 2; clampy++) {
                    int sampley = y + clampy - kernelRadius;
                    if (sampley < 0 || sampley >= height)
                        continue;
                    for (int clampx = 0; clampx < kernelRadius * 2; clampx++) {
                        int samplex = x + clampx - kernelRadius;
                        if (samplex < 0 || samplex >= width)
                            continue;

                        avg = avg + curFilteredColor(samplex, sampley);
                        count++;
                    }
                }
                if (count > 0) {
                    avg /= count;
                }
                // sigma
                Float3 sigma;
                count = 0;
                for (int clampy = 0; clampy < kernelRadius * 2; clampy++) {
                    int sampley = y + clampy - kernelRadius;
                    if (sampley < 0 || sampley >= height)
                        continue;
                    for (int clampx = 0; clampx < kernelRadius * 2; clampx++) {
                        int samplex = x + clampx - kernelRadius;
                        if (samplex < 0 || samplex >= width)
                            continue;

                        sigma = sigma + Sqr(curFilteredColor(samplex, sampley) - avg);
                        count++;
                    }
                }
                if (count > 0) {
                    sigma /= count;
                }

                color = Clamp(color, avg - sigma, avg + sigma * m_colorBoxK);
                alpha = m_alpha;
            }
            m_misc(x, y) = Lerp(color, curFilteredColor(x, y), alpha);
        }
    }
    std::swap(m_misc, m_accColor);
}

Buffer2D<Float3> Denoiser::Filter(const FrameInfo &frameInfo) {
    int height = frameInfo.m_beauty.m_height;
    int width = frameInfo.m_beauty.m_width;
    Buffer2D<Float3> filteredImage = CreateBuffer2D<Float3>(width, height);
    int kernelRadius = 16;
    int doubleKernelRadius = kernelRadius * 2;
#pragma omp parallel for
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // TODO: Joint bilateral filter
            Float3 sum_of_value;
            float sum_of_weight = 0.0;
            for (int ky = 0; ky < doubleKernelRadius; ky++) {
                int sampley = y + ky - kernelRadius;
                if (sampley >= 0 && sampley < height) {
                    for (int kx = 0; kx < doubleKernelRadius; kx++) {
                        int samplex = x + kx - kernelRadius;
                        if (samplex >= 0 && samplex < width) {

                            float sqrdistance =
                                SqrDistance(Float3{x, y, 0}, Float3{samplex, sampley, 0});
                            float sqrdistance2 =
                                SqrDistance(frameInfo.m_beauty(x, y),
                                            frameInfo.m_beauty(samplex, sampley));
                            float dnormal =
                                SafeAcos(Dot(frameInfo.m_normal(x, y),
                                             frameInfo.m_normal(samplex, sampley)));

                            Float3 itoj = frameInfo.m_position(samplex, sampley) -
                                          frameInfo.m_position(x, y);

                            float dplane =
                                Length(itoj) < 0.001
                                    ? 0.0
                                    : Dot(frameInfo.m_normal(x, y), Normalize(itoj));

                            float weight = exp(
                                -sqrdistance / (2.0 * m_sigmaCoord * m_sigmaCoord) -
                                sqrdistance2 / (2.0 * m_sigmaColor * m_sigmaColor) -
                                dnormal * dnormal /
                                    (2.0 * m_sigmaNormal * m_sigmaNormal) -
                                dplane * dplane / (2.0 * m_sigmaPlane * m_sigmaPlane));

                            sum_of_weight += weight;
                            sum_of_value += frameInfo.m_beauty(samplex, sampley) * weight;
                        }
                    }
                }
            }
            if (sum_of_weight > 0)
                sum_of_value = sum_of_value / sum_of_weight;
            filteredImage(x, y) = sum_of_value;
            // filteredImage(x, y) = frameInfo.m_beauty(x, y);
        }
    }
    return filteredImage;
}

void Denoiser::Init(const FrameInfo &frameInfo, const Buffer2D<Float3> &filteredColor) {
    m_accColor.Copy(filteredColor);
    int height = m_accColor.m_height;
    int width = m_accColor.m_width;
    m_misc = CreateBuffer2D<Float3>(width, height);
    m_valid = CreateBuffer2D<bool>(width, height);
}

void Denoiser::Maintain(const FrameInfo &frameInfo) { m_preFrameInfo = frameInfo; }

Buffer2D<Float3> Denoiser::ProcessFrame(const FrameInfo &frameInfo) {
    // Filter current frame
    Buffer2D<Float3> filteredColor;
    filteredColor = Filter(frameInfo);

    // Reproject previous frame color to current
    if (m_useTemportal) {
        Reprojection(frameInfo);
        TemporalAccumulation(filteredColor);
    } else {
        Init(frameInfo, filteredColor);
    }

    // Maintain
    Maintain(frameInfo);
    if (!m_useTemportal) {
        m_useTemportal = true;
    }
    return m_accColor;
}

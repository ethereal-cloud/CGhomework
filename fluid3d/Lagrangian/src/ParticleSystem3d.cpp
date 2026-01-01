#include "ParticleSystem3d.h"
#include <iostream>
#include <algorithm>
#include <Global.h>

namespace FluidSimulation
{
    namespace Lagrangian3d
    {
        ParticleSystem3d::ParticleSystem3d()
        {
        }

        ParticleSystem3d::~ParticleSystem3d()
        {
        }

        void ParticleSystem3d::setContainerSize(glm::vec3 corner = glm::vec3(0.0, 0.0, 0.0), glm::vec3 size = glm::vec3(1, 1, 1))
        {

            size *= Lagrangian3dPara::scale;

            lowerBound = corner;
            upperBound = corner + size;
            containerCenter = (lowerBound + upperBound) / 2.0f;
            size = upperBound - lowerBound;

            // 三个方向的block数量
            blockNum.x = floor(size.x / supportRadius);
            blockNum.y = floor(size.y / supportRadius);
            blockNum.z = floor(size.z / supportRadius);

            // 一个block的大小
            blockSize = glm::vec3(size.x / blockNum.x, size.y / blockNum.y, size.z / blockNum.z);

            blockIdOffs.resize(27);
            int p = 0;
            for (int k = -1; k <= 1; k++)
            {
                for (int j = -1; j <= 1; j++)
                {
                    for (int i = -1; i <= 1; i++)
                    {
                        blockIdOffs[p] = blockNum.x * blockNum.y * k + blockNum.x * j + i;
                        p++;
                    }
                }
            }

            particles.clear();
        }

        int32_t ParticleSystem3d::addFluidBlock(glm::vec3 lowerCorner, glm::vec3 upperCorner, glm::vec3 v0, float particleSpace)
        {

            lowerCorner *= Lagrangian3dPara::scale;
            upperCorner *= Lagrangian3dPara::scale;

            glm::vec3 size = upperCorner - lowerCorner;

            if (lowerCorner.x < lowerBound.x ||
                lowerCorner.y < lowerBound.y ||
                lowerCorner.z < lowerBound.z ||
                upperCorner.x > upperBound.x ||
                upperCorner.y > upperBound.y ||
                upperCorner.z > upperBound.z)
            {
                return 0;
            }

            glm::uvec3 particleNum = glm::uvec3(size.x / particleSpace, size.y / particleSpace, size.z / particleSpace);
            std::vector<particle3d> tempParticles(particleNum.x * particleNum.y * particleNum.z);

            Glb::RandomGenerator rand;
            int p = 0;
            for (int idX = 0; idX < particleNum.x; idX++)
            {
                for (int idY = 0; idY < particleNum.y; idY++)
                {
                    for (int idZ = 0; idZ < particleNum.z; idZ++)
                    {
                        float x = (idX + rand.GetUniformRandom()) * particleSpace;
                        float y = (idY + rand.GetUniformRandom()) * particleSpace;
                        float z = (idZ + rand.GetUniformRandom()) * particleSpace;
                        tempParticles[p].position = lowerCorner + glm::vec3(x, y, z);
                        tempParticles[p].blockId = getBlockIdByPosition(tempParticles[p].position);
                        tempParticles[p].density = Lagrangian3dPara::density;
                        tempParticles[p].velocity = v0;
                        p++;
                    }
                }
            }

            particles.insert(particles.end(), tempParticles.begin(), tempParticles.end());
            return particles.size();
        }

        uint32_t ParticleSystem3d::getBlockIdByPosition(glm::vec3 position)
        {
            if (position.x < lowerBound.x ||
                position.y < lowerBound.y ||
                position.z < lowerBound.z ||
                position.x > upperBound.x ||
                position.y > upperBound.y ||
                position.z > upperBound.z)
            {
                return -1;
            }

            glm::vec3 deltePos = position - lowerBound;
            uint32_t c = floor(deltePos.x / blockSize.x);
            uint32_t r = floor(deltePos.y / blockSize.y);
            uint32_t h = floor(deltePos.z / blockSize.z);
            return h * blockNum.x * blockNum.y + r * blockNum.x + c;
        }

        void ParticleSystem3d::updateBlockInfo()
        {
            const uint32_t totalBlocks = blockNum.x * blockNum.y * blockNum.z;
            blockExtens = std::vector<glm::uvec2>(totalBlocks, glm::uvec2(0, 0));
            if (particles.empty() || totalBlocks == 0)
            {
                return;
            }

            // Bucket sort by blockId for O(N) grouping.
            std::vector<uint32_t> counts(totalBlocks, 0);
            for (auto &p : particles)
            {
                uint32_t id = p.blockId;
                if (id >= totalBlocks)
                {
                    id = getBlockIdByPosition(p.position);
                    p.blockId = id;
                }
                if (id < totalBlocks)
                {
                    counts[id]++;
                }
            }

            std::vector<uint32_t> offsets(totalBlocks, 0);
            uint32_t running = 0;
            for (uint32_t i = 0; i < totalBlocks; i++)
            {
                offsets[i] = running;
                running += counts[i];
            }

            std::vector<particle3d> sorted(particles.size());
            std::vector<uint32_t> cursor = offsets;
            for (auto &p : particles)
            {
                uint32_t id = p.blockId;
                if (id < totalBlocks)
                {
                    sorted[cursor[id]++] = p;
                }
            }
            particles.swap(sorted);

            for (uint32_t i = 0; i < totalBlocks; i++)
            {
                blockExtens[i] = glm::uvec2(offsets[i], offsets[i] + counts[i]);
            }
        }

    }
}

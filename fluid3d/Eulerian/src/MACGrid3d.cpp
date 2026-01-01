#include "MACGrid3d.h"
#include "Configure.h"
#include <math.h>
#include <map>
#include <stdio.h>

namespace FluidSimulation
{
    namespace Eulerian3d
    {

        MACGrid3d::MACGrid3d()
        {
            cellSize = Eulerian3dPara::theCellSize3d;
            dim[0] = Eulerian3dPara::theDim3d[0];
            dim[1] = Eulerian3dPara::theDim3d[1];
            dim[2] = Eulerian3dPara::theDim3d[2];
            initialize();
        }

        MACGrid3d::MACGrid3d(const MACGrid3d &orig)
        {
            mU = orig.mU;
            mV = orig.mV;
            mW = orig.mW;
            mD = orig.mD;
            mT = orig.mT;
            mSolid = orig.mSolid;
        }

        MACGrid3d &MACGrid3d::operator=(const MACGrid3d &orig)
        {
            if (&orig == this)
            {
                return *this;
            }
            mU = orig.mU;
            mV = orig.mV;
            mW = orig.mW;
            mD = orig.mD;
            mT = orig.mT;
            mSolid = orig.mSolid;

            return *this;
        }

        MACGrid3d::~MACGrid3d()
        {
        }

        void MACGrid3d::reset()
        {
            mU.initialize(0.0);
            mV.initialize(0.0);
            mW.initialize(0.0);
            mD.initialize(0.0);
            mT.initialize(Eulerian3dPara::ambientTemp);
        }

        void MACGrid3d::updateSources()
        {
            for (int i = 0; i < Eulerian3dPara::source.size(); i++) {
                int x = Eulerian3dPara::source[i].position.x;
                int y = Eulerian3dPara::source[i].position.y;
                int z = Eulerian3dPara::source[i].position.z;
                mT(x, y, z) = Eulerian3dPara::source[i].temp;
                mD(x, y, z) = Eulerian3dPara::source[i].density;
                mU(x, y, z) = Eulerian3dPara::source[i].velocity.x;
                mV(x, y, z) = Eulerian3dPara::source[i].velocity.y;
                mW(x, y, z) = Eulerian3dPara::source[i].velocity.z;
            }
        }

        void MACGrid3d::createSolids()
        {
            mSolid.initialize();

            if (Eulerian3dPara::addSolid) {
                for (int k = dim[2] / 2 - 2; k <= dim[2] / 2 + 2; k++) {
                    for (int j = dim[1] / 2 - 2; j <= dim[1] / 2 + 2; j++) {
                        for (int i = dim[0] / 2 - 2; i <= dim[0] / 2 + 2; i++) {
                            mSolid(i, j, k) = 1;
                        }
                    }
                }
            }

        }

        void MACGrid3d::initialize()
        {
            reset();
            createSolids();
        }

        double MACGrid3d::getBoussinesqForce(const glm::vec3 &pos)
        {
            double temperature = getTemperature(pos);
            double smokeDensity = getDensity(pos);

            double zforce = -Eulerian3dPara::boussinesqAlpha * smokeDensity +
                            Eulerian3dPara::boussinesqBeta * (temperature - Eulerian3dPara::ambientTemp);

            return zforce;
        }

        double MACGrid3d::checkDivergence(int i, int j, int k)
        {
            double x1 = mU(i + 1, j, k);
            double x0 = mU(i, j, k);

            double y1 = mV(i, j + 1, k);
            double y0 = mV(i, j, k);

            double z1 = mW(i, j, k + 1);
            double z0 = mW(i, j, k);

            double xdiv = x1 - x0;
            double ydiv = y1 - y0;
            double zdiv = z1 - z0;
            double div = (xdiv + ydiv + zdiv) / cellSize;
            return div;
        }

        double MACGrid3d::getDivergence(int i, int j, int k)
        {

            double x1 = isSolidCell(i + 1, j, k) ? 0.0 : mU(i + 1, j, k);
            double x0 = isSolidCell(i - 1, j, k) ? 0.0 : mU(i, j, k);

            double y1 = isSolidCell(i, j + 1, k) ? 0.0 : mV(i, j + 1, k);
            double y0 = isSolidCell(i, j - 1, k) ? 0.0 : mV(i, j, k);

            double z1 = isSolidCell(i, j, k + 1) ? 0.0 : mW(i, j, k + 1);
            double z0 = isSolidCell(i, j, k - 1) ? 0.0 : mW(i, j, k);

            double xdiv = x1 - x0;
            double ydiv = y1 - y0;
            double zdiv = z1 - z0;
            double div = (xdiv + ydiv + zdiv) / cellSize;

            return div;
        }

        bool MACGrid3d::checkDivergence()
        {
            FOR_EACH_CELL
            {
                double div = checkDivergence(i, j, k);
                if (fabs(div) > 0.01)
                {
                    printf("Divergence(%d,%d,%d) = %.2f\n", i, j, k, div);
                    return false;
                }
            }
            return true;
        }

        glm::vec3 MACGrid3d::semiLagrangian(const glm::vec3 &pt, double dt)
        {
            glm::vec3 vel = getVelocity(pt);
            glm::vec3 pos = pt - vel * (float)dt;

            pos[0] = max(0.0, min((dim[0] - 1) * cellSize, pos[0]));
            pos[1] = max(0.0, min((dim[1] - 1) * cellSize, pos[1]));
            pos[2] = max(0.0, min((dim[2] - 1) * cellSize, pos[2]));

            int i, j, k;
            if (inSolid(pt, i, j, k))
            {
                double t = 0;
                if (intersects(pt, vel, i, j, k, t))
                {
                    pos = pt - vel * (float)t;
                }
                else
                {
                    Glb::Logger::getInstance().addLog("Error: something goes wrong during advection");
                }
            }
            return pos;
        }

        bool MACGrid3d::intersects(const glm::vec3 &wPos, const glm::vec3 &wDir, int i, int j, int k, double &time)
        {

            glm::vec3 pos = getCenter(i, j, k);

            glm::vec3 rayStart = wPos - pos; 
            glm::vec3 rayDir = wDir;

            double tmin = -9999999999.0;
            double tmax = 9999999999.0;

            double min = -0.5 * cellSize;
            double max = 0.5 * cellSize;

            for (int i = 0; i < 3; i++)
            {
                double e = rayStart[i];
                double f = rayDir[i];
                if (fabs(f) > 0.000000001)
                {
                    double t1 = (min - e) / f;
                    double t2 = (max - e) / f;
                    if (t1 > t2)
                        std::swap(t1, t2);
                    if (t1 > tmin)
                        tmin = t1;
                    if (t2 < tmax)
                        tmax = t2;
                    if (tmin > tmax)
                        return false;
                    if (tmax < 0)
                        return false;
                }
                else if (e < min || e > max)
                    return false;
            }

            if (tmin >= 0)
            {
                time = tmin;
                return true;
            }
            else
            {
                time = tmax;
                return true;
            }
            return false;
        }

        int MACGrid3d::getIndex(int i, int j, int k)
        {
            if (i < 0 || i > dim[0] - 1)
                return -1;
            if (j < 0 || j > dim[1] - 1)
                return -1;
            if (k < 0 || k > dim[2] - 1)
                return -1;

            int col = i;
            int row = k * dim[0];
            int stack = j * dim[0] * dim[2];
            return col + row + stack;
        }

        void MACGrid3d::getCell(int index, int &i, int &j, int &k)
        {
            j = (int)index / (dim[0] * dim[2]);
            k = (int)(index - j * dim[0] * dim[2]) / dim[0];
            i = index - j * dim[0] * dim[2] - k * dim[0];
        }

        glm::vec3 MACGrid3d::getCenter(int i, int j, int k)
        {
            double xstart = cellSize / 2.0;
            double ystart = cellSize / 2.0;
            double zstart = cellSize / 2.0;

            double x = xstart + i * cellSize;
            double y = ystart + j * cellSize;
            double z = zstart + k * cellSize;
            return glm::vec3(x, y, z);
        }

        glm::vec3 MACGrid3d::getLeft(int i, int j, int k)
        {
            return getCenter(i, j, k) - glm::vec3(0.0, cellSize * 0.5, 0.0);
        }

        glm::vec3 MACGrid3d::getRight(int i, int j, int k)
        {
            return getCenter(i, j, k) + glm::vec3(0.0, cellSize * 0.5, 0.0);
        }

        glm::vec3 MACGrid3d::getTop(int i, int j, int k)
        {
            return getCenter(i, j, k) + glm::vec3(0.0, 0.0, cellSize * 0.5);
        }

        glm::vec3 MACGrid3d::getBottom(int i, int j, int k)
        {
            return getCenter(i, j, k) - glm::vec3(0.0, 0.0, cellSize * 0.5);
        }

        glm::vec3 MACGrid3d::getFront(int i, int j, int k)
        {
            return getCenter(i, j, k) + glm::vec3(cellSize * 0.5, 0.0, 0.0);
        }

        glm::vec3 MACGrid3d::getBack(int i, int j, int k)
        {
            return getCenter(i, j, k) - glm::vec3(cellSize * 0.5, 0.0, 0.0);
        }

        glm::vec3 MACGrid3d::getVelocity(const glm::vec3 &pt)
        {
            if (inSolid(pt))
            {
                return glm::vec3(0);
            }

            glm::vec3 vel;
            vel[0] = getVelocityX(pt);
            vel[1] = getVelocityY(pt);
            vel[2] = getVelocityZ(pt);
            return vel;
        }

        double MACGrid3d::getVelocityX(const glm::vec3 &pt)
        {
            return mU.interpolate(pt);
        }

        double MACGrid3d::getVelocityY(const glm::vec3 &pt)
        {
            return mV.interpolate(pt);
        }

        double MACGrid3d::getVelocityZ(const glm::vec3 &pt)
        {
            return mW.interpolate(pt);
        }

        double MACGrid3d::getTemperature(const glm::vec3 &pt)
        {
            return mT.interpolate(pt);
        }

        double MACGrid3d::getDensity(const glm::vec3 &pt)
        {
            return mD.interpolate(pt);
        }

        int MACGrid3d::numSolidCells()
        {
            int numSolid = 0;
            FOR_EACH_CELL { numSolid += mSolid(i, j, k); }
            return numSolid;
        }

        bool MACGrid3d::inSolid(const glm::vec3 &pt)
        {
            int i, j, k;
            mSolid.getCell(pt, i, j, k);
            return isSolidCell(i, j, k) == 1;
        }

        bool MACGrid3d::inSolid(const glm::vec3 &pt, int &i, int &j, int &k)
        {
            mSolid.getCell(pt, i, j, k);
            return isSolidCell(i, j, k) == 1;
        }

        int MACGrid3d::isSolidCell(int i, int j, int k)
        {
            bool containerBoundary = (i < 0 || i > dim[0] - 1) ||
                                     (j < 0 || j > dim[1] - 1) ||
                                     (k < 0 || k > dim[2] - 1);

            bool objectBoundary = (mSolid(i, j, k) == 1);

            return containerBoundary || objectBoundary ? 1 : 0;
        }

        int MACGrid3d::isSolidFace(int i, int j, int k, MACGrid3d::Direction d)
        {
            if (d == X && (i == 0 || i == dim[0]))
                return 1;
            else if (d == Y && (j == 0 || j == dim[1]))
                return 1;
            else if (d == Z && (k == 0 || k == dim[2]))
                return 1;

            if (d == X && (mSolid(i, j, k) || mSolid(i - 1, j, k)))
                return 1;
            if (d == Y && (mSolid(i, j, k) || mSolid(i, j - 1, k)))
                return 1;
            if (d == Z && (mSolid(i, j, k) || mSolid(i, j, k - 1)))
                return 1;

            return 0;
        }

        bool MACGrid3d::isNeighbor(int i0, int j0, int k0, int i1, int j1, int k1)
        {
            if (abs(i0 - i1) == 1 && j0 == j1 && k0 == k1)
                return true;
            if (abs(j0 - j1) == 1 && i0 == i1 && k0 == k1)
                return true;
            if (abs(k0 - k1) == 1 && j0 == j1 && i0 == i1)
                return true;
            return false;
        }

        double MACGrid3d::getPressureCoeffBetweenCells(
            int i, int j, int k, int pi, int pj, int pk)
        {
            if (i == pi && j == pj && k == pk) // self
            {
                int numSolidNeighbors = (isSolidCell(i + 1, j, k) +
                                         isSolidCell(i - 1, j, k) +
                                         isSolidCell(i, j + 1, k) +
                                         isSolidCell(i, j - 1, k) +
                                         isSolidCell(i, j, k + 1) +
                                         isSolidCell(i, j, k - 1));
                return 6.0 - numSolidNeighbors;
            }
            if (isNeighbor(i, j, k, pi, pj, pk) && !isSolidCell(pi, pj, pk))
                return -1.0;
            return 0.0;
        }

        glm::vec4 MACGrid3d::getRenderColor(int i, int j, int k)
        {
            double value = mD(i, j, k);
            return glm::vec4(1.0, 1.0, 1.0, value);
        }

        glm::vec4 MACGrid3d::getRenderColor(const glm::vec3 &pt)
        {
            double value = getDensity(pt);
            return glm::vec4(value, value, value, value);
        }

        bool MACGrid3d::isValid(int i, int j, int k, MACGrid3d::Direction d)
        {
            switch (d)
            {
            case X:
                return (i >= 0 && i < dim[X] + 1 &&
                        j >= 0 && j < dim[Y] &&
                        k >= 0 && k < dim[Z]);
            case Y:
                return (i >= 0 && i < dim[X] &&
                        j >= 0 && j < dim[Y] + 1 &&
                        k >= 0 && k < dim[Z]);
            case Z:
                return (i >= 0 && i < dim[X] &&
                        j >= 0 && j < dim[Y] &&
                        k >= 0 && k < dim[Z] + 1);
            }
            Glb::Logger::getInstance().addLog("Error: bad direction");
            return false;
        }
    }
}
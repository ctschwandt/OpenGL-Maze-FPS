// File: HeightMap.h
// Name: Cole Schwandt

#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <GL/freeglut.h>
#include <iostream>
#include <cmath>
#include <cstdlib>

//==============================================================
// Helpers
//==============================================================
inline
GLfloat random_in_range(GLfloat max_range)
{
    GLfloat random = GLfloat(rand()) / GLfloat(RAND_MAX); // \in [0, 1]
    GLfloat centered = (random + random) - 1.0f; // \in [-1, 1]
    return centered * max_range; // \in [-max_range, max_range]
}

//==============================================================
// HeightMap
//==============================================================
class HeightMap
{
public:
    // n : so that N_ = 2^n + 1
    // M : user scale for initial random range (we use M_ = 0.25 * M)
    // r : roughness parameter
    HeightMap(GLint n, GLfloat M, GLfloat r)
    {
        reset(n, M, r);
    }

    ~HeightMap()
    {
        if (map_ != nullptr)
        {
            for (GLint i = 0; i < N_; ++i)
            {
                delete[] map_[i];
            }
            delete[] map_;
        }
    }

    void destroy_map()
    {
        if (!map_)
            return;

        for (GLint i = 0;i < N_; ++i)
        {
            delete[] map_[i];
        }
        delete[] map_;
        map_ = nullptr;
    }

    // reinitialize with new n, M, r
    void reset(GLint n, GLfloat M, GLfloat r)
    {
        destroy_map();

        N_ = (1 << n) + 1;
        M_ = M * 0.25f;
        r_ = r;

        allocate_heightmap();
        set_corners();
        run_diamond_square();
    }

    void allocate_heightmap()
    {
        map_ = new GLfloat*[N_];
        for (GLint i = 0; i < N_; ++i)
        {
            map_[i] = new GLfloat[N_];
            for (GLint j = 0; j < N_; ++j)
            {
                map_[i][j] = 0.0f;
            }
        }
    }

    // set the 4 corners of heightmap to random numbers in [-M_, M_]
    void set_corners()
    {
        map_[0][0] = random_in_range(M_);
        map_[0][N_ - 1] = random_in_range(M_);
        map_[N_ - 1][0] = random_in_range(M_);
        map_[N_ - 1][N_ - 1] = random_in_range(M_);
    }

    // average of N,S,E,W neighbors at distance half (fixed boundaries)
    GLfloat average_cardinal_dirs(GLint i, GLint j, GLint half)
    {
        GLfloat sum = 0.0f;
        GLint count = 0;

        // N
        if (j - half >= 0)
        {
            sum += map_[i][j - half];
            ++count;
        }

        // S
        if (j + half < N_)
        {
            sum += map_[i][j + half];
            ++count;
        }

        // W
        if (i - half >= 0)
        {
            sum += map_[i - half][j];
            ++count;
        }

        // E
        if (i + half < N_)
        {
            sum += map_[i + half][j];
            ++count;
        }

        return sum / GLfloat(count);
    }

    // diamond step for all squares of width w
    void diamond_step(GLint w)
    {
        GLint half = w / 2;

        for (GLint x = 0; x < N_ - 1; x += w)
        {
            for (GLint z = 0; z < N_ - 1; z += w)
            {
                GLint cx = x + half;
                GLint cz = z + half;

                GLfloat NW = map_[x][z];
                GLfloat NE = map_[x+w][z];
                GLfloat SW = map_[x][z+w];
                GLfloat SE = map_[x+w][z+w];

                GLfloat avg = (NW + NE + SW + SE) / 4.0f;

                // set center of square to avg(NW,NE,SW,SE) + random in [-M_, M_]
                map_[cx][cz] = avg + random_in_range(M_);
            }
        }
    }

    // square step for all edge midpoints of all squares of width w
    void square_step(GLint w)
    {
        GLint half = w / 2;

        for (GLint x = 0; x < N_; x += half)
        {
            GLint start_z = ((x / half) % 2 == 0) ? half : 0;

            for (GLint z = start_z; z < N_; z += w)
            {
                GLfloat avg = average_cardinal_dirs(x, z, half);
                map_[x][z] = avg + random_in_range(M_);
            }
        }
    }

    void run_diamond_square()
    {
        for (GLint w = N_ - 1; w >= 2; w /= 2)
        {
            diamond_step(w);
            square_step(w);

            // reduce M_ by changing M_ to M_ * 2^(-r_)
            M_ *= std::pow(2.0f, -r_);
        }
    }

    GLint     N_; // size of map: N_ x N_
    GLfloat ** map_; // heightmap
    GLfloat   M_; // current random range
    GLfloat   r_; // roughness parameter

private:
};

inline
std::ostream & operator<<(std::ostream & cout, const HeightMap & hm)
{
    for (GLint z = 0; z < hm.N_; ++z)
    {
        std::string delim = "";
        for (GLint x = 0; x < hm.N_; ++x)
        {
            cout << delim << hm.map_[x][z];
            delim = " ";
        }
        cout << '\n';
    }
    return cout;
}

#endif // HeightMap.h

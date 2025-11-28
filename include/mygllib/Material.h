#ifndef MATERIAL_H
#define MATERIAL_H

#include <GL/glew.h>
#include <iostream>

namespace mygllib
{
    class MaterialError
    {};
        
    class Material
    {
    public:
        Material(int i, GLenum face=GL_FRONT)
            : i_(i),
              ambient_(material + 13 * i),
              diffuse_(material + 13 * i + 4),
              specular_(material + 13 * i + 8),
              shininess_(material + 13 * i + 12),
              face_(face)
            
        {
            if (i < 0 || i > 23)
            {
                std::cout << "ERROR: wrong material" << std::endl;
                throw MaterialError();
            }
        }
        void set()
        {
            glMaterialfv(face_, GL_AMBIENT, ambient());
            glMaterialfv(face_, GL_DIFFUSE, diffuse());
            glMaterialfv(face_, GL_SPECULAR, specular());
            glMaterialfv(face_, GL_SHININESS, shininess());
        }
        
        float * ambient() { return ambient_; }
        float * diffuse() { return diffuse_; }
        float * specular() { return specular_; }
        float * shininess() { return shininess_; }
        
        static const int       EMERALD;
        static const int          JADE;
        static const int      OBSIDIAN;
        static const int         PEARL;
        static const int          RUBY;
        static const int     TURQUOISE;
        static const int         BRASS;
        static const int        BRONZE;
        static const int        CHROME;
        static const int        COPPER;
        static const int          GOLD;
        static const int        SILVER;
        static const int BLACK_PLASTIC;
        static const int  CYAN_PLASTIC;
        static const int GREEN_PLASTIC;
        static const int   RED_PLASTIC;
        static const int WHITE_PLASTIC;
        static const int YELLOW_PLASTIC;
        static const int  BLACK_RUBBER;
        static const int   CYAN_RUBBER;
        static const int  GREEN_RUBBER;
        static const int    RED_RUBBER;
        static const int  WHITE_RUBBER;
        static const int YELLOW_RUBBER;
        static float material[];
    private:
        int i_;
        float * ambient_;
        float * diffuse_;
        float * specular_;
        float * shininess_;
        GLenum face_;
    };
};

#endif // MATERIAL_H

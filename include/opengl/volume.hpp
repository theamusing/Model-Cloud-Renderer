#ifndef VOLUME_H
#define VOLUME_H

#include <glad/glad.h> // holds all OpenGL type declarations
#include <opengl/model.hpp>
#include <opengl/shader.hpp>

#include <iostream>

class Volume
{
public:
    // volume data
    Model model;
    glm::vec3 AABB[2]; // axis-aligned bounding box
    // constructor
    Volume(const char *path, bool flip = false) : model(path, flip)
    {
        setupAABB();
        std::cout << "Volume AABB: " << AABB[0].x << " " << AABB[0].y << " " << AABB[0].z << " " << AABB[1].x << " " << AABB[1].y << " " << AABB[1].z << std::endl;
    }

    // render the volume
    void Draw(Shader &shader)
    {
        shader.setVec3("AABBMin", AABB[0]);
        shader.setVec3("AABBMax", AABB[1]);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    GLuint VAO, VBO, EBO;
    vector<glm::vec3> vertices;
    vector<unsigned int> indices;

    void setupAABB(){
        // generate AABB for volume
        for (unsigned int i = 0; i < model.meshes.size(); i++)
        {
            for (unsigned int j = 0; j < model.meshes[i].vertices.size(); j++)
            {
                glm::vec3 vertex = model.meshes[i].vertices[j].Position;
                if (j == 0)
                {
                    AABB[0] = AABB[1] = vertex;
                }
                else
                {
                    for (unsigned int k = 0; k < 3; k++)
                    {
                        if (vertex[k] < AABB[0][k])
                            AABB[0][k] = vertex[k];
                        if (vertex[k] > AABB[1][k])
                            AABB[1][k] = vertex[k];
                    }
                }
            }
        }
        vertices = {
            // front
            glm::vec3(AABB[0].x, AABB[0].y, AABB[0].z),
            glm::vec3(AABB[1].x, AABB[0].y, AABB[0].z),
            glm::vec3(AABB[1].x, AABB[1].y, AABB[0].z),
            glm::vec3(AABB[0].x, AABB[1].y, AABB[0].z),
            // back
            glm::vec3(AABB[0].x, AABB[0].y, AABB[1].z),
            glm::vec3(AABB[1].x, AABB[0].y, AABB[1].z),
            glm::vec3(AABB[1].x, AABB[1].y, AABB[1].z),
            glm::vec3(AABB[0].x, AABB[1].y, AABB[1].z),
        };
        indices = {
            0, 1, 2, 2, 3, 0,
            1, 5, 6, 6, 2, 1,
            5, 4, 7, 7, 6, 5,
            4, 0, 3, 3, 7, 4,
            3, 2, 6, 6, 7, 3,
            4, 5, 1, 1, 0, 4
        };
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // vertex Positions
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        glBindVertexArray(0);
    }
};


#endif // VOLUME_H

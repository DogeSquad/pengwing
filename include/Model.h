#pragma once

#include "Shader.h"
#include "Mesh.h"
#include "config.hpp"

#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
    Model()
    {

    }
    Model(char* path, bool smooth)
    {
        loadModel(DATA_ROOT + "../models/" + path, smooth);
    }
    void Draw(Shader& shader);
private:
    // model data
    vector<Mesh> meshes;
    vector<Mesh::Texture> textures_loaded;
    string directory;

    void loadModel(string path, bool smooth);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    vector<Mesh::Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
        string typeName);
};
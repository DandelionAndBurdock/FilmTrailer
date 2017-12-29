#pragma once

#include <vector>
class string;

class aiNode;
class aiScene;
class aiMesh;
class aiMaterial;
enum aiTextureType;

#include "MeshAI.h"
#include "ShaderAI.h"

class Model
{
public:
	/*  Functions   */
	Model(const char* path);
	void Draw(const ShaderAI& shader) const;
private:
	/*  Model Data  */
	std::vector<MeshAI> meshes;
	std::string directory;
	/*  Functions   */
	void LoadModel(std::string path);
	void ProcessNode(aiNode* node, const aiScene* scene);
	MeshAI ProcessMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<TextureAI> LoadMaterialTextures(aiMaterial* mat, aiTextureType type,
		                                      std::string typeName);
};
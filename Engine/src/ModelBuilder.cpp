#include "ModelBuilder.h"

#include "Model.h"

ModelBuilder& ModelBuilder::setNumOfVertices(size_t size)
{
	m_numOfVertices = size;

	return *this;
}

ModelBuilder& ModelBuilder::setPositions(std::vector<glm::vec3>& positions, bool copy)
{
	if (copy)
	{
		m_positions = std::make_shared<std::vector<glm::vec3>>(positions);
	}
	else
	{
		m_positions = std::shared_ptr<std::vector<glm::vec3>>(&positions);
	}

	return *this;
}

ModelBuilder& ModelBuilder::setPositions(const float* positions, size_t size)
{
	m_positions = std::make_shared<std::vector<glm::vec3>>();

	for (int i = 0; i < size; i++)
	{
		glm::vec3 pos{ positions[i * 3 + 0], positions[i * 3 + 1], positions[i * 3 + 2] };
		m_positions->emplace_back(pos);
	}

	return *this;
}

ModelBuilder& ModelBuilder::setNormals(std::vector<glm::vec3>& normals, bool copy)
{
	if (copy)
	{
		m_normals = std::make_shared<std::vector<glm::vec3>>(normals);
	}
	else
	{
		m_normals = std::shared_ptr<std::vector<glm::vec3>>(&normals);
	}

	return *this;
}

ModelBuilder& ModelBuilder::setNormals(const float* normals, size_t size)
{
	m_normals = std::make_shared<std::vector<glm::vec3>>();

	for (int i = 0; i < size; i++)
	{
		glm::vec3 normal{ normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2] };
		m_normals->emplace_back(normal);
	}

	return *this;
}

ModelBuilder& ModelBuilder::setTexcoords(std::vector<glm::vec2>& texCoords, bool copy)
{
	if (copy)
	{
		m_texCoords = std::make_shared<std::vector<glm::vec2>>(texCoords);
	}
	else
	{
		m_texCoords = std::shared_ptr<std::vector<glm::vec2>>(&texCoords);
	}

	return *this;
}

ModelBuilder& ModelBuilder::setTexcoords(const float* texCoords, size_t size)
{
	m_texCoords = std::make_shared<std::vector<glm::vec2>>();

	for (int i = 0; i < size; i++)
	{
		glm::vec3 texCoord{ texCoords[i * 3 + 0], texCoords[i * 3 + 1], texCoords[i * 3 + 2] };
		m_texCoords->emplace_back(texCoord);
	}

	return *this;
}

ModelBuilder& ModelBuilder::setColors(std::vector<glm::vec3>& colors, bool copy)
{
	if (copy)
	{
		m_colors = std::make_shared<std::vector<glm::vec3>>(colors);
	}
	else
	{
		m_colors = std::shared_ptr<std::vector<glm::vec3>>(&colors);
	}

	return *this;
}

ModelBuilder& ModelBuilder::setColors(const float* colors, size_t size)
{
	m_colors = std::make_shared<std::vector<glm::vec3>>();

	for (int i = 0; i < size; i++)
	{
		glm::vec3 color{ colors[i * 3 + 0], colors[i * 3 + 1], colors[i * 3 + 2] };
		m_colors->emplace_back(color);
	}

	return *this;
}

ModelBuilder& ModelBuilder::setIndices(std::vector<unsigned int>& indices, bool copy)
{
	if (copy)
	{
		m_indices = std::make_shared<std::vector<unsigned int>>(indices);
	}
	else
	{
		m_indices = std::shared_ptr<std::vector<unsigned int>>(&indices);
	}

	return *this;
}

ModelBuilder& ModelBuilder::setRawVertices(const float* vertices, Mesh::VerticesLayout layout)
{
	setNumOfVertices(layout.numOfVertices);
	// calculate stride
	int stride = 0;
	for (auto entry : layout.attribs)
	{
		stride += getAttributeSize(entry);
	}

	int offset = 0;

	// Parse vertices
	auto positions = new std::vector<glm::vec3>();
	auto normals = new std::vector<glm::vec3>();
	auto texcoords = new std::vector<glm::vec2>();
	auto colors = new std::vector<glm::vec3>();

	for (auto entry : layout.attribs)
	{
		// Parse positions
		if (LayoutAttributes::Positions == entry)
		{
			positions->reserve(layout.numOfVertices * getAttributeSize(entry));
			for (int i = 0; i < layout.numOfVertices; i++)
			{
				glm::vec3 pos;
				for (int j = 0; j < getAttributeSize(entry); j++)
				{
					pos[j] = vertices[stride * i + j + offset];
				}
				positions->emplace_back(pos);
			}
			setPositions(*positions);
		}

		// Parse normals
		else if (LayoutAttributes::Normals == entry)
		{
			normals->reserve(layout.numOfVertices * getAttributeSize(entry));
			for (int i = 0; i < layout.numOfVertices; i++)
			{
				glm::vec3 normal;
				for (int j = 0; j < getAttributeSize(entry); j++)
				{
					normal[j] = vertices[stride * i + j + offset];
				}
				normals->emplace_back(normal);
			}
			setNormals(*normals);
		}

		// Parse texcoords
		else if (LayoutAttributes::Texcoords == entry)
		{
			texcoords->reserve(layout.numOfVertices * getAttributeSize(entry));
			for (int i = 0; i < layout.numOfVertices; i++)
			{
				glm::vec2 vec;
				for (int j = 0; j < getAttributeSize(entry); j++)
				{
					vec[j] = vertices[stride * i + j + offset];
				}
				texcoords->emplace_back(vec);
			}
			setTexcoords(*texcoords);
		}

		// Parse colors
		else if (LayoutAttributes::Colors == entry)
		{
			colors->reserve(layout.numOfVertices * getAttributeSize(entry));
			for (int i = 0; i < layout.numOfVertices; i++)
			{
				glm::vec3 color;
				for (int j = 0; j < getAttributeSize(entry); j++)
				{
					color[j] = vertices[stride * i + j + offset];
				}
				colors->emplace_back(color);
			}
			setColors(*colors);
		}

		offset += getAttributeSize(entry);
	}

	return *this;
}

ModelBuilder& ModelBuilder::setShader(Shader& shader, bool copy)
{
	if (copy)
	{
		m_shader = std::make_shared<Shader>(shader);
	}
	else
	{
		m_shader = std::shared_ptr<Shader>(&shader);
	}

	return *this;
}

ModelBuilder& ModelBuilder::setRawIndices(const unsigned int* indices, size_t size)
{
	auto vec = new std::vector<unsigned int>();
	vec->reserve(size);
	for (auto i = 0; i < size; i++)
	{
		vec->push_back(indices[i]);
	}
	setIndices(*vec);

	return *this;
}

Model* ModelBuilder::build()
{
	auto model = new Model();

	auto mesh = generateMesh();

	//TODO optimize: can load textuer on startup and simply assign texture Ptr / ID
	auto texturediff = Texture::loadTextureFromFile("Resources\\Textures\\template.png");
	texturediff->setType(Texture::Type::Diffuse);

	auto textureSpec = Texture::loadTextureFromFile("Resources\\Textures\\template.png");
	textureSpec->setType(Texture::Type::Specular);

	mesh->addTexture(texturediff);
	mesh->addTexture(textureSpec);

	std::shared_ptr<Material> material = std::make_shared<Material>(32.0f);
	model->UseMaterial(material);

	model->addMesh(mesh);

	return model;
}

Mesh* ModelBuilder::generateMesh()
{
	auto mesh = new Mesh();

	if(m_positions)
		mesh->setPositions(m_positions);

	if (m_normals)
		mesh->setNormals(m_normals);

	if (m_texCoords)
		mesh->setTexcoords(m_texCoords);

	if (m_colors)
		mesh->setColors(m_colors);

	if (m_indices)
		mesh->setIndices(m_indices);

	mesh->setNumOfVertices(m_numOfVertices);

	mesh->build();

	return mesh;
}

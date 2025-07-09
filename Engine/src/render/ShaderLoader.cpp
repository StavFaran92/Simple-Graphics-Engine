#include "ShaderLoader.h"
#include "Utils.h"
#include "Logger.h"
#include <map>

enum class ShaderType
{
    VERTEX,
    FRAGMENT,
    GEOMETRY,
    COMPUTE,
    TESC,
    TESE,

    NONE
};

ShaderLoader::ShaderLoader(std::shared_ptr<IShaderParser> shaderParser, LoadParams eParams)
    : m_shaderParser(shaderParser), m_params(eParams)
{
}

std::string ShaderLoader::readShader(const std::string& path)
{
    // If extender is not active, simply read the file content and return it as is.
    if (!m_params.extendShader)
    {
        return Utils::ReadFile(path);
    }

    return m_shaderParser->parse(path);
}

bool ShaderLoader::parseGLSLShader(const std::string& sCode, ShadersInfo& shader) 
{
    std::stringstream stream(sCode);

    std::string line, directive;
    ShaderType sType = ShaderType::NONE;
    std::map<ShaderType, std::stringstream> shaderStreams;

    while (std::getline(stream, line)) 
    {
        std::stringstream lineStream(line);
        lineStream >> directive;

        if (directive == "#vert") {
            sType = ShaderType::VERTEX;
            continue;
        }
        else if (directive == "#frag") {
            sType = ShaderType::FRAGMENT;
            continue;
        }
        else if (directive == "#geom") {
            sType = ShaderType::GEOMETRY;
            continue;
        }
        else if (directive == "#compute") {
            sType = ShaderType::COMPUTE;
            continue;
        }
        else if (directive == "#tesc") {
            sType = ShaderType::TESC;
            continue;
        }
        else if (directive == "#tese") {
            sType = ShaderType::TESE;
            continue;
        }

        shaderStreams[sType] << line << '\n';
    }

    shader.vertexCode = shaderStreams[ShaderType::VERTEX].str();
    shader.fragmentCode = shaderStreams[ShaderType::FRAGMENT].str();
    shader.geometryCode = shaderStreams[ShaderType::GEOMETRY].str();
    shader.computeCode = shaderStreams[ShaderType::COMPUTE].str();
    shader.tessControlCode = shaderStreams[ShaderType::TESC].str();
    shader.tessEvaluationCode = shaderStreams[ShaderType::TESE].str();

    return true;
}

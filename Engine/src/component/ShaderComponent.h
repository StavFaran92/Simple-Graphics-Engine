#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "render/Shader.h"
#include "render/RenderView.h"
#include "texture/Texture.h"
#include <map>
#include <unordered_map>
#include <string>

struct EngineAPI ShaderComponent : public Component
{
        enum ProjectionType : int
        {
                DefaultProjection = 0,
                Texture2D = 1
        };

        ShaderComponent();

        ShaderComponent(Shader* vertexShader, Shader* fragmentShader);

        void addTexture(const std::string& name, Resource<Texture> texture)
        {
                customTextures[name] = texture;
        }

        void setProjectionTexture(Resource<Texture> texture);

        void update();

        void parseUniforms(const std::string& sourceCode);

        void setShader(Resource<Shader> shader);

        template <class Archive>
        void serialize(Archive& archive) {
                        SERIALIZED_MEMBER(m_customShader);
                        SERIALIZED_MEMBER(shaderOverride);
                        SERIALIZED_MEMBER(customTextures);
                        SERIALIZED_MEMBER(projection);
                        SERIALIZED_MEMBER(projectionTexture);
                        SERIALIZED_MEMBER(m_shaderFilePath);
                        SERIALIZED_MEMBER(isValid);
        }

        Resource<Shader> m_customShader;
        std::map<std::string, Resource<Texture>> customTextures;
        ProjectionType projection = ProjectionType::DefaultProjection;
        Resource<Texture> projectionTexture;
        std::shared_ptr<RenderView> renderViewProjection;
        std::unordered_map<std::string, Value> m_uniformProperties;
        std::string m_shaderFilePath;
        ShaderOverride shaderOverride;
        bool isValid = false;
        static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);
};

REGISTER_COMPONENT(ShaderComponent)

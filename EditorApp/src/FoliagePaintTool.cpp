#include "FoliagePaintTool.h"

#include "sge.h"

#include "imgui.h"

extern Entity g_editorCamera;
extern Terrain* g_activeTerrain;



struct Ray
{
	Ray(glm::vec3 origin, glm::vec3 dir)
		: origin(origin), direction(dir)
	{
	}

	glm::vec3 origin;
	glm::vec3 direction;
};

bool RayIntersectXZBounds(
	const Ray& ray,
	const AABB& boundingBox,
	float& tEnter,
	float& tExit
) {
	tEnter = 0.0f;
	tExit = std::numeric_limits<float>::infinity();

	auto slab = [&](float origin, float dir, float minB, float maxB) {
		if (std::abs(dir) < 1e-6f) {
			if (origin < minB || origin > maxB)
				return false;
			return true;
		}

		float invD = 1.0f / dir;
		float t0 = (minB - origin) * invD;
		float t1 = (maxB - origin) * invD;
		if (t0 > t1) std::swap(t0, t1);

		tEnter = std::max(tEnter, t0);
		tExit = std::min(tExit, t1);
		return tEnter <= tExit;
		};

	if (!slab(ray.origin.x, ray.direction.x, boundingBox.minX(), boundingBox.maxX())) return false;
	if (!slab(ray.origin.z, ray.direction.z, boundingBox.minZ(), boundingBox.maxZ())) return false;

	return true;
}

RayHit RaycastTerrainHeightmap(
	const Ray& ray,
	const Terrain& terrain,
	float maxDistance = 10000.0f
) {
	RayHit result;

	if (ray.direction.y >= 0.0f)
		return result;

	float dirXZ = glm::length(glm::vec2(ray.direction.x, ray.direction.z));
	if (dirXZ < 1e-5f)
		return result;

	AABB bounds = terrain.getAABB();

	float tStart, tEnd;
	if (!RayIntersectXZBounds(ray, bounds, tStart, tEnd))
		return result;

	tStart = std::max(tStart, 0.0f);
	tEnd = std::min(tEnd, maxDistance);

	constexpr float STEP_XZ = 0.5f;
	float stepT = STEP_XZ / dirXZ;

	float t = tStart;
	float prevDiff = 0.0f;
	bool first = true;

	while (t <= tEnd) {
		glm::vec3 p = ray.origin + ray.direction * t;

		float h;
		if (!terrain.getHeightAtPoint(p.x, p.z, h)) {
			t += stepT;
			first = true;
			continue;
		}

		float diff = p.y - h;

		if (!first && prevDiff > 0.0f && diff <= 0.0f) {
			// refine hit
			float t0 = t - stepT;
			float t1 = t;

			for (int i = 0; i < 5; ++i) {
				float tm = 0.5f * (t0 + t1);
				glm::vec3 pm = ray.origin + ray.direction * tm;

				float hm;
				terrain.getHeightAtPoint(pm.x, pm.z, hm);

				if (pm.y > hm)
					t0 = tm;
				else
					t1 = tm;
			}

			float tHit = 0.5f * (t0 + t1);

			result.hit = true;
			result.t = tHit;
			result.position = ray.origin + ray.direction * tHit;

			// normal
			const float eps = 0.1f;
			float hL, hR, hD, hU;
			terrain.getHeightAtPoint(result.position.x - eps, result.position.z, hL);
			terrain.getHeightAtPoint(result.position.x + eps, result.position.z, hR);
			terrain.getHeightAtPoint(result.position.x, result.position.z - eps, hD);
			terrain.getHeightAtPoint(result.position.x, result.position.z + eps, hU);

			result.normal = glm::normalize(glm::vec3(
				hL - hR,
				2.0f * eps,
				hD - hU
			));

			return result;
		}

		prevDiff = diff;
		first = false;
		t += stepT;
	}

	return result;
}

std::pair<glm::vec3, glm::vec3 > ScreenPointToRay(
	float mouseX, float mouseY,
	float viewportWidth, float viewportHeight,
	const glm::mat4& view,
	const glm::mat4& projection
) {
	// 1. NDC
	glm::vec2 ndc;
	ndc.x = (mouseX * 2.0) / viewportWidth - 1.0f;
	ndc.y = -((mouseY * 2.0) / viewportHeight - 1.0f);

	// 2. Clip space
	glm::vec4 rayClip(ndc, -1.0f, 1.0f);

	// 3. View space
	glm::vec4 rayView = glm::inverse(projection) * rayClip;
	rayView = glm::vec4(rayView.x, rayView.y, -1.0f, 0.0f);

	// 4. World space
	glm::vec3 rayDir = glm::normalize(glm::vec3(glm::inverse(view) * rayView));

	// 5. Origin
	glm::vec3 rayOrigin = glm::vec3(glm::inverse(view)[3]); // todo use camera pos here

	return { rayOrigin, rayDir };
}

void FoliagePaintTool::update(ImVec2 windowPos, ImVec2 viewportSize)
{
	ImVec2 renderViewWindowSize = viewportSize;
	float innerWindowWidth = renderViewWindowSize.x;
	float innerWindowHeight = 35.0f;
	ImVec2 toolbarPos(windowPos.x + 10, windowPos.y + 30);

	ImVec2 mousePos = ImGui::GetMousePos();
	ImVec2 viewportOffset = ImGui::GetWindowContentRegionMin();
	ImVec2 viewportPos{ windowPos.x + viewportOffset.x, windowPos.y + viewportOffset.y };

	bool mouseInsideViewport = (mousePos.x >= viewportPos.x && mousePos.x <= viewportPos.x + renderViewWindowSize.x &&
		mousePos.y >= viewportPos.y && mousePos.y <= viewportPos.y + renderViewWindowSize.y);
	bool mouseInsideToolbar = (mousePos.x >= toolbarPos.x && mousePos.x <= toolbarPos.x + innerWindowWidth &&
		mousePos.y >= toolbarPos.y && mousePos.y <= toolbarPos.y + innerWindowHeight);

	// We alter the mouse position from small window into full screen (the renderered object pick texture)
	int alteredX = (mousePos.x - viewportPos.x) / renderViewWindowSize.x * Engine::get()->getWindow()->getWidth();
	int alteredY = (mousePos.y - viewportPos.y) / renderViewWindowSize.y * Engine::get()->getWindow()->getHeight();

	//logDebug("altered mouse x:{}, y:{}", alteredX, alteredY);

	auto& cameraComponent = g_editorCamera.getComponent<CameraComponent>();
	auto& cameraTransform = g_editorCamera.getComponent<Transformation>();
	glm::mat4 view = glm::lookAt(cameraTransform.getWorldPosition(), cameraTransform.getWorldPosition() + cameraComponent.front, cameraComponent.up);
	auto projection = g_editorCamera.getComponent<CameraComponent>().getProjection();

	auto& [rayOrigin, rayDir] = ScreenPointToRay(alteredX, alteredY, Engine::get()->getWindow()->getWidth(), Engine::get()->getWindow()->getHeight(), view, projection);

	m_currentResult = RaycastTerrainHeightmap(Ray(rayOrigin, rayDir), *g_activeTerrain);
	if (m_currentResult.hit)
	{
		//if (Engine::get()->getInput()->getKeyboard()->getKeyState(KeyCode::SCANCODE_X) > 0)
		{
			Engine::get()->getSubSystem<Graphics>()->view = view;
			Engine::get()->getSubSystem<Graphics>()->projection = projection;
			//logDebug("ray origin {},{},{}, ray dir {},{},{}, hit position {},{},{}", rayOrigin.x, rayOrigin.y, rayOrigin.z, rayDir.x, rayDir.y, rayDir.z, results.position.x, results.position.y, results.position.z);
			unsigned int editorFrameBufferID = Engine::get()->getContext()->getActiveScene()->getRenderViewFrameBufferID("Editor View");
			glBindFramebuffer(GL_FRAMEBUFFER, editorFrameBufferID);
			DebugHelper::getInstance().drawLine(m_currentResult.position, m_currentResult.position + glm::vec3(0, 100, 0), glm::vec3(1, 0, 0), 3.f);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

	}
}

bool FoliagePaintTool::onEvent(SDL_Event e)
{
	if (Engine::get()->getInput()->getMouse()->getButtonPressed(MouseButton::MOUSE_BUTTON_LEFT))
	{
		glm::vec2 offsetPos = glm::vec2(m_currentResult.position.x, m_currentResult.position.z);
		offsetPos.x += g_activeTerrain->getWidth() * .5f;
		offsetPos.y += g_activeTerrain->getHeight() * .5f;
		//g_activeTerrain->m_foliageField.setPixel(offsetPos.x, offsetPos.y, 1);
		g_activeTerrain->m_foliageField.paintCircle(offsetPos.x, offsetPos.y, 5, 255);

		return true;
	}

	return false;
}

const char* FoliagePaintTool::name() const
{
    return "FoliagePaintTool";
}

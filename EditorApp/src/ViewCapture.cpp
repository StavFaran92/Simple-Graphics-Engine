#include "ViewCapture.h"

#include "sge.h"
#include "utils/STBIHelper.h"

#include <cstring>

bool captureRenderView(const std::string& viewName, CapturedImage& out)
{
	auto texture = Engine::get()->getContext()->getActiveScene()->getRenderViewTexture(viewName);
	if (!texture || !texture->download())
		return false;

	// Render targets are RGB8 (see RenderTarget), so the downloaded bytes are already 3 per pixel
	const TextureData& data = texture->getData();
	const int stride = data.width * 3;
	const uint8_t* pixels = static_cast<const uint8_t*>(data.data.data());

	out.width = data.width;
	out.height = data.height;
	out.rgb.resize(static_cast<size_t>(stride) * data.height);

	// GL rows start at the bottom, images at the top
	for (int row = 0; row < data.height; ++row)
		memcpy(&out.rgb[static_cast<size_t>(row) * stride], pixels + static_cast<size_t>(data.height - 1 - row) * stride, stride);

	return true;
}

CapturedImage downscaleToWidth(const CapturedImage& image, int maxWidth)
{
	if (maxWidth <= 0 || image.width <= maxWidth)
		return image;

	const int factor = (image.width + maxWidth - 1) / maxWidth;

	CapturedImage out;
	out.width = image.width / factor;
	out.height = image.height / factor;
	out.rgb.resize(static_cast<size_t>(out.width) * out.height * 3);

	for (int y = 0; y < out.height; ++y)
	{
		for (int x = 0; x < out.width; ++x)
		{
			for (int c = 0; c < 3; ++c)
			{
				int sum = 0;
				for (int dy = 0; dy < factor; ++dy)
					for (int dx = 0; dx < factor; ++dx)
						sum += image.rgb[(static_cast<size_t>(y * factor + dy) * image.width + (x * factor + dx)) * 3 + c];

				out.rgb[(static_cast<size_t>(y) * out.width + x) * 3 + c] = static_cast<uint8_t>(sum / (factor * factor));
			}
		}
	}

	return out;
}

std::vector<uint8_t> encodePng(const CapturedImage& image)
{
	return STBIHelper::encodePNG(image.width, image.height, 3, image.rgb.data(), image.width * 3);
}

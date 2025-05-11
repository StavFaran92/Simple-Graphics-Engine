#include "EventLayerStack.h"

void EventLayerStack::addLayer(std::shared_ptr<EventLayer> layer)
{
	m_layers.push_back(layer);
}

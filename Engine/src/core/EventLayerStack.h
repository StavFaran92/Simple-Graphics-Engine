#pragma once

#include <memory>
#include <vector>

#include "EventLayer.h"

class EventLayerStack
{
public:
	void addLayer(std::shared_ptr<EventLayer> layer);
	std::shared_ptr<EventLayer> pop();

	// Iterators (const and non-const)
	auto begin() { return m_layers.begin(); }
	auto end() { return m_layers.end(); }

	auto begin() const { return m_layers.begin(); }
	auto end() const { return m_layers.end(); }

private:
	std::vector<std::shared_ptr<EventLayer >> m_layers;
};
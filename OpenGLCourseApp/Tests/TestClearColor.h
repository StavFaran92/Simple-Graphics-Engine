#pragma once

#include "Test.h"

namespace test{
	class TestClearColor : public Test
	{
	public:
		TestClearColor();
		~TestClearColor();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnGuiRender() override;
	private:
		float mClearColor[4];
	};

}
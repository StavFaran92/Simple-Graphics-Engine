#pragma once

#include <functional>

template<typename T>
class Observable
{
public:
	Observable() = default;

	Observable(const std::string& name)
	{
		m_name = name;
	}

	Observable(const T& obj, const std::string& name)
	{
		m_obj = obj;
		m_name = name;
	}

	void set(const T& value)
	{
		m_obj = value;
		notifyOnChanged();
	}

	Observable& operator=(const T& other)
	{
		set(other);

		return *this;
	}

	Observable& operator=(T&& other)
	{
		m_obj = std::move(other);
		notifyOnChanged();
		return *this;
	}

	template<typename U>
	Observable<U> as() const
	{
		Observable<U> newObservable;
		newObservable.m_name = m_name;
		newObservable.m_obj = static_cast<U>(m_obj);
		newObservable.m_onChangedListener = m_onChangedListener;
		return newObservable;
	}

	const T& get() const
	{
		return m_obj;
	}

	void registerOnChangedListener(const std::function<void()>& cb)
	{
		m_onChangedListener = cb;
	}

	void removeOnChangedListener()
	{
		m_onChangedListener = nullptr;
	}
private:
	void notifyOnChanged()
	{
		if (m_onChangedListener)
		{
			m_onChangedListener();
		}
	}
public:
	T m_obj;
	std::string m_name;
	std::function<void()> m_onChangedListener;
};

//#define OBSERVABLE(type, name, value) \
//    Observable<type> name{value, #name}

#define OBSERVABLE(type, name) \
    Observable<type> name{#name}
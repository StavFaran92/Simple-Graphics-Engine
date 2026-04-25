#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

template<typename T>
class Observable
{
public:
	Observable() = default;

	explicit Observable(const std::string& name)
		: m_name(name)
	{
		if constexpr (std::is_default_constructible_v<T>)
		{
			m_obj = std::make_shared<T>();
		}
	}

	Observable(const T& obj, const std::string& name)
		: m_name(name)
		, m_obj(std::make_shared<T>(obj))
	{
	}

	// Implicit copy / move: shared_ptr is shared (same value + notifications via same storage).
	Observable(const Observable& other) = default;
	Observable(Observable&& other) noexcept = default;
	Observable& operator=(const Observable& other) = default;
	Observable& operator=(Observable&& other) noexcept = default;

	void set(const T& value)
	{
		if (!m_obj)
		{
			m_obj = std::make_shared<T>(value);
		}
		else
		{
			*m_obj = value;
		}
		notifyOnChanged();
	}

	Observable& operator=(const T& other)
	{
		set(other);
		return *this;
	}

	Observable& operator=(T&& other)
	{
		if (!m_obj)
		{
			m_obj = std::make_shared<T>(std::move(other));
		}
		else
		{
			*m_obj = std::move(other);
		}
		notifyOnChanged();
		return *this;
	}

	template<typename U>
	Observable<U> as() const
	{
		Observable<U> out;
		out.m_name = m_name;
		out.m_onChangedListener = m_onChangedListener;
		out.m_obj = std::static_pointer_cast<U>(m_obj);
		return out;
	}

	const T& get() const
	{
		assert(m_obj);
		return *m_obj;
	}

	const T& operator->() const
	{
		return get();
	}

	void registerOnChangedListener(const std::function<void()>& cb)
	{
		m_onChangedListener = cb;
	}

	void removeOnChangedListener()
	{
		m_onChangedListener = nullptr;
	}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_obj);
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
	std::shared_ptr<T> m_obj;
	std::string m_name;
	std::function<void()> m_onChangedListener;
};

#define OBSERVABLE(type, name) \
    Observable<type> name{ #name }
